/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

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

/*! \file PHY/LTE_TRANSPORT/dlsch_llr_computation.c
* \brief Top-level routines for LLR computation of the PDSCH physical channel from 36-211, V8.6 2009-03
* \author R. Knopp, F. Kaltenberger,A. Bhamri, S. Aubert, S. Wagner
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr,ankit.bhamri@eurecom.fr,sebastien.aubert@eurecom.fr, sebastian.wagner@eurecom.fr
* \note
* \warning
*/

#ifdef __SSE2__
#include <emmintrin.h>
#include <xmmintrin.h>
#endif
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"

#ifndef __SSE3__
//#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_abs_epi16(xmmx) _mm_add_epi16(_mm_xor_si128((xmmx),_mm_cmpgt_epi16((*(__m128i*)&zero[0]),(xmmx))),_mm_srli_epi16(_mm_cmpgt_epi16((*(__m128i*)&zero[0]),(xmmx)),15))
//#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmy)))
#endif

#ifndef USER_MODE
#define NOCYGWIN_STATIC static
#else
#define NOCYGWIN_STATIC 
#endif

NOCYGWIN_STATIC short zero[8] __attribute__ ((aligned(16))) = {0,0,0,0,0,0,0,0};
NOCYGWIN_STATIC short ones[8] __attribute__ ((aligned(16))) = {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff};
NOCYGWIN_STATIC __m128i rho_rpi __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_1_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_1_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_1_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_1_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_3_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_3_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_3_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_3_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_5_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_5_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_5_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_5_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_7_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_7_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_7_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_7_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_1_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_1_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_1_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_1_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_3_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_3_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_3_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_3_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_5_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_5_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_5_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_5_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_7_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_7_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_7_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_7_7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i psi_r_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i psi_i_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i a_r_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i a_i_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i psi_a_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i a_sq_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i bit_met_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  y0_p_1_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_1_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_1_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_1_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_3_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_3_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_3_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_3_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_5_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_5_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_5_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_5_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_7_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_7_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_7_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_7_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_1_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_1_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_1_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_1_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_3_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_3_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_3_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_3_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_5_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_5_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_5_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_5_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_7_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_7_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_7_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_7_7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  xmm0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm4 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm6 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm8 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  y0r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y1r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y1i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y2r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y2i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  logmax_num_re0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_num_im0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_den_re0 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m128i  logmax_den_im0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_num_re1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_num_im1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_den_re1 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m128i  logmax_den_im1 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i tmp_result  __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i tmp_result2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i tmp_result3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i tmp_result4 __attribute__ ((aligned(16)));

//==============================================================================================
// Auxiliary Makros

// calculates psi_a = psi_r*a_r + psi_i*a_i 
#define prodsum_psi_a_epi16(psi_r,a_r,psi_i,a_i,psi_a) tmp_result = _mm_mulhi_epi16(psi_r,a_r); tmp_result = _mm_slli_epi16(tmp_result,1); tmp_result2 = _mm_mulhi_epi16(psi_i,a_i); tmp_result2 = _mm_slli_epi16(tmp_result2,1); psi_a = _mm_adds_epi16(tmp_result,tmp_result2);

// calculate interference magnitude
#define interference_abs_epi16(psi,int_ch_mag,int_mag,c1,c2) tmp_result = _mm_cmplt_epi16(psi,int_ch_mag); tmp_result2 = _mm_xor_si128(tmp_result,(*(__m128i*)&ones[0])); tmp_result = _mm_and_si128(tmp_result,c1); tmp_result2 = _mm_and_si128(tmp_result2,c2); int_mag = _mm_or_si128(tmp_result,tmp_result2); 

// calculate interference magnitude
// tmp_result = ones in shorts corr. to interval 2<=x<=4, tmp_result2 interval < 2, tmp_result3 interval 4<x<6 and tmp_result4 interval x>6 
#define interference_abs_64qam_epi16(psi,int_ch_mag,int_two_ch_mag,int_three_ch_mag,a,c1,c3,c5,c7) tmp_result = _mm_cmplt_epi16(psi,int_two_ch_mag); tmp_result3 = _mm_xor_si128(tmp_result,(*(__m128i*)&ones[0])); tmp_result2 = _mm_cmplt_epi16(psi,int_ch_mag); tmp_result = _mm_xor_si128(tmp_result,tmp_result2); tmp_result4 = _mm_cmpgt_epi16(psi,int_three_ch_mag); tmp_result3 = _mm_xor_si128(tmp_result3,tmp_result4); tmp_result = _mm_and_si128(tmp_result,c3); tmp_result2 = _mm_and_si128(tmp_result2,c1); tmp_result3 = _mm_and_si128(tmp_result3,c5); tmp_result4 = _mm_and_si128(tmp_result4,c7); tmp_result = _mm_or_si128(tmp_result,tmp_result2); tmp_result3 = _mm_or_si128(tmp_result3,tmp_result4); a = _mm_or_si128(tmp_result,tmp_result3); 

// calculates a_sq = int_ch_mag*(a_r^2 + a_i^2)*scale_factor 
#define square_a_epi16(a_r,a_i,int_ch_mag,scale_factor,a_sq) tmp_result = _mm_mulhi_epi16(a_r,a_r); tmp_result = _mm_slli_epi16(tmp_result,1); tmp_result = _mm_mulhi_epi16(tmp_result,scale_factor); tmp_result = _mm_slli_epi16(tmp_result,1); tmp_result = _mm_mulhi_epi16(tmp_result,int_ch_mag); tmp_result = _mm_slli_epi16(tmp_result,1); tmp_result2 = _mm_mulhi_epi16(a_i,a_i); tmp_result2 = _mm_slli_epi16(tmp_result2,1); tmp_result2 = _mm_mulhi_epi16(tmp_result2,scale_factor); tmp_result2 = _mm_slli_epi16(tmp_result2,1); tmp_result2 = _mm_mulhi_epi16(tmp_result2,int_ch_mag); tmp_result2 = _mm_slli_epi16(tmp_result2,1); a_sq = _mm_adds_epi16(tmp_result,tmp_result2);

// calculates a_sq = int_ch_mag*(a_r^2 + a_i^2)*scale_factor for 64-QAM
#define square_a_64qam_epi16(a_r,a_i,int_ch_mag,scale_factor,a_sq)  tmp_result = _mm_mulhi_epi16(a_r,a_r); tmp_result = _mm_slli_epi16(tmp_result,1); tmp_result = _mm_mulhi_epi16(tmp_result,scale_factor); tmp_result = _mm_slli_epi16(tmp_result,3); tmp_result = _mm_mulhi_epi16(tmp_result,int_ch_mag); tmp_result = _mm_slli_epi16(tmp_result,1); tmp_result2 = _mm_mulhi_epi16(a_i,a_i); tmp_result2 = _mm_slli_epi16(tmp_result2,1); tmp_result2 = _mm_mulhi_epi16(tmp_result2,scale_factor); tmp_result2 = _mm_slli_epi16(tmp_result2,3); tmp_result2 = _mm_mulhi_epi16(tmp_result2,int_ch_mag); tmp_result2 = _mm_slli_epi16(tmp_result2,1); a_sq = _mm_adds_epi16(tmp_result,tmp_result2);

//==============================================================================================
// SINGLE-STREAM
//==============================================================================================

//----------------------------------------------------------------------------------------------
// QPSK
//----------------------------------------------------------------------------------------------

int dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
                   int **rxdataF_comp,
                   short *dlsch_llr,
                   unsigned char symbol,
                   uint8_t first_symbol_flag,
                   uint16_t nb_rb,
                   uint16_t pbch_pss_sss_adjust,
                   short **llr32p) {

  uint32_t *rxF = (uint32_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  uint32_t *llr32;
  int i,len;
  uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;

  if (first_symbol_flag==1) {
      llr32 = (uint32_t*)dlsch_llr;
  }
  else {
      llr32 = (uint32_t*)(*llr32p);
  }
 
  if (!llr32) {
      msg("dlsch_qpsk_llr: llr is null, symbol %d, llr32=%p\n",symbol, llr32);
      return(-1);
  } 


  if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
      if (frame_parms->mode1_flag==0)
          len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
      else
          len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
  }
  else {
      len = (nb_rb*12) - pbch_pss_sss_adjust;
  }
  //  printf("dlsch_qpsk_llr: symbol %d,len %d,pbch_pss_sss_adjust %d\n",symbol,len,pbch_pss_sss_adjust);
  for (i=0;i<len;i++) {
      *llr32 = *rxF;
      rxF++;
      llr32++;
  }

  *llr32p = (short *)llr32;

  _mm_empty();
  _m_empty();

  return(0);
}

//----------------------------------------------------------------------------------------------
// 16-QAM
//----------------------------------------------------------------------------------------------

void dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                     int **rxdataF_comp,
                     short *dlsch_llr,
                     int **dl_ch_mag,
                     unsigned char symbol,
                     uint8_t first_symbol_flag,
                     unsigned short nb_rb,
                     uint16_t pbch_pss_sss_adjust,
                     int16_t **llr32p) {

    __m128i *rxF = (__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    __m128i *ch_mag;
    __m128i llr128[2];
    int i,len;
    unsigned char symbol_mod,len_mod4=0;
    uint32_t *llr32;
    
    if (first_symbol_flag==1) {
        llr32 = (uint32_t*)dlsch_llr;
    }
    else {
        llr32 = (uint32_t*)*llr32p;
    }
  
    symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
    
    ch_mag = (__m128i*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    
    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        if (frame_parms->mode1_flag==0)
            len = nb_rb*8 - (2*pbch_pss_sss_adjust/3);
        else
            len = nb_rb*10 - (5*pbch_pss_sss_adjust/6);
    }
    else {
        len = nb_rb*12 - pbch_pss_sss_adjust;
    }
    
  // update output pointer according to number of REs in this symbol (<<2 because 4 bits per RE)
    if (first_symbol_flag == 1)
        *llr32p = dlsch_llr + (len<<2);
    else
        *llr32p += (len<<2);
    
    len_mod4 = len&3;
    len>>=2;  // length in quad words (4 REs)
    len+=(len_mod4==0 ? 0 : 1);
    
    for (i=0;i<len;i++) {
        
        xmm0 = _mm_abs_epi16(rxF[i]);
        xmm0 = _mm_subs_epi16(ch_mag[i],xmm0);

        // lambda_1=y_R, lambda_2=|y_R|-|h|^2, lamda_3=y_I, lambda_4=|y_I|-|h|^2
        llr128[0] = _mm_unpacklo_epi32(rxF[i],xmm0); 
        llr128[1] = _mm_unpackhi_epi32(rxF[i],xmm0);
        llr32[0] = _mm_extract_epi32(llr128[0],0); //((uint32_t *)&llr128[0])[0];
        llr32[1] = _mm_extract_epi32(llr128[0],1); //((uint32_t *)&llr128[0])[1];
        llr32[2] = _mm_extract_epi32(llr128[0],2); //((uint32_t *)&llr128[0])[2];
        llr32[3] = _mm_extract_epi32(llr128[0],3); //((uint32_t *)&llr128[0])[3];
        llr32[4] = _mm_extract_epi32(llr128[1],0); //((uint32_t *)&llr128[1])[0];
        llr32[5] = _mm_extract_epi32(llr128[1],1); //((uint32_t *)&llr128[1])[1];
        llr32[6] = _mm_extract_epi32(llr128[1],2); //((uint32_t *)&llr128[1])[2];
        llr32[7] = _mm_extract_epi32(llr128[1],3); //((uint32_t *)&llr128[1])[3];
        llr32+=8;
  }
  _mm_empty();
  _m_empty();
}

//----------------------------------------------------------------------------------------------
// 64-QAM
//----------------------------------------------------------------------------------------------

void dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                     int **rxdataF_comp,
                     short *dlsch_llr,
                     int **dl_ch_mag,
                     int **dl_ch_magb,
                     unsigned char symbol,
                     uint8_t first_symbol_flag,
                     unsigned short nb_rb,
                     uint16_t pbch_pss_sss_adjust,
                     short **llr_save) {

    __m128i *rxF = (__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    __m128i *ch_mag,*ch_magb;
    int i,len,len2;
    unsigned char symbol_mod,len_mod4;
    short *llr;
    int16_t *llr2;
    
    if (first_symbol_flag==1)
        llr = dlsch_llr;
    else
        llr = *llr_save;
    
    symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
    
    ch_mag = (__m128i*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    ch_magb = (__m128i*)&dl_ch_magb[0][(symbol*frame_parms->N_RB_DL*12)];

    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        if (frame_parms->mode1_flag==0)
            len = nb_rb*8 - (2*pbch_pss_sss_adjust/3);
        else
            len = nb_rb*10 - (5*pbch_pss_sss_adjust/6);
    }
    else {
        len = nb_rb*12 - pbch_pss_sss_adjust;
    }

    llr2 = llr;
    llr += (len*6);

    len_mod4 =len&3;
    len2=len>>2;  // length in quad words (4 REs)
    len2+=(len_mod4?0:1);

    for (i=0;i<len2;i++) {
        
        xmm1 = _mm_abs_epi16(rxF[i]);
        xmm1  = _mm_subs_epi16(ch_mag[i],xmm1);
        xmm2 = _mm_abs_epi16(xmm1);
        xmm2 = _mm_subs_epi16(ch_magb[i],xmm2);
        
        // loop over all LLRs in quad word (24 coded bits)
	/*
        for (j=0;j<8;j+=2) {
            llr2[0] = ((short *)&rxF[i])[j];
            llr2[1] = ((short *)&rxF[i])[j+1];
            llr2[2] = ((short *)&xmm1)[j];
            llr2[3] = ((short *)&xmm1)[j+1];
            llr2[4] = ((short *)&xmm2)[j];
            llr2[5] = ((short *)&xmm2)[j+1];
            
            llr2+=6;
        }
	*/
      llr2[0] = ((short *)&rxF[i])[0];
      llr2[1] = ((short *)&rxF[i])[1];
      llr2[2] = _mm_extract_epi16(xmm1,0);
      llr2[3] = _mm_extract_epi16(xmm1,1);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,0);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,1);//((short *)&xmm2)[j+1];
      
      llr2+=6;
      llr2[0] = ((short *)&rxF[i])[2];
      llr2[1] = ((short *)&rxF[i])[3];
      llr2[2] = _mm_extract_epi16(xmm1,2);
      llr2[3] = _mm_extract_epi16(xmm1,3);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,2);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,3);//((short *)&xmm2)[j+1];
      
      llr2+=6;
      llr2[0] = ((short *)&rxF[i])[4];
      llr2[1] = ((short *)&rxF[i])[5];
      llr2[2] = _mm_extract_epi16(xmm1,4);
      llr2[3] = _mm_extract_epi16(xmm1,5);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,4);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,5);//((short *)&xmm2)[j+1];
      
      llr2+=6;
      llr2[0] = ((short *)&rxF[i])[6];
      llr2[1] = ((short *)&rxF[i])[7];
      llr2[2] = _mm_extract_epi16(xmm1,6);
      llr2[3] = _mm_extract_epi16(xmm1,7);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,6);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,7);//((short *)&xmm2)[j+1];
      
      llr2+=6;

    }
    *llr_save = llr;
    _mm_empty();
    _m_empty();
}


//==============================================================================================
// DUAL-STREAM
//==============================================================================================

//----------------------------------------------------------------------------------------------
// QPSK
//----------------------------------------------------------------------------------------------

NOCYGWIN_STATIC __m128i  y0r_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0i_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y1r_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y1i_over2 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  A __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  B __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  C __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  D __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  E __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  F __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  G __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  H __attribute__ ((aligned(16))); 


int dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
                        int **rxdataF_comp,
                        int **rxdataF_comp_i,
                        int **rho_i,
                        short *dlsch_llr,
                        unsigned char symbol,
                        unsigned char first_symbol_flag,
                        unsigned short nb_rb,
                        uint16_t pbch_pss_sss_adjust,
                        short **llr16p) {
    
    int16_t *rxF=(int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i=(int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho=(int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
    
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
    
    if (!llr16) {
        msg("dlsch_qpsk_qpsk_llr: llr is null, symbol %d\n",symbol);
        return -1;
    }
    
    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }

    //    printf("qpsk_qpsk: len %d, llr16 %p\n",len,llr16);
    qpsk_qpsk((short *)rxF,
              (short *)rxF_i,
              (short *)llr16,
              (short *)rho,
              len);
    
    llr16 += (len<<1);
    *llr16p = (short *)llr16;
    
    return(0);
}

NOCYGWIN_STATIC __m128i ONE_OVER_SQRT_8 __attribute__((aligned(16))); 

void qpsk_qpsk(short *stream0_in,
               short *stream1_in, 
               short *stream0_out,
               short *rho01,
               int length
               ) {

/* 
This function computes the LLRs of stream 0 (s_0) in presence of the interfering stream 1 (s_1) assuming that both symbols are QPSK. It can be used for both MU-MIMO interference-aware receiver or for SU-MIMO receivers.

Parameters:
stream0_in = Matched filter output y0' = (h0*g0)*y0
stream1_in = Matched filter output y1' = (h0*g1)*y0
stream0_out = LLRs
rho01 = Correlation between the two effective channels \rho_{10} = (h1*g1)*(h0*g0)
length = number of resource elements
*/

    __m128i *rho01_128i = (__m128i *)rho01;
    __m128i *stream0_128i_in = (__m128i *)stream0_in;
    __m128i *stream1_128i_in = (__m128i *)stream1_in;
    __m128i *stream0_128i_out = (__m128i *)stream0_out;

#ifdef DEBUG_LLR
    print_shorts2("rho01_128i:\n",rho01_128i);
#endif

    int i;
    ONE_OVER_SQRT_8 = _mm_set1_epi16(23170); //round(2^16/sqrt(8))

    for (i=0;i<length>>2;i+=2) {
        // in each iteration, we take 8 complex samples

        xmm0 = rho01_128i[i]; // 4 symbols
        xmm1 = rho01_128i[i+1]; 
    
        // put (rho_r + rho_i)/2sqrt2 in rho_rpi
        // put (rho_r - rho_i)/2sqrt2 in rho_rmi
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)
         
        // divide by sqrt(8), no shift needed ONE_OVER_SQRT_8 = Q1.16
        rho_rpi = _mm_mulhi_epi16(rho_rpi,ONE_OVER_SQRT_8);
        rho_rmi = _mm_mulhi_epi16(rho_rmi,ONE_OVER_SQRT_8);

        // Compute LLR for first bit of stream 0
        
        // Compute real and imaginary parts of MF output for stream 0
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];

        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 

        y0r_over2  = _mm_srai_epi16(y0r,1);   // divide by 2                    
        y0i_over2  = _mm_srai_epi16(y0i,1);   // divide by 2 
    
        // Compute real and imaginary parts of MF output for stream 1
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];

        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]
        
        y1r_over2  = _mm_srai_epi16(y1r,1);   // divide by 2                    
        y1i_over2  = _mm_srai_epi16(y1i,1);   // divide by 2
    
        // Compute the terms for the LLR of first bit
    
        xmm0 = _mm_xor_si128(xmm0,xmm0); // ZERO
    
        // 1 term for nominator of LLR
        xmm3 = _mm_subs_epi16(y1r_over2,rho_rpi);
        A = _mm_abs_epi16(xmm3); // A = |y1r/2 - rho/sqrt(8)|
        xmm2 = _mm_adds_epi16(A,y0i_over2); // = |y1r/2 - rho/sqrt(8)| + y0i/2
        xmm3 = _mm_subs_epi16(y1i_over2,rho_rmi); 
        B = _mm_abs_epi16(xmm3); // B = |y1i/2 - rho*/sqrt(8)|
        logmax_num_re0 = _mm_adds_epi16(B,xmm2); // = |y1r/2 - rho/sqrt(8)|+|y1i/2 - rho*/sqrt(8)| + y0i/2

        // 2 term for nominator of LLR        
        xmm3 = _mm_subs_epi16(y1r_over2,rho_rmi); 
        C = _mm_abs_epi16(xmm3); // C = |y1r/2 - rho*/4|
        xmm2 = _mm_subs_epi16(C,y0i_over2); // = |y1r/2 - rho*/4| - y0i/2
        xmm3 = _mm_adds_epi16(y1i_over2,rho_rpi); 
        D = _mm_abs_epi16(xmm3); // D = |y1i/2 + rho/4|      
        xmm2 = _mm_adds_epi16(xmm2,D); // |y1r/2 - rho*/4| + |y1i/2 + rho/4| - y0i/2
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0,xmm2); // max, numerator done

        // 1 term for denominator of LLR
        xmm3 = _mm_adds_epi16(y1r_over2,rho_rmi); 
        E = _mm_abs_epi16(xmm3); // E = |y1r/2 + rho*/4|
        xmm2 = _mm_adds_epi16(E,y0i_over2); // = |y1r/2 + rho*/4| + y0i/2
        xmm3 = _mm_subs_epi16(y1i_over2,rho_rpi); 
        F = _mm_abs_epi16(xmm3); // F = |y1i/2 - rho/4|
        logmax_den_re0 = _mm_adds_epi16(F,xmm2); // = |y1r/2 + rho*/4| + |y1i/2 - rho/4| + y0i/2

        // 2 term for denominator of LLR
        xmm3 = _mm_adds_epi16(y1r_over2,rho_rpi); 
        G = _mm_abs_epi16(xmm3); // G = |y1r/2 + rho/4|      
        xmm2 = _mm_subs_epi16(G,y0i_over2); // = |y1r/2 + rho/4| - y0i/2
        xmm3 = _mm_adds_epi16(y1i_over2,rho_rmi); 
        H = _mm_abs_epi16(xmm3); // H = |y1i/2 + rho*/4|
        xmm2 = _mm_adds_epi16(xmm2,H); // = |y1r/2 + rho/4| + |y1i/2 + rho*/4| - y0i/2    
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0,xmm2); // max, denominator done

        // Compute the terms for the LLR of first bit

        // 1 term for nominator of LLR    
        xmm2 = _mm_adds_epi16(A,y0r_over2); 
        logmax_num_im0 = _mm_adds_epi16(B,xmm2); // = |y1r/2 - rho/4| + |y1i/2 - rho*/4| + y0r/2
 
        // 2 term for nominator of LLR
        xmm2 = _mm_subs_epi16(E,y0r_over2); 
        xmm2 = _mm_adds_epi16(xmm2,F); // = |y1r/2 + rho*/4| + |y1i/2 - rho/4| - y0r/2

        logmax_num_im0 = _mm_max_epi16(logmax_num_im0,xmm2); // max, nominator done
    
        // 1 term for denominator of LLR
        xmm2 = _mm_adds_epi16(C,y0r_over2); 
        logmax_den_im0 = _mm_adds_epi16(D,xmm2); // = |y1r/2 - rho*/4| + |y1i/2 + rho/4| - y0r/2

        xmm2 = _mm_subs_epi16(G,y0r_over2); 
        xmm2 = _mm_adds_epi16(xmm2,H); // = |y1r/2 + rho/4| + |y1i/2 + rho*/4| - y0r/2
    
        logmax_den_im0 = _mm_max_epi16(logmax_den_im0,xmm2); // max, denominator done

        // LLR of first bit [L1(1), L1(2), L1(3), L1(4)]
        y0r = _mm_adds_epi16(y0r,logmax_num_re0);
        y0r = _mm_subs_epi16(y0r,logmax_den_re0);
    
        // LLR of second bit [L2(1), L2(2), L2(3), L2(4)]
        y0i = _mm_adds_epi16(y0i,logmax_num_im0);
        y0i = _mm_subs_epi16(y0i,logmax_den_im0);

	_mm_storeu_si128(&stream0_128i_out[i],_mm_unpacklo_epi16(y0r,y0i)); // = [L1(1), L2(1), L1(2), L2(2)]
	if (i<((length>>1) - 1)) // false if only 2 REs remain
	  _mm_storeu_si128(&stream0_128i_out[i+1],_mm_unpackhi_epi16(y0r,y0i));
	
    }
    _mm_empty();
    _m_empty();
}

int dlsch_qpsk_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                         int **rxdataF_comp,
                         int **rxdataF_comp_i,
                         int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
                         int **rho_i,
                         short *dlsch_llr,
                         unsigned char symbol,
                         unsigned char first_symbol_flag,
                         unsigned short nb_rb,
                         uint16_t pbch_pss_sss_adjust,
                         short **llr16p) {
    
    int16_t *rxF=(int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i=(int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag_i = (int16_t*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho=(int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
    
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
    
    if (!llr16) {
        msg("dlsch_qpsk_qpsk_llr: llr is null, symbol %d\n",symbol);
        return -1;
    }
    
    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }
    
    qpsk_qam16((short *)rxF,
               (short *)rxF_i,
               (short *)ch_mag_i,
               (short *)llr16,
               (short *)rho,
               len);
    
    llr16 += (len<<1);
    *llr16p = (short *)llr16;
    
    return(0);
}

NOCYGWIN_STATIC __m128i ONE_OVER_SQRT_2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ONE_OVER_SQRT_10 __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i THREE_OVER_SQRT_10 __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i ONE_OVER_SQRT_10_Q15 __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i SQRT_10_OVER_FOUR __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i ch_mag_int __attribute__((aligned(16))); 

void qpsk_qam16(short *stream0_in,
                short *stream1_in, 
                short *ch_mag_i,
                short *stream0_out,
                short *rho01,
                int length
                ) {

/* 
This function computes the LLRs of stream 0 (s_0) in presence of the interfering stream 1 (s_1) assuming that both symbols are QPSK. It can be used for both MU-MIMO interference-aware receiver or for SU-MIMO receivers.

Parameters:
stream0_in = Matched filter output y0' = (h0*g0)*y0
stream1_in = Matched filter output y1' = (h0*g1)*y0
stream0_out = LLRs
rho01 = Correlation between the two effective channels \rho_{10} = (h1*g1)*(h0*g0)
length = number of resource elements
*/

    __m128i *rho01_128i = (__m128i *)rho01;
    __m128i *stream0_128i_in = (__m128i *)stream0_in;
    __m128i *stream1_128i_in = (__m128i *)stream1_in;
    __m128i *stream0_128i_out = (__m128i *)stream0_out;
    __m128i *ch_mag_128i_i    = (__m128i *)ch_mag_i;

#ifdef DEBUG_LLR
    print_shorts2("rho01_128i:\n",rho01_128i);
#endif

    int i;

    ONE_OVER_SQRT_2 = _mm_set1_epi16(23170); // round(1/sqrt(2)*2^15)
    ONE_OVER_SQRT_10_Q15 = _mm_set1_epi16(10362); // round(1/sqrt(10)*2^15)
    THREE_OVER_SQRT_10 = _mm_set1_epi16(31086); // round(3/sqrt(10)*2^15)
    SQRT_10_OVER_FOUR = _mm_set1_epi16(25905); // round(sqrt(10)/4*2^15)

    for (i=0;i<length>>2;i+=2) {
        // in each iteration, we take 8 complex samples

        xmm0 = rho01_128i[i]; // 4 symbols
        xmm1 = rho01_128i[i+1]; 
    
        // put (rho_r + rho_i)/2sqrt2 in rho_rpi
        // put (rho_r - rho_i)/2sqrt2 in rho_rmi
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)
        
        // divide by sqrt(2)
        rho_rpi = _mm_mulhi_epi16(rho_rpi, ONE_OVER_SQRT_2);
        rho_rmi = _mm_mulhi_epi16(rho_rmi, ONE_OVER_SQRT_2);
        rho_rpi = _mm_slli_epi16(rho_rpi,1);
        rho_rmi = _mm_slli_epi16(rho_rmi,1);
 
        // Compute LLR for first bit of stream 0
        
        // Compute real and imaginary parts of MF output for stream 0
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];

        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 

        // divide by sqrt(2)
        y0r_over2 = _mm_mulhi_epi16(y0r, ONE_OVER_SQRT_2);
        y0i_over2 = _mm_mulhi_epi16(y0i, ONE_OVER_SQRT_2);
        y0r_over2  = _mm_slli_epi16(y0r,1);
        y0i_over2  = _mm_slli_epi16(y0i,1);
    
        y0_p_1_1 = _mm_adds_epi16(y0r_over2, y0i_over2);
        y0_m_1_1 = _mm_subs_epi16(y0r_over2, y0i_over2);

        // Compute real and imaginary parts of MF output for stream 1
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];

        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]
    
        xmm0 = _mm_xor_si128(xmm0,xmm0); // ZERO
    
        // compute psi
        xmm3 = _mm_subs_epi16(y1r,rho_rpi);
        psi_r_p1_p1 = _mm_abs_epi16(xmm3);    
        xmm3 = _mm_subs_epi16(y1i,rho_rmi);
        psi_i_p1_p1 = _mm_abs_epi16(xmm3);     
        xmm3 = _mm_subs_epi16(y1r,rho_rmi);
        psi_r_p1_m1 = _mm_abs_epi16(xmm3);           
        xmm3 = _mm_adds_epi16(y1i,rho_rpi);
        psi_i_p1_m1 = _mm_abs_epi16(xmm3);     
        xmm3 = _mm_adds_epi16(y1r,rho_rmi);
        psi_r_m1_p1 = _mm_abs_epi16(xmm3);     
        xmm3 = _mm_subs_epi16(y1i,rho_rpi);
        psi_i_m1_p1 = _mm_abs_epi16(xmm3); 
        xmm3 = _mm_adds_epi16(y1r,rho_rpi);
        psi_r_m1_m1 = _mm_abs_epi16(xmm3);         
        xmm3 = _mm_adds_epi16(y1i,rho_rmi);
        psi_i_m1_m1 = _mm_abs_epi16(xmm3);      
    
        // Rearrange interfering channel magnitudes
        xmm2 = ch_mag_128i_i[i];   
        xmm3 = ch_mag_128i_i[i+1]; 

        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_int = _mm_unpacklo_epi64(xmm2,xmm3); 
        
        // calculate optimal interference amplitudes		
        interference_abs_epi16(psi_r_p1_p1 , ch_mag_int, a_r_p1_p1 , ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_p1 , ch_mag_int, a_i_p1_p1 , ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);	
        interference_abs_epi16(psi_r_p1_m1 , ch_mag_int, a_r_p1_m1 , ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_m1 , ch_mag_int, a_i_p1_m1 , ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);	
        interference_abs_epi16(psi_r_m1_p1 , ch_mag_int, a_r_m1_p1 , ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_p1 , ch_mag_int, a_i_m1_p1 , ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_m1 , ch_mag_int, a_r_m1_m1 , ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_m1 , ch_mag_int, a_i_m1_m1 , ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        
        // prodsum
        prodsum_psi_a_epi16(psi_r_p1_p1, a_r_p1_p1, psi_i_p1_p1, a_i_p1_p1, psi_a_p1_p1);
        prodsum_psi_a_epi16(psi_r_p1_m1, a_r_p1_m1, psi_i_p1_m1, a_i_p1_m1, psi_a_p1_m1);
        prodsum_psi_a_epi16(psi_r_m1_p1, a_r_m1_p1, psi_i_m1_p1, a_i_m1_p1, psi_a_m1_p1);
        prodsum_psi_a_epi16(psi_r_m1_m1, a_r_m1_m1, psi_i_m1_m1, a_i_m1_m1, psi_a_m1_m1);
        
        // squares
        square_a_epi16(a_r_p1_p1, a_i_p1_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_p1);
        square_a_epi16(a_r_p1_m1, a_i_p1_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_m1);
        square_a_epi16(a_r_m1_p1, a_i_m1_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_p1);
        square_a_epi16(a_r_m1_m1, a_i_m1_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_m1);

       // Computing Metrics
        xmm0 = _mm_subs_epi16(psi_a_p1_p1, a_sq_p1_p1);
        bit_met_p1_p1 = _mm_adds_epi16(xmm0, y0_p_1_1);
        
        xmm0 = _mm_subs_epi16(psi_a_p1_m1, a_sq_p1_m1);
        bit_met_p1_m1 = _mm_adds_epi16(xmm0, y0_m_1_1);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_p1, a_sq_m1_p1);
        bit_met_m1_p1 = _mm_subs_epi16(xmm0, y0_m_1_1);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_m1, a_sq_m1_m1);
        bit_met_m1_m1 = _mm_subs_epi16(xmm0, y0_p_1_1);
        
        // MSB
        logmax_num_re0 = _mm_max_epi16(bit_met_p1_p1,bit_met_p1_m1); // bit=0
        logmax_den_re0 = _mm_max_epi16(bit_met_m1_p1,bit_met_m1_m1); // bit=1
        
        y0r = _mm_subs_epi16(logmax_num_re0,logmax_den_re0);
        
        // LSB
        logmax_num_im0 = _mm_max_epi16(bit_met_p1_p1,bit_met_m1_p1); // bit=0
        logmax_den_im0 = _mm_max_epi16(bit_met_p1_m1,bit_met_m1_m1); // bit=1
        
        y0i = _mm_subs_epi16(logmax_num_im0,logmax_den_im0);
        
        stream0_128i_out[i] = _mm_unpacklo_epi16(y0r,y0i); // = [L1(1), L2(1), L1(2), L2(2)]
        if (i<((length>>1) - 1)) // false if only 2 REs remain
            stream0_128i_out[i+1] = _mm_unpackhi_epi16(y0r,y0i);
   
    }
    _mm_empty();
    _m_empty();
}

int dlsch_qpsk_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                         int **rxdataF_comp,
                         int **rxdataF_comp_i,
                         int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
                         int **rho_i,
                         short *dlsch_llr,
                         unsigned char symbol,
                         unsigned char first_symbol_flag,
                         unsigned short nb_rb,
                         uint16_t pbch_pss_sss_adjust,
                         short **llr16p) {
    
    int16_t *rxF=(int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i=(int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag_i = (int16_t*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho=(int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
    
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
    
    if (!llr16) {
        msg("dlsch_qpsk_qam64_llr: llr is null, symbol %d\n",symbol);
        return -1;
    }
    
    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }
    
    qpsk_qam64((short *)rxF,
               (short *)rxF_i,
               (short *)ch_mag_i,
               (short *)llr16,
               (short *)rho,
               len);
    
    llr16 += (len<<1);
    *llr16p = (short *)llr16;
    
    return(0);
}

NOCYGWIN_STATIC __m128i ONE_OVER_SQRT_2_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i THREE_OVER_SQRT_2_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i FIVE_OVER_SQRT_2_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i SEVEN_OVER_SQRT_2_42 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m128i ch_mag_int_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i two_ch_mag_int_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i three_ch_mag_int_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i SQRT_42_OVER_FOUR __attribute__((aligned(16))); 

void qpsk_qam64(short *stream0_in,
                short *stream1_in, 
                short *ch_mag_i,
                short *stream0_out,
                short *rho01,
                int length
                ) {

/* 
This function computes the LLRs of stream 0 (s_0) in presence of the interfering stream 1 (s_1) assuming that both symbols are QPSK. It can be used for both MU-MIMO interference-aware receiver or for SU-MIMO receivers.

Parameters:
stream0_in = Matched filter output y0' = (h0*g0)*y0
stream1_in = Matched filter output y1' = (h0*g1)*y0
stream0_out = LLRs
rho01 = Correlation between the two effective channels \rho_{10} = (h1*g1)*(h0*g0)
length = number of resource elements
*/

    __m128i *rho01_128i = (__m128i *)rho01;
    __m128i *stream0_128i_in = (__m128i *)stream0_in;
    __m128i *stream1_128i_in = (__m128i *)stream1_in;
    __m128i *stream0_128i_out = (__m128i *)stream0_out;
    __m128i *ch_mag_128i_i    = (__m128i *)ch_mag_i;

#ifdef DEBUG_LLR
    print_shorts2("rho01_128i:\n",rho01_128i);
#endif

    int i;

    ONE_OVER_SQRT_2 = _mm_set1_epi16(23170); // round(1/sqrt(2)*2^15)
    ONE_OVER_SQRT_2_42 = _mm_set1_epi16(3575); // round(1/sqrt(2*42)*2^15)
    THREE_OVER_SQRT_2_42 = _mm_set1_epi16(10726); // round(3/sqrt(2*42)*2^15)
    FIVE_OVER_SQRT_2_42 = _mm_set1_epi16(17876); // round(5/sqrt(2*42)*2^15)
    SEVEN_OVER_SQRT_2_42 = _mm_set1_epi16(25027); // round(7/sqrt(2*42)*2^15)
    SQRT_42_OVER_FOUR = _mm_set1_epi16(13272); // round(sqrt(42)/4*2^13), Q3.12

    for (i=0;i<length>>2;i+=2) {
        // in each iteration, we take 8 complex samples

        xmm0 = rho01_128i[i]; // 4 symbols
        xmm1 = rho01_128i[i+1]; 
    
        // put (rho_r + rho_i)/sqrt2 in rho_rpi
        // put (rho_r - rho_i)/sqrt2 in rho_rmi
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)
        
        // divide by sqrt(2)
        rho_rpi = _mm_mulhi_epi16(rho_rpi, ONE_OVER_SQRT_2);
        rho_rmi = _mm_mulhi_epi16(rho_rmi, ONE_OVER_SQRT_2);
        rho_rpi = _mm_slli_epi16(rho_rpi,1);
        rho_rmi = _mm_slli_epi16(rho_rmi,1);
 
        // Compute LLR for first bit of stream 0
        
        // Compute real and imaginary parts of MF output for stream 0
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];

        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 

        // divide by sqrt(2)
        y0r_over2 = _mm_mulhi_epi16(y0r, ONE_OVER_SQRT_2);
        y0i_over2 = _mm_mulhi_epi16(y0i, ONE_OVER_SQRT_2);
        y0r_over2  = _mm_slli_epi16(y0r,1);
        y0i_over2  = _mm_slli_epi16(y0i,1);
    
        y0_p_1_1 = _mm_adds_epi16(y0r_over2, y0i_over2);
        y0_m_1_1 = _mm_subs_epi16(y0r_over2, y0i_over2);

        // Compute real and imaginary parts of MF output for stream 1
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];

        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]
    
        xmm0 = _mm_xor_si128(xmm0,xmm0); // ZERO
    
        // compute psi
        xmm3 = _mm_subs_epi16(y1r,rho_rpi);
        psi_r_p1_p1 = _mm_abs_epi16(xmm3);    
        xmm3 = _mm_subs_epi16(y1i,rho_rmi);
        psi_i_p1_p1 = _mm_abs_epi16(xmm3);     
        xmm3 = _mm_subs_epi16(y1r,rho_rmi);
        psi_r_p1_m1 = _mm_abs_epi16(xmm3);           
        xmm3 = _mm_adds_epi16(y1i,rho_rpi);
        psi_i_p1_m1 = _mm_abs_epi16(xmm3);     
        xmm3 = _mm_adds_epi16(y1r,rho_rmi);
        psi_r_m1_p1 = _mm_abs_epi16(xmm3);     
        xmm3 = _mm_subs_epi16(y1i,rho_rpi);
        psi_i_m1_p1 = _mm_abs_epi16(xmm3); 
        xmm3 = _mm_adds_epi16(y1r,rho_rpi);
        psi_r_m1_m1 = _mm_abs_epi16(xmm3);         
        xmm3 = _mm_adds_epi16(y1i,rho_rmi);
        psi_i_m1_m1 = _mm_abs_epi16(xmm3);      
    
        // Rearrange interfering channel magnitudes
        xmm2 = ch_mag_128i_i[i];   
        xmm3 = ch_mag_128i_i[i+1]; 

        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_int = _mm_unpacklo_epi64(xmm2,xmm3); 
        ch_mag_int_with_sigma2       = _mm_srai_epi16(ch_mag_int, 1); // *2
        two_ch_mag_int_with_sigma2   = ch_mag_int; // *4
        three_ch_mag_int_with_sigma2 = _mm_adds_epi16(ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2); // *6

        interference_abs_64qam_epi16(psi_r_p1_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);    
        interference_abs_64qam_epi16(psi_i_p1_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);    
        interference_abs_64qam_epi16(psi_r_p1_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);    
        interference_abs_64qam_epi16(psi_i_p1_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);    
        interference_abs_64qam_epi16(psi_r_m1_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);    
        interference_abs_64qam_epi16(psi_i_m1_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);    
        interference_abs_64qam_epi16(psi_r_m1_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);    
        interference_abs_64qam_epi16(psi_i_m1_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);    
        
        // prodsum
        prodsum_psi_a_epi16(psi_r_p1_p1, a_r_p1_p1, psi_i_p1_p1, a_i_p1_p1, psi_a_p1_p1);
        prodsum_psi_a_epi16(psi_r_p1_m1, a_r_p1_m1, psi_i_p1_m1, a_i_p1_m1, psi_a_p1_m1);
        prodsum_psi_a_epi16(psi_r_m1_p1, a_r_m1_p1, psi_i_m1_p1, a_i_m1_p1, psi_a_m1_p1);
        prodsum_psi_a_epi16(psi_r_m1_m1, a_r_m1_m1, psi_i_m1_m1, a_i_m1_m1, psi_a_m1_m1);

        // Multiply by sqrt(2)
        psi_a_p1_p1 = _mm_mulhi_epi16(psi_a_p1_p1, ONE_OVER_SQRT_2);
        psi_a_p1_p1 = _mm_slli_epi16(psi_a_p1_p1, 2);
        psi_a_p1_m1 = _mm_mulhi_epi16(psi_a_p1_m1, ONE_OVER_SQRT_2);
        psi_a_p1_m1 = _mm_slli_epi16(psi_a_p1_m1, 2);
        psi_a_m1_p1 = _mm_mulhi_epi16(psi_a_m1_p1, ONE_OVER_SQRT_2);
        psi_a_m1_p1 = _mm_slli_epi16(psi_a_m1_p1, 2);
        psi_a_m1_m1 = _mm_mulhi_epi16(psi_a_m1_m1, ONE_OVER_SQRT_2);
        psi_a_m1_m1 = _mm_slli_epi16(psi_a_m1_m1, 2);
        
        square_a_64qam_epi16(a_r_p1_p1, a_i_p1_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p1);
        square_a_64qam_epi16(a_r_p1_m1, a_i_p1_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m1);
        square_a_64qam_epi16(a_r_m1_p1, a_i_m1_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p1);
        square_a_64qam_epi16(a_r_m1_m1, a_i_m1_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m1);

        // Computing Metrics
        xmm0 = _mm_subs_epi16(psi_a_p1_p1, a_sq_p1_p1);
        bit_met_p1_p1 = _mm_adds_epi16(xmm0, y0_p_1_1);
        
        xmm0 = _mm_subs_epi16(psi_a_p1_m1, a_sq_p1_m1);
        bit_met_p1_m1 = _mm_adds_epi16(xmm0, y0_m_1_1);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_p1, a_sq_m1_p1);
        bit_met_m1_p1 = _mm_subs_epi16(xmm0, y0_m_1_1);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_m1, a_sq_m1_m1);
        bit_met_m1_m1 = _mm_subs_epi16(xmm0, y0_p_1_1);
        
        // MSB
        logmax_num_re0 = _mm_max_epi16(bit_met_p1_p1,bit_met_p1_m1); // bit=0
        logmax_den_re0 = _mm_max_epi16(bit_met_m1_p1,bit_met_m1_m1); // bit=1
        
        y0r = _mm_subs_epi16(logmax_num_re0,logmax_den_re0);
        
        // LSB
        logmax_num_im0 = _mm_max_epi16(bit_met_p1_p1,bit_met_m1_p1); // bit=0
        logmax_den_im0 = _mm_max_epi16(bit_met_p1_m1,bit_met_m1_m1); // bit=1
        
        y0i = _mm_subs_epi16(logmax_num_im0,logmax_den_im0);
        
        stream0_128i_out[i] = _mm_unpacklo_epi16(y0r,y0i); // = [L1(1), L2(1), L1(2), L2(2)]
        if (i<((length>>1) - 1)) // false if only 2 REs remain
            stream0_128i_out[i+1] = _mm_unpackhi_epi16(y0r,y0i);
   
    }
    _mm_empty();
    _m_empty();
}


//----------------------------------------------------------------------------------------------
// 16-QAM
//----------------------------------------------------------------------------------------------

NOCYGWIN_STATIC __m128i ONE_OVER_TWO_SQRT_10 __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i NINE_OVER_TWO_SQRT_10 __attribute__((aligned(16)));

NOCYGWIN_STATIC __m128i  y0r_over_sqrt10 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0i_over_sqrt10 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0r_three_over_sqrt10 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0i_three_over_sqrt10 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i ch_mag_des __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_over_10 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_over_2 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_9_over_10 __attribute__ ((aligned(16))); 

void qam16_qpsk(short *stream0_in,
                short *stream1_in,
                short *ch_mag,
                short *stream0_out,
                short *rho01,
                int length
                ) {

/* 
   Author: Sebastian Wagner
   Date: 2012-06-04

   Input: 
   stream0_in:  MF filter for 1st stream, i.e., y0=h0'*y
   stream!_in:  MF filter for 2nd stream, i.e., y1=h1'*y
   ch_mag:      2*h0/sqrt(00), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   ch_mag_i:    2*h1/sqrt(00), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   rho01:       Channel cross correlation, i.e., h1'*h0
   
   Output:
   stream0_out: output LLRs for 1st stream
*/

    __m128i *rho01_128i       = (__m128i *)rho01;
    __m128i *stream0_128i_in  = (__m128i *)stream0_in;
    __m128i *stream1_128i_in  = (__m128i *)stream1_in;
    __m128i *stream0_128i_out = (__m128i *)stream0_out;
    __m128i *ch_mag_128i      = (__m128i *)ch_mag;

    int i;

    ONE_OVER_SQRT_2 = _mm_set1_epi16(23170); // round(1/sqrt(2)*2^15)
    ONE_OVER_SQRT_10 = _mm_set1_epi16(20724); // round(1/sqrt(10)*2^16)
    THREE_OVER_SQRT_10 = _mm_set1_epi16(31086); // round(3/sqrt(10)*2^15)
    SQRT_10_OVER_FOUR = _mm_set1_epi16(25905); // round(sqrt(10)/4*2^15)
    ONE_OVER_TWO_SQRT_10 = _mm_set1_epi16(10362); // round(1/2/sqrt(10)*2^16)
    NINE_OVER_TWO_SQRT_10 = _mm_set1_epi16(23315); // round(9/2/sqrt(10)*2^14)

    for (i=0;i<length>>2;i+=2) {
        // In one iteration, we deal with 8 REs

        // Get rho
        xmm0 = rho01_128i[i];
        xmm1 = rho01_128i[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1 = _mm_mulhi_epi16(rho_rpi,ONE_OVER_SQRT_10);
        rho_rmi_1_1 = _mm_mulhi_epi16(rho_rmi,ONE_OVER_SQRT_10);
        rho_rpi_3_3 = _mm_mulhi_epi16(rho_rpi,THREE_OVER_SQRT_10);
        rho_rmi_3_3 = _mm_mulhi_epi16(rho_rmi,THREE_OVER_SQRT_10);
        rho_rpi_3_3 = _mm_slli_epi16(rho_rpi_3_3,1);
        rho_rmi_3_3 = _mm_slli_epi16(rho_rmi_3_3,1);

        xmm4 = _mm_mulhi_epi16(xmm2,ONE_OVER_SQRT_10); // Re(rho)
        xmm5 = _mm_mulhi_epi16(xmm3,THREE_OVER_SQRT_10); // Im(rho)
        xmm5 = _mm_slli_epi16(xmm5,1);

        rho_rpi_1_3 = _mm_adds_epi16(xmm4,xmm5);
        rho_rmi_1_3 = _mm_subs_epi16(xmm4,xmm5);

        xmm6 = _mm_mulhi_epi16(xmm2,THREE_OVER_SQRT_10); // Re(rho)
        xmm7 = _mm_mulhi_epi16(xmm3,ONE_OVER_SQRT_10); // Im(rho)
        xmm6 = _mm_slli_epi16(xmm6,1);

        rho_rpi_3_1 = _mm_adds_epi16(xmm6,xmm7);
        rho_rmi_3_1 = _mm_subs_epi16(xmm6,xmm7);

        // Rearrange interfering MF output
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]

        xmm0 = _mm_xor_si128(xmm0,xmm0); // ZERO
        xmm2 = _mm_subs_epi16(rho_rpi_1_1,y1r); // = [Re(rho)+ Im(rho)]/sqrt(10) - y1r
        psi_r_p1_p1 = _mm_abs_epi16(xmm2); // = |[Re(rho)+ Im(rho)]/sqrt(10) - y1r|

        xmm2= _mm_subs_epi16(rho_rmi_1_1,y1r);       psi_r_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_1,y1i);       psi_i_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_1_3,y1r);       psi_r_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_3,y1r);       psi_r_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_1,y1i);       psi_i_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_1,y1r);       psi_r_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_1,y1r);       psi_r_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_3,y1i);       psi_i_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_3,y1r);       psi_r_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_3,y1r);       psi_r_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_3,y1i);       psi_i_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_1_1,y1i);       psi_i_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_1,y1i);       psi_i_m1_p3 = _mm_abs_epi16(xmm2);        
        xmm2= _mm_subs_epi16(rho_rpi_1_3,y1i);       psi_i_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_3,y1i);       psi_i_m3_p3 = _mm_abs_epi16(xmm2);    
        xmm2= _mm_adds_epi16(rho_rpi_1_1,y1i);       psi_i_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_1,y1i);       psi_i_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_3,y1i);       psi_i_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_3,y1i);       psi_i_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_1,y1r);       psi_r_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_3,y1r);       psi_r_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_1,y1r);       psi_r_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_3,y1r);       psi_r_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_1_1);       psi_r_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_1_3);       psi_r_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_1_1);       psi_i_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_3_1);       psi_i_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_3_1);       psi_r_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_3_3);       psi_r_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_1_3);       psi_i_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_3_3);       psi_i_m3_m3 = _mm_abs_epi16(xmm2);

        // Rearrange desired MF output
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 
    
        // Rearrange desired channel magnitudes
        xmm2 = ch_mag_128i[i]; // = [|h|^2(1),|h|^2(1),|h|^2(2),|h|^2(2)]*(2/sqrt(10))
        xmm3 = ch_mag_128i[i+1]; // = [|h|^2(3),|h|^2(3),|h|^2(4),|h|^2(4)]*(2/sqrt(10))
        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_des = _mm_unpacklo_epi64(xmm2,xmm3); // = [|h|^2(1),|h|^2(2),|h|^2(3),|h|^2(4)]*(2/sqrt(10))

        // Scale MF output of desired signal
        y0r_over_sqrt10 = _mm_mulhi_epi16(y0r,ONE_OVER_SQRT_10);
        y0i_over_sqrt10 = _mm_mulhi_epi16(y0i,ONE_OVER_SQRT_10);
        y0r_three_over_sqrt10 = _mm_mulhi_epi16(y0r,THREE_OVER_SQRT_10);
        y0i_three_over_sqrt10 = _mm_mulhi_epi16(y0i,THREE_OVER_SQRT_10);
        y0r_three_over_sqrt10 = _mm_slli_epi16(y0r_three_over_sqrt10,1);
        y0i_three_over_sqrt10 = _mm_slli_epi16(y0i_three_over_sqrt10,1);

        // Compute necessary combination of required terms
        y0_p_1_1 = _mm_adds_epi16(y0r_over_sqrt10,y0i_over_sqrt10);
        y0_m_1_1 = _mm_subs_epi16(y0r_over_sqrt10,y0i_over_sqrt10);  

        y0_p_1_3 = _mm_adds_epi16(y0r_over_sqrt10,y0i_three_over_sqrt10);
        y0_m_1_3 = _mm_subs_epi16(y0r_over_sqrt10,y0i_three_over_sqrt10);

        y0_p_3_1 = _mm_adds_epi16(y0r_three_over_sqrt10,y0i_over_sqrt10);
        y0_m_3_1 = _mm_subs_epi16(y0r_three_over_sqrt10,y0i_over_sqrt10);

        y0_p_3_3 = _mm_adds_epi16(y0r_three_over_sqrt10,y0i_three_over_sqrt10);
        y0_m_3_3 = _mm_subs_epi16(y0r_three_over_sqrt10,y0i_three_over_sqrt10);

        // Add psi
        psi_a_p1_p1 = _mm_adds_epi16(psi_r_p1_p1 ,psi_i_p1_p1);
        psi_a_p1_p3 = _mm_adds_epi16(psi_r_p1_p3 ,psi_i_p1_p3);
        psi_a_p3_p1 = _mm_adds_epi16(psi_r_p3_p1 ,psi_i_p3_p1);
        psi_a_p3_p3 = _mm_adds_epi16(psi_r_p3_p3 ,psi_i_p3_p3);
        psi_a_p1_m1 = _mm_adds_epi16(psi_r_p1_m1 ,psi_i_p1_m1);
        psi_a_p1_m3 = _mm_adds_epi16(psi_r_p1_m3 ,psi_i_p1_m3);      
        psi_a_p3_m1 = _mm_adds_epi16(psi_r_p3_m1 ,psi_i_p3_m1);
        psi_a_p3_m3 = _mm_adds_epi16(psi_r_p3_m3 ,psi_i_p3_m3);
        psi_a_m1_p1 = _mm_adds_epi16(psi_r_m1_p1 ,psi_i_m1_p1);
        psi_a_m1_p3 = _mm_adds_epi16(psi_r_m1_p3 ,psi_i_m1_p3);
        psi_a_m3_p1 = _mm_adds_epi16(psi_r_m3_p1 ,psi_i_m3_p1);
        psi_a_m3_p3 = _mm_adds_epi16(psi_r_m3_p3 ,psi_i_m3_p3);
        psi_a_m1_m1 = _mm_adds_epi16(psi_r_m1_m1 ,psi_i_m1_m1);
        psi_a_m1_m3 = _mm_adds_epi16(psi_r_m1_m3 ,psi_i_m1_m3);
        psi_a_m3_m1 = _mm_adds_epi16(psi_r_m3_m1 ,psi_i_m3_m1);
        psi_a_m3_m3 = _mm_adds_epi16(psi_r_m3_m3 ,psi_i_m3_m3);
        
        // scale by sqrt(2)
        psi_a_p1_p1 = _mm_mulhi_epi16(psi_a_p1_p1,ONE_OVER_SQRT_2);
        psi_a_p1_p1 = _mm_slli_epi16(psi_a_p1_p1,1);
        psi_a_p1_p3 = _mm_mulhi_epi16(psi_a_p1_p3,ONE_OVER_SQRT_2);
        psi_a_p1_p3 = _mm_slli_epi16(psi_a_p1_p3,1);        
        psi_a_p3_p1 = _mm_mulhi_epi16(psi_a_p3_p1,ONE_OVER_SQRT_2);
        psi_a_p3_p1 = _mm_slli_epi16(psi_a_p3_p1,1);
        psi_a_p3_p3 = _mm_mulhi_epi16(psi_a_p3_p3,ONE_OVER_SQRT_2);
        psi_a_p3_p3 = _mm_slli_epi16(psi_a_p3_p3,1);
    
        psi_a_p1_m1 = _mm_mulhi_epi16(psi_a_p1_m1,ONE_OVER_SQRT_2);
        psi_a_p1_m1 = _mm_slli_epi16(psi_a_p1_m1,1);
        psi_a_p1_m3 = _mm_mulhi_epi16(psi_a_p1_m3,ONE_OVER_SQRT_2);
        psi_a_p1_m3 = _mm_slli_epi16(psi_a_p1_m3,1);
        psi_a_p3_m1 = _mm_mulhi_epi16(psi_a_p3_m1,ONE_OVER_SQRT_2);
        psi_a_p3_m1 = _mm_slli_epi16(psi_a_p3_m1,1);
        psi_a_p3_m3 = _mm_mulhi_epi16(psi_a_p3_m3,ONE_OVER_SQRT_2);
        psi_a_p3_m3 = _mm_slli_epi16(psi_a_p3_m3,1);
        
        psi_a_m1_p1 = _mm_mulhi_epi16(psi_a_m1_p1,ONE_OVER_SQRT_2);
        psi_a_m1_p1 = _mm_slli_epi16(psi_a_m1_p1,1);
        psi_a_m1_p3 = _mm_mulhi_epi16(psi_a_m1_p3,ONE_OVER_SQRT_2);
        psi_a_m1_p3 = _mm_slli_epi16(psi_a_m1_p3,1);
        psi_a_m3_p1 = _mm_mulhi_epi16(psi_a_m3_p1,ONE_OVER_SQRT_2);
        psi_a_m3_p1 = _mm_slli_epi16(psi_a_m3_p1,1);
        psi_a_m3_p3 = _mm_mulhi_epi16(psi_a_m3_p3,ONE_OVER_SQRT_2);
        psi_a_m3_p3 = _mm_slli_epi16(psi_a_m3_p3,1);
        
        psi_a_m1_m1 = _mm_mulhi_epi16(psi_a_m1_m1,ONE_OVER_SQRT_2);
        psi_a_m1_m1 = _mm_slli_epi16(psi_a_m1_m1,1);
        psi_a_m1_m3 = _mm_mulhi_epi16(psi_a_m1_m3,ONE_OVER_SQRT_2);
        psi_a_m1_m3 = _mm_slli_epi16(psi_a_m1_m3,1);
        psi_a_m3_m1 = _mm_mulhi_epi16(psi_a_m3_m1,ONE_OVER_SQRT_2);
        psi_a_m3_m1 = _mm_slli_epi16(psi_a_m3_m1,1);
        psi_a_m3_m3 = _mm_mulhi_epi16(psi_a_m3_m3,ONE_OVER_SQRT_2);
        psi_a_m3_m3 = _mm_slli_epi16(psi_a_m3_m3,1);
       
        // Computing different multiples of channel norms
        ch_mag_over_10=_mm_mulhi_epi16(ch_mag_des, ONE_OVER_TWO_SQRT_10);
        ch_mag_over_2=_mm_mulhi_epi16(ch_mag_des, SQRT_10_OVER_FOUR);
        ch_mag_over_2=_mm_slli_epi16(ch_mag_over_2, 1);
        ch_mag_9_over_10=_mm_mulhi_epi16(ch_mag_des, NINE_OVER_TWO_SQRT_10);
        ch_mag_9_over_10=_mm_slli_epi16(ch_mag_9_over_10, 2);                  

        // Computing Metrics
        xmm1 = _mm_adds_epi16(psi_a_p1_p1, y0_p_1_1);
        bit_met_p1_p1= _mm_subs_epi16(xmm1, ch_mag_over_10);
                
        xmm1 = _mm_adds_epi16(psi_a_p1_p3, y0_p_1_3);
        bit_met_p1_p3= _mm_subs_epi16(xmm1, ch_mag_over_2);
                
        xmm1 = _mm_adds_epi16(psi_a_p1_m1, y0_m_1_1);
        bit_met_p1_m1= _mm_subs_epi16(xmm1, ch_mag_over_10);                
                
        xmm1 = _mm_adds_epi16(psi_a_p1_m3, y0_m_1_3);
        bit_met_p1_m3= _mm_subs_epi16(xmm1, ch_mag_over_2);                
                
        xmm1 = _mm_adds_epi16(psi_a_p3_p1, y0_p_3_1);
        bit_met_p3_p1= _mm_subs_epi16(xmm1, ch_mag_over_2);
                 
        xmm1 = _mm_adds_epi16(psi_a_p3_p3, y0_p_3_3);
        bit_met_p3_p3= _mm_subs_epi16(xmm1, ch_mag_9_over_10);
                                
        xmm1 = _mm_adds_epi16(psi_a_p3_m1, y0_m_3_1);
        bit_met_p3_m1= _mm_subs_epi16(xmm1, ch_mag_over_2);
                                
        xmm1 = _mm_adds_epi16(psi_a_p3_m3, y0_m_3_3);
        bit_met_p3_m3= _mm_subs_epi16(xmm1, ch_mag_9_over_10);
                
        xmm1 = _mm_subs_epi16(psi_a_m1_p1, y0_m_1_1);
        bit_met_m1_p1= _mm_subs_epi16(xmm1, ch_mag_over_10);                
                
        xmm1 = _mm_subs_epi16(psi_a_m1_p3, y0_m_1_3);
        bit_met_m1_p3= _mm_subs_epi16(xmm1, ch_mag_over_2);
                                
        xmm1 = _mm_subs_epi16(psi_a_m1_m1, y0_p_1_1);
        bit_met_m1_m1= _mm_subs_epi16(xmm1, ch_mag_over_10);                
                
        xmm1 = _mm_subs_epi16(psi_a_m1_m3, y0_p_1_3);
        bit_met_m1_m3= _mm_subs_epi16(xmm1, ch_mag_over_2);
                                
        xmm1 = _mm_subs_epi16(psi_a_m3_p1, y0_m_3_1);
        bit_met_m3_p1= _mm_subs_epi16(xmm1, ch_mag_over_2);
                                
        xmm1 = _mm_subs_epi16(psi_a_m3_p3, y0_m_3_3);
        bit_met_m3_p3= _mm_subs_epi16(xmm1, ch_mag_9_over_10);                
                
        xmm1 = _mm_subs_epi16(psi_a_m3_m1, y0_p_3_1);
        bit_met_m3_m1= _mm_subs_epi16(xmm1, ch_mag_over_2);
                                
        xmm1 = _mm_subs_epi16(psi_a_m3_m3, y0_p_3_3);
        bit_met_m3_m3= _mm_subs_epi16(xmm1, ch_mag_9_over_10);
        
        // LLR of the first bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_m1_p1,bit_met_m1_p3);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m1_m3);
        xmm2 = _mm_max_epi16(bit_met_m3_p1,bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_m1,bit_met_m3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_re0= _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_p1,bit_met_p1_p3);
        xmm1 = _mm_max_epi16(bit_met_p1_m1,bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_p3_p1,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_m1,bit_met_p3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4,xmm5);

        // LLR of first bit [L1(1), L1(2), L1(3), L1(4), L1(5), L1(6), L1(7), L1(8)]
        y0r = _mm_subs_epi16(logmax_den_re0,logmax_num_re0);    

        // LLR of the second bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_p1_m1,bit_met_p3_m1);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_m3,bit_met_p3_m3);
        xmm3 = _mm_max_epi16(bit_met_m1_m3,bit_met_m3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_re1 = _mm_max_epi16(xmm4,xmm5);
   
        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_p1,bit_met_p3_p1);
        xmm1 = _mm_max_epi16(bit_met_m1_p1,bit_met_m3_p1);
        xmm2 = _mm_max_epi16(bit_met_p1_p3,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p3,bit_met_m3_p3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_re1 = _mm_max_epi16(xmm4,xmm5);

        // LLR of second bit [L2(1), L2(2), L2(3), L2(4)]
        y1r = _mm_subs_epi16(logmax_den_re1,logmax_num_re1);

        // LLR of the third bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_m3_p1,bit_met_m3_p3);
        xmm1 = _mm_max_epi16(bit_met_m3_m1,bit_met_m3_m3);
        xmm2 = _mm_max_epi16(bit_met_p3_p1,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_m1,bit_met_p3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_im0 = _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_m1_p1,bit_met_m1_p3);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m1_m3);
        xmm2 = _mm_max_epi16(bit_met_p1_p1,bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_m1,bit_met_p1_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_im0 = _mm_max_epi16(xmm4,xmm5);

        // LLR of third bit [L3(1), L3(2), L3(3), L3(4)]
        y0i = _mm_subs_epi16(logmax_den_im0,logmax_num_im0);

        // LLR of the fourth bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_p1_m3,bit_met_p3_m3);
        xmm1 = _mm_max_epi16(bit_met_m1_m3,bit_met_m3_m3);
        xmm2 = _mm_max_epi16(bit_met_p1_p3,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p3,bit_met_m3_p3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_im1 = _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_m1,bit_met_p3_m1);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1,bit_met_p3_p1);
        xmm3 = _mm_max_epi16(bit_met_m1_p1,bit_met_m3_p1);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_im1 = _mm_max_epi16(xmm4,xmm5);

        // LLR of fourth bit [L4(1), L4(2), L4(3), L4(4)]
        y1i = _mm_subs_epi16(logmax_den_im1,logmax_num_im1);

        // Pack LLRs in output
        // [L1(1), L2(1), L1(2), L2(2), L1(3), L2(3), L1(4), L2(4)]
        xmm0 = _mm_unpacklo_epi16(y0r,y1r);
        // [L1(5), L2(5), L1(6), L2(6), L1(7), L2(7), L1(8), L2(8)]
        xmm1 = _mm_unpackhi_epi16(y0r,y1r);
        // [L3(1), L4(1), L3(2), L4(2), L3(3), L4(3), L3(4), L4(4)]
        xmm2 = _mm_unpacklo_epi16(y0i,y1i);
        // [L3(5), L4(5), L3(6), L4(6), L3(7), L4(7), L3(8), L4(8)]
        xmm3 = _mm_unpackhi_epi16(y0i,y1i);

        stream0_128i_out[2*i+0] = _mm_unpacklo_epi32(xmm0,xmm2); // 8LLRs, 2REs
        stream0_128i_out[2*i+1] = _mm_unpackhi_epi32(xmm0,xmm2);
        stream0_128i_out[2*i+2] = _mm_unpacklo_epi32(xmm1,xmm3);
        stream0_128i_out[2*i+3] = _mm_unpackhi_epi32(xmm1,xmm3);
    }
    _mm_empty();
    _m_empty();
}

int dlsch_16qam_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
                         int **rxdataF_comp,
                         int **rxdataF_comp_i,
                         int **dl_ch_mag,   //|h_0|^2*(2/sqrt{10})
                         int **rho_i,
                         short *dlsch_llr,
                         unsigned char symbol,
                         unsigned char first_symbol_flag,
                         unsigned short nb_rb,
                         uint16_t pbch_pss_sss_adjust,
                         short **llr16p) {

    int16_t *rxF      = (int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i    = (int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag   = (int16_t*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho      = (int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
  
    // first symbol has different structure due to more pilots
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
  

    if (!llr16) {
        msg("dlsch_16qam_qpsk_llr: llr is null, symbol %d\n",symbol);
        return(-1);
    }

    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }
    // printf("symbol %d: qam16_llr, len %d (llr16 %p)\n",symbol,len,llr16);
  
    qam16_qpsk((short *)rxF,
               (short *)rxF_i,
               (short *)ch_mag,
               (short *)llr16,
               (short *)rho,
               len);
  
    llr16 += (len<<2);
    *llr16p = (short *)llr16;

  return(0);
}

void qam16_qam16(short *stream0_in,
                 short *stream1_in,
                 short *ch_mag,
                 short *ch_mag_i,
                 short *stream0_out,
                 short *rho01,
                 int length
                 ) {

/* 
   Author: Sebastian Wagner
   Date: 2012-06-04

   Input: 
   stream0_in:  MF filter for 1st stream, i.e., y0=h0'*y
   stream!_in:  MF filter for 2nd stream, i.e., y1=h1'*y
   ch_mag:      2*h0/sqrt(00), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   ch_mag_i:    2*h1/sqrt(00), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   rho01:       Channel cross correlation, i.e., h1'*h0
   
   Output:
   stream0_out: output LLRs for 1st stream
*/
    __m128i *rho01_128i       = (__m128i *)rho01;
    __m128i *stream0_128i_in  = (__m128i *)stream0_in;
    __m128i *stream1_128i_in  = (__m128i *)stream1_in;
    __m128i *stream0_128i_out = (__m128i *)stream0_out;
    __m128i *ch_mag_128i      = (__m128i *)ch_mag;
    __m128i *ch_mag_128i_i    = (__m128i *)ch_mag_i;

    int i;

    ONE_OVER_SQRT_10 = _mm_set1_epi16(20724); // round(1/sqrt(10)*2^16)
    ONE_OVER_SQRT_10_Q15 = _mm_set1_epi16(10362); // round(1/sqrt(10)*2^15)
    THREE_OVER_SQRT_10 = _mm_set1_epi16(31086); // round(3/sqrt(10)*2^15)
    SQRT_10_OVER_FOUR = _mm_set1_epi16(25905); // round(sqrt(10)/4*2^15)
    ONE_OVER_TWO_SQRT_10 = _mm_set1_epi16(10362); // round(1/2/sqrt(10)*2^16)
    NINE_OVER_TWO_SQRT_10 = _mm_set1_epi16(23315); // round(9/2/sqrt(10)*2^14)

    for (i=0;i<length>>2;i+=2) {
        // In one iteration, we deal with 8 REs

        // Get rho
        xmm0 = rho01_128i[i];
        xmm1 = rho01_128i[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1 = _mm_mulhi_epi16(rho_rpi,ONE_OVER_SQRT_10);
        rho_rmi_1_1 = _mm_mulhi_epi16(rho_rmi,ONE_OVER_SQRT_10);
        rho_rpi_3_3 = _mm_mulhi_epi16(rho_rpi,THREE_OVER_SQRT_10);
        rho_rmi_3_3 = _mm_mulhi_epi16(rho_rmi,THREE_OVER_SQRT_10);
        rho_rpi_3_3 = _mm_slli_epi16(rho_rpi_3_3,1);
        rho_rmi_3_3 = _mm_slli_epi16(rho_rmi_3_3,1);

        xmm4 = _mm_mulhi_epi16(xmm2,ONE_OVER_SQRT_10); // Re(rho)
        xmm5 = _mm_mulhi_epi16(xmm3,THREE_OVER_SQRT_10); // Im(rho)
        xmm5 = _mm_slli_epi16(xmm5,1);

        rho_rpi_1_3 = _mm_adds_epi16(xmm4,xmm5);
        rho_rmi_1_3 = _mm_subs_epi16(xmm4,xmm5);

        xmm6 = _mm_mulhi_epi16(xmm2,THREE_OVER_SQRT_10); // Re(rho)
        xmm7 = _mm_mulhi_epi16(xmm3,ONE_OVER_SQRT_10); // Im(rho)
        xmm6 = _mm_slli_epi16(xmm6,1);

        rho_rpi_3_1 = _mm_adds_epi16(xmm6,xmm7);
        rho_rmi_3_1 = _mm_subs_epi16(xmm6,xmm7);

        // Rearrange interfering MF output
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]

        xmm0 = _mm_xor_si128(xmm0,xmm0); // ZERO
        xmm2 = _mm_subs_epi16(rho_rpi_1_1,y1r); // = [Re(rho)+ Im(rho)]/sqrt(10) - y1r
        psi_r_p1_p1 = _mm_abs_epi16(xmm2); // = |[Re(rho)+ Im(rho)]/sqrt(10) - y1r|

        xmm2= _mm_subs_epi16(rho_rmi_1_1,y1r);       psi_r_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_1,y1i);       psi_i_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_1_3,y1r);       psi_r_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_3,y1r);       psi_r_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_1,y1i);       psi_i_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_1,y1r);       psi_r_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_1,y1r);       psi_r_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_3,y1i);       psi_i_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_3,y1r);       psi_r_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_3,y1r);       psi_r_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_3,y1i);       psi_i_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_1_1,y1i);       psi_i_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_1,y1i);       psi_i_m1_p3 = _mm_abs_epi16(xmm2);        
        xmm2= _mm_subs_epi16(rho_rpi_1_3,y1i);       psi_i_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_3,y1i);       psi_i_m3_p3 = _mm_abs_epi16(xmm2);    
        xmm2= _mm_adds_epi16(rho_rpi_1_1,y1i);       psi_i_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_1,y1i);       psi_i_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_3,y1i);       psi_i_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_3,y1i);       psi_i_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_1,y1r);       psi_r_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_3,y1r);       psi_r_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_1,y1r);       psi_r_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_3,y1r);       psi_r_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_1_1);       psi_r_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_1_3);       psi_r_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_1_1);       psi_i_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_3_1);       psi_i_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_3_1);       psi_r_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_3_3);       psi_r_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_1_3);       psi_i_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_3_3);       psi_i_m3_m3 = _mm_abs_epi16(xmm2);

        // Rearrange desired MF output
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 
    
        // Rearrange desired channel magnitudes
        xmm2 = ch_mag_128i[i]; // = [|h|^2(1),|h|^2(1),|h|^2(2),|h|^2(2)]*(2/sqrt(10))
        xmm3 = ch_mag_128i[i+1]; // = [|h|^2(3),|h|^2(3),|h|^2(4),|h|^2(4)]*(2/sqrt(10))
        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_des = _mm_unpacklo_epi64(xmm2,xmm3); // = [|h|^2(1),|h|^2(2),|h|^2(3),|h|^2(4)]*(2/sqrt(10))

        // Rearrange interfering channel magnitudes
        xmm2 = ch_mag_128i_i[i];   
        xmm3 = ch_mag_128i_i[i+1]; 

        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_int  = _mm_unpacklo_epi64(xmm2,xmm3); 

        // Scale MF output of desired signal
        y0r_over_sqrt10 = _mm_mulhi_epi16(y0r,ONE_OVER_SQRT_10);
        y0i_over_sqrt10 = _mm_mulhi_epi16(y0i,ONE_OVER_SQRT_10);
        y0r_three_over_sqrt10 = _mm_mulhi_epi16(y0r,THREE_OVER_SQRT_10);
        y0i_three_over_sqrt10 = _mm_mulhi_epi16(y0i,THREE_OVER_SQRT_10);
        y0r_three_over_sqrt10 = _mm_slli_epi16(y0r_three_over_sqrt10,1);
        y0i_three_over_sqrt10 = _mm_slli_epi16(y0i_three_over_sqrt10,1);

        // Compute necessary combination of required terms
        y0_p_1_1 = _mm_adds_epi16(y0r_over_sqrt10,y0i_over_sqrt10);
        y0_m_1_1 = _mm_subs_epi16(y0r_over_sqrt10,y0i_over_sqrt10);  

        y0_p_1_3 = _mm_adds_epi16(y0r_over_sqrt10,y0i_three_over_sqrt10);
        y0_m_1_3 = _mm_subs_epi16(y0r_over_sqrt10,y0i_three_over_sqrt10);

        y0_p_3_1 = _mm_adds_epi16(y0r_three_over_sqrt10,y0i_over_sqrt10);
        y0_m_3_1 = _mm_subs_epi16(y0r_three_over_sqrt10,y0i_over_sqrt10);

        y0_p_3_3 = _mm_adds_epi16(y0r_three_over_sqrt10,y0i_three_over_sqrt10);
        y0_m_3_3 = _mm_subs_epi16(y0r_three_over_sqrt10,y0i_three_over_sqrt10);

        // Compute optimal interfering symbol magnitude
        interference_abs_epi16(psi_r_p1_p1 ,ch_mag_int,a_r_p1_p1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_p1 ,ch_mag_int,a_i_p1_p1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_p3 ,ch_mag_int,a_r_p1_p3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_p3 ,ch_mag_int,a_i_p1_p3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_m1 ,ch_mag_int,a_r_p1_m1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_m1 ,ch_mag_int,a_i_p1_m1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_m3 ,ch_mag_int,a_r_p1_m3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_m3 ,ch_mag_int,a_i_p1_m3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_p1 ,ch_mag_int,a_r_p3_p1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_p1 ,ch_mag_int,a_i_p3_p1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_p3 ,ch_mag_int,a_r_p3_p3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_p3 ,ch_mag_int,a_i_p3_p3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_m1 ,ch_mag_int,a_r_p3_m1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_m1 ,ch_mag_int,a_i_p3_m1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_m3 ,ch_mag_int,a_r_p3_m3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_m3 ,ch_mag_int,a_i_p3_m3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_p1 ,ch_mag_int,a_r_m1_p1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_p1 ,ch_mag_int,a_i_m1_p1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_p3 ,ch_mag_int,a_r_m1_p3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_p3 ,ch_mag_int,a_i_m1_p3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_m1 ,ch_mag_int,a_r_m1_m1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_m1 ,ch_mag_int,a_i_m1_m1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_m3 ,ch_mag_int,a_r_m1_m3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_m3 ,ch_mag_int,a_i_m1_m3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_p1 ,ch_mag_int,a_r_m3_p1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_p1 ,ch_mag_int,a_i_m3_p1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_p3 ,ch_mag_int,a_r_m3_p3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_p3 ,ch_mag_int,a_i_m3_p3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_m1 ,ch_mag_int,a_r_m3_m1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_m1 ,ch_mag_int,a_i_m3_m1,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_m3 ,ch_mag_int,a_r_m3_m3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_m3 ,ch_mag_int,a_i_m3_m3,ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        
        // Calculation of groups of two terms in the bit metric involving product of psi and interference magnitude
        prodsum_psi_a_epi16(psi_r_p1_p1,a_r_p1_p1,psi_i_p1_p1,a_i_p1_p1,psi_a_p1_p1);
        prodsum_psi_a_epi16(psi_r_p1_p3,a_r_p1_p3,psi_i_p1_p3,a_i_p1_p3,psi_a_p1_p3);
        prodsum_psi_a_epi16(psi_r_p3_p1,a_r_p3_p1,psi_i_p3_p1,a_i_p3_p1,psi_a_p3_p1);
        prodsum_psi_a_epi16(psi_r_p3_p3,a_r_p3_p3,psi_i_p3_p3,a_i_p3_p3,psi_a_p3_p3);
        prodsum_psi_a_epi16(psi_r_p1_m1,a_r_p1_m1,psi_i_p1_m1,a_i_p1_m1,psi_a_p1_m1);
        prodsum_psi_a_epi16(psi_r_p1_m3,a_r_p1_m3,psi_i_p1_m3,a_i_p1_m3,psi_a_p1_m3);
        prodsum_psi_a_epi16(psi_r_p3_m1,a_r_p3_m1,psi_i_p3_m1,a_i_p3_m1,psi_a_p3_m1);
        prodsum_psi_a_epi16(psi_r_p3_m3,a_r_p3_m3,psi_i_p3_m3,a_i_p3_m3,psi_a_p3_m3);
        prodsum_psi_a_epi16(psi_r_m1_p1,a_r_m1_p1,psi_i_m1_p1,a_i_m1_p1,psi_a_m1_p1);
        prodsum_psi_a_epi16(psi_r_m1_p3,a_r_m1_p3,psi_i_m1_p3,a_i_m1_p3,psi_a_m1_p3);
        prodsum_psi_a_epi16(psi_r_m3_p1,a_r_m3_p1,psi_i_m3_p1,a_i_m3_p1,psi_a_m3_p1);
        prodsum_psi_a_epi16(psi_r_m3_p3,a_r_m3_p3,psi_i_m3_p3,a_i_m3_p3,psi_a_m3_p3);
        prodsum_psi_a_epi16(psi_r_m1_m1,a_r_m1_m1,psi_i_m1_m1,a_i_m1_m1,psi_a_m1_m1);
        prodsum_psi_a_epi16(psi_r_m1_m3,a_r_m1_m3,psi_i_m1_m3,a_i_m1_m3,psi_a_m1_m3);
        prodsum_psi_a_epi16(psi_r_m3_m1,a_r_m3_m1,psi_i_m3_m1,a_i_m3_m1,psi_a_m3_m1);
        prodsum_psi_a_epi16(psi_r_m3_m3,a_r_m3_m3,psi_i_m3_m3,a_i_m3_m3,psi_a_m3_m3);

        
        // squared interference magnitude times int. ch. power
        square_a_epi16(a_r_p1_p1,a_i_p1_p1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p1_p1);
        square_a_epi16(a_r_p1_p3,a_i_p1_p3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p1_p3);
        square_a_epi16(a_r_p3_p1,a_i_p3_p1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p3_p1);
        square_a_epi16(a_r_p3_p3,a_i_p3_p3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p3_p3);
        square_a_epi16(a_r_p1_m1,a_i_p1_m1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p1_m1);
        square_a_epi16(a_r_p1_m3,a_i_p1_m3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p1_m3);
        square_a_epi16(a_r_p3_m1,a_i_p3_m1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p3_m1);
        square_a_epi16(a_r_p3_m3,a_i_p3_m3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p3_m3);
        square_a_epi16(a_r_m1_p1,a_i_m1_p1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m1_p1);
        square_a_epi16(a_r_m1_p3,a_i_m1_p3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m1_p3);
        square_a_epi16(a_r_m3_p1,a_i_m3_p1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m3_p1);
        square_a_epi16(a_r_m3_p3,a_i_m3_p3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m3_p3);
        square_a_epi16(a_r_m1_m1,a_i_m1_m1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m1_m1);
        square_a_epi16(a_r_m1_m3,a_i_m1_m3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m1_m3);
        square_a_epi16(a_r_m3_m1,a_i_m3_m1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m3_m1);
        square_a_epi16(a_r_m3_m3,a_i_m3_m3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m3_m3);   

        // Computing different multiples of channel norms
        ch_mag_over_10=_mm_mulhi_epi16(ch_mag_des, ONE_OVER_TWO_SQRT_10);
        ch_mag_over_2=_mm_mulhi_epi16(ch_mag_des, SQRT_10_OVER_FOUR);
        ch_mag_over_2=_mm_slli_epi16(ch_mag_over_2, 1);
        ch_mag_9_over_10=_mm_mulhi_epi16(ch_mag_des, NINE_OVER_TWO_SQRT_10);
        ch_mag_9_over_10=_mm_slli_epi16(ch_mag_9_over_10, 2);                  

        // Computing Metrics
        xmm0 = _mm_subs_epi16(psi_a_p1_p1,a_sq_p1_p1);
        xmm1 = _mm_adds_epi16(xmm0,y0_p_1_1);
        bit_met_p1_p1= _mm_subs_epi16(xmm1,ch_mag_over_10);

        xmm0 = _mm_subs_epi16(psi_a_p1_p3,a_sq_p1_p3);
        xmm1 = _mm_adds_epi16(xmm0,y0_p_1_3);
        bit_met_p1_p3= _mm_subs_epi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_epi16(psi_a_p1_m1,a_sq_p1_m1);
        xmm1 = _mm_adds_epi16(xmm0,y0_m_1_1);
        bit_met_p1_m1= _mm_subs_epi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_p1_m3,a_sq_p1_m3);
        xmm1 = _mm_adds_epi16(xmm0,y0_m_1_3);
        bit_met_p1_m3= _mm_subs_epi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_epi16(psi_a_p3_p1,a_sq_p3_p1);
        xmm1 = _mm_adds_epi16(xmm0,y0_p_3_1);
        bit_met_p3_p1= _mm_subs_epi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_epi16(psi_a_p3_p3,a_sq_p3_p3);
        xmm1 = _mm_adds_epi16(xmm0,y0_p_3_3);
        bit_met_p3_p3= _mm_subs_epi16(xmm1,ch_mag_9_over_10);

        xmm0 = _mm_subs_epi16(psi_a_p3_m1,a_sq_p3_m1);
        xmm1 = _mm_adds_epi16(xmm0,y0_m_3_1);
        bit_met_p3_m1= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_p3_m3,a_sq_p3_m3);
        xmm1 = _mm_adds_epi16(xmm0,y0_m_3_3);
        bit_met_p3_m3= _mm_subs_epi16(xmm1,ch_mag_9_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_p1,a_sq_m1_p1);
        xmm1 = _mm_subs_epi16(xmm0,y0_m_1_1);
        bit_met_m1_p1= _mm_subs_epi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_p3,a_sq_m1_p3);
        xmm1 = _mm_subs_epi16(xmm0,y0_m_1_3);
        bit_met_m1_p3= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_m1,a_sq_m1_m1);
        xmm1 = _mm_subs_epi16(xmm0,y0_p_1_1);
        bit_met_m1_m1= _mm_subs_epi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_m3,a_sq_m1_m3);
        xmm1 = _mm_subs_epi16(xmm0,y0_p_1_3);
        bit_met_m1_m3= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_m3_p1,a_sq_m3_p1);
        xmm1 = _mm_subs_epi16(xmm0,y0_m_3_1);
        bit_met_m3_p1= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_m3_p3,a_sq_m3_p3);
        xmm1 = _mm_subs_epi16(xmm0,y0_m_3_3);
        bit_met_m3_p3= _mm_subs_epi16(xmm1,ch_mag_9_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_m3_m1,a_sq_m3_m1);
        xmm1 = _mm_subs_epi16(xmm0,y0_p_3_1);
        bit_met_m3_m1= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_m3_m3,a_sq_m3_m3);
        xmm1 = _mm_subs_epi16(xmm0,y0_p_3_3);
        bit_met_m3_m3= _mm_subs_epi16(xmm1,ch_mag_9_over_10);
        
        // LLR of the first bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_m1_p1,bit_met_m1_p3);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m1_m3);
        xmm2 = _mm_max_epi16(bit_met_m3_p1,bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_m1,bit_met_m3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_re0= _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_p1,bit_met_p1_p3);
        xmm1 = _mm_max_epi16(bit_met_p1_m1,bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_p3_p1,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_m1,bit_met_p3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4,xmm5);

        // LLR of first bit [L1(1), L1(2), L1(3), L1(4), L1(5), L1(6), L1(7), L1(8)]
        y0r = _mm_subs_epi16(logmax_den_re0,logmax_num_re0);    

        // LLR of the second bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_p1_m1,bit_met_p3_m1);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_m3,bit_met_p3_m3);
        xmm3 = _mm_max_epi16(bit_met_m1_m3,bit_met_m3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_re1 = _mm_max_epi16(xmm4,xmm5);
   
        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_p1,bit_met_p3_p1);
        xmm1 = _mm_max_epi16(bit_met_m1_p1,bit_met_m3_p1);
        xmm2 = _mm_max_epi16(bit_met_p1_p3,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p3,bit_met_m3_p3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_re1 = _mm_max_epi16(xmm4,xmm5);

        // LLR of second bit [L2(1), L2(2), L2(3), L2(4)]
        y1r = _mm_subs_epi16(logmax_den_re1,logmax_num_re1);

        // LLR of the third bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_m3_p1,bit_met_m3_p3);
        xmm1 = _mm_max_epi16(bit_met_m3_m1,bit_met_m3_m3);
        xmm2 = _mm_max_epi16(bit_met_p3_p1,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_m1,bit_met_p3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_im0 = _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_m1_p1,bit_met_m1_p3);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m1_m3);
        xmm2 = _mm_max_epi16(bit_met_p1_p1,bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_m1,bit_met_p1_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_im0 = _mm_max_epi16(xmm4,xmm5);

        // LLR of third bit [L3(1), L3(2), L3(3), L3(4)]
        y0i = _mm_subs_epi16(logmax_den_im0,logmax_num_im0);

        // LLR of the fourth bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_p1_m3,bit_met_p3_m3);
        xmm1 = _mm_max_epi16(bit_met_m1_m3,bit_met_m3_m3);
        xmm2 = _mm_max_epi16(bit_met_p1_p3,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p3,bit_met_m3_p3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_im1 = _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_m1,bit_met_p3_m1);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1,bit_met_p3_p1);
        xmm3 = _mm_max_epi16(bit_met_m1_p1,bit_met_m3_p1);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_im1 = _mm_max_epi16(xmm4,xmm5);

        // LLR of fourth bit [L4(1), L4(2), L4(3), L4(4)]
        y1i = _mm_subs_epi16(logmax_den_im1,logmax_num_im1);

        // Pack LLRs in output
        // [L1(1), L2(1), L1(2), L2(2), L1(3), L2(3), L1(4), L2(4)]
        xmm0 = _mm_unpacklo_epi16(y0r,y1r);
        // [L1(5), L2(5), L1(6), L2(6), L1(7), L2(7), L1(8), L2(8)]
        xmm1 = _mm_unpackhi_epi16(y0r,y1r);
        // [L3(1), L4(1), L3(2), L4(2), L3(3), L4(3), L3(4), L4(4)]
        xmm2 = _mm_unpacklo_epi16(y0i,y1i);
        // [L3(5), L4(5), L3(6), L4(6), L3(7), L4(7), L3(8), L4(8)]
        xmm3 = _mm_unpackhi_epi16(y0i,y1i);

        stream0_128i_out[2*i+0] = _mm_unpacklo_epi32(xmm0,xmm2); // 8LLRs, 2REs
        stream0_128i_out[2*i+1] = _mm_unpackhi_epi32(xmm0,xmm2);
        stream0_128i_out[2*i+2] = _mm_unpacklo_epi32(xmm1,xmm3);
        stream0_128i_out[2*i+3] = _mm_unpackhi_epi32(xmm1,xmm3);
    }
    _mm_empty();
    _m_empty();
}

int dlsch_16qam_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                          int **rxdataF_comp,
                          int **rxdataF_comp_i,
                          int **dl_ch_mag,   //|h_0|^2*(2/sqrt{10})
                          int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
                          int **rho_i,
                          short *dlsch_llr,
                          unsigned char symbol,
                          unsigned char first_symbol_flag,
                          unsigned short nb_rb,
                          uint16_t pbch_pss_sss_adjust,
                          short **llr16p) {

    int16_t *rxF      = (int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i    = (int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag   = (int16_t*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag_i = (int16_t*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho      = (int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
  
    // first symbol has different structure due to more pilots
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
  

    if (!llr16) {
        msg("dlsch_16qam_16qam_llr: llr is null, symbol %d\n",symbol);
        return(-1);
    }

    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }
    // printf("symbol %d: qam16_llr, len %d (llr16 %p)\n",symbol,len,llr16);
  
    qam16_qam16((short *)rxF,
                (short *)rxF_i,
                (short *)ch_mag,
                (short *)ch_mag_i,
                (short *)llr16,
                (short *)rho,
                len);
  
    llr16 += (len<<2);
    *llr16p = (short *)llr16;

  return(0);
}

void qam16_qam64(short *stream0_in,
                 short *stream1_in,
                 short *ch_mag,
                 short *ch_mag_i,
                 short *stream0_out,
                 short *rho01,
                 int length
                 ) {

/* 
   Author: Sebastian Wagner
   Date: 2012-06-04

   Input: 
   stream0_in:  MF filter for 1st stream, i.e., y0=h0'*y
   stream!_in:  MF filter for 2nd stream, i.e., y1=h1'*y
   ch_mag:      2*h0/sqrt(00), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   ch_mag_i:    2*h1/sqrt(00), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   rho01:       Channel cross correlation, i.e., h1'*h0
   
   Output:
   stream0_out: output LLRs for 1st stream
*/
    __m128i *rho01_128i       = (__m128i *)rho01;
    __m128i *stream0_128i_in  = (__m128i *)stream0_in;
    __m128i *stream1_128i_in  = (__m128i *)stream1_in;
    __m128i *stream0_128i_out = (__m128i *)stream0_out;
    __m128i *ch_mag_128i      = (__m128i *)ch_mag;
    __m128i *ch_mag_128i_i    = (__m128i *)ch_mag_i;

    int i;

    ONE_OVER_SQRT_2 = _mm_set1_epi16(23170); // round(1/sqrt(2)*2^15)
    ONE_OVER_SQRT_10 = _mm_set1_epi16(20724); // round(1/sqrt(10)*2^16)
    ONE_OVER_SQRT_10_Q15 = _mm_set1_epi16(10362); // round(1/sqrt(10)*2^15)
    THREE_OVER_SQRT_10 = _mm_set1_epi16(31086); // round(3/sqrt(10)*2^15)
    SQRT_10_OVER_FOUR = _mm_set1_epi16(25905); // round(sqrt(10)/4*2^15)
    ONE_OVER_TWO_SQRT_10 = _mm_set1_epi16(10362); // round(1/2/sqrt(10)*2^16)
    NINE_OVER_TWO_SQRT_10 = _mm_set1_epi16(23315); // round(9/2/sqrt(10)*2^14)
    ONE_OVER_SQRT_2_42 = _mm_set1_epi16(3575); // round(1/sqrt(2*42)*2^15)
    THREE_OVER_SQRT_2_42 = _mm_set1_epi16(10726); // round(3/sqrt(2*42)*2^15)
    FIVE_OVER_SQRT_2_42 = _mm_set1_epi16(17876); // round(5/sqrt(2*42)*2^15)
    SEVEN_OVER_SQRT_2_42 = _mm_set1_epi16(25027); // round(7/sqrt(2*42)*2^15)
    SQRT_42_OVER_FOUR = _mm_set1_epi16(13272); // round(sqrt(42)/4*2^13), Q3.12

    for (i=0;i<length>>2;i+=2) {
        // In one iteration, we deal with 8 REs

        // Get rho
        xmm0 = rho01_128i[i];
        xmm1 = rho01_128i[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1 = _mm_mulhi_epi16(rho_rpi,ONE_OVER_SQRT_10);
        rho_rmi_1_1 = _mm_mulhi_epi16(rho_rmi,ONE_OVER_SQRT_10);
        rho_rpi_3_3 = _mm_mulhi_epi16(rho_rpi,THREE_OVER_SQRT_10);
        rho_rmi_3_3 = _mm_mulhi_epi16(rho_rmi,THREE_OVER_SQRT_10);
        rho_rpi_3_3 = _mm_slli_epi16(rho_rpi_3_3,1);
        rho_rmi_3_3 = _mm_slli_epi16(rho_rmi_3_3,1);

        xmm4 = _mm_mulhi_epi16(xmm2,ONE_OVER_SQRT_10); // Re(rho)
        xmm5 = _mm_mulhi_epi16(xmm3,THREE_OVER_SQRT_10); // Im(rho)
        xmm5 = _mm_slli_epi16(xmm5,1);

        rho_rpi_1_3 = _mm_adds_epi16(xmm4,xmm5);
        rho_rmi_1_3 = _mm_subs_epi16(xmm4,xmm5);

        xmm6 = _mm_mulhi_epi16(xmm2,THREE_OVER_SQRT_10); // Re(rho)
        xmm7 = _mm_mulhi_epi16(xmm3,ONE_OVER_SQRT_10); // Im(rho)
        xmm6 = _mm_slli_epi16(xmm6,1);

        rho_rpi_3_1 = _mm_adds_epi16(xmm6,xmm7);
        rho_rmi_3_1 = _mm_subs_epi16(xmm6,xmm7);

        // Rearrange interfering MF output
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]

        xmm0 = _mm_xor_si128(xmm0,xmm0); // ZERO
        xmm2 = _mm_subs_epi16(rho_rpi_1_1,y1r); // = [Re(rho)+ Im(rho)]/sqrt(10) - y1r
        psi_r_p1_p1 = _mm_abs_epi16(xmm2); // = |[Re(rho)+ Im(rho)]/sqrt(10) - y1r|

        xmm2= _mm_subs_epi16(rho_rmi_1_1,y1r);       psi_r_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_1,y1i);       psi_i_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_1_3,y1r);       psi_r_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_3,y1r);       psi_r_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_1,y1i);       psi_i_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_1,y1r);       psi_r_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_1,y1r);       psi_r_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_1_3,y1i);       psi_i_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_3,y1r);       psi_r_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_3,y1r);       psi_r_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rmi_3_3,y1i);       psi_i_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_1_1,y1i);       psi_i_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_1,y1i);       psi_i_m1_p3 = _mm_abs_epi16(xmm2);        
        xmm2= _mm_subs_epi16(rho_rpi_1_3,y1i);       psi_i_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_subs_epi16(rho_rpi_3_3,y1i);       psi_i_m3_p3 = _mm_abs_epi16(xmm2);    
        xmm2= _mm_adds_epi16(rho_rpi_1_1,y1i);       psi_i_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_1,y1i);       psi_i_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_3,y1i);       psi_i_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_3,y1i);       psi_i_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_1,y1r);       psi_r_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_1_3,y1r);       psi_r_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_1,y1r);       psi_r_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(rho_rpi_3_3,y1r);       psi_r_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_1_1);       psi_r_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_1_3);       psi_r_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_1_1);       psi_i_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_3_1);       psi_i_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_3_1);       psi_r_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1r,rho_rmi_3_3);       psi_r_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_1_3);       psi_i_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2= _mm_adds_epi16(y1i,rho_rmi_3_3);       psi_i_m3_m3 = _mm_abs_epi16(xmm2);

        // Rearrange desired MF output
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 
    
        // Rearrange desired channel magnitudes
        xmm2 = ch_mag_128i[i]; // = [|h|^2(1),|h|^2(1),|h|^2(2),|h|^2(2)]*(2/sqrt(10))
        xmm3 = ch_mag_128i[i+1]; // = [|h|^2(3),|h|^2(3),|h|^2(4),|h|^2(4)]*(2/sqrt(10))
        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_des = _mm_unpacklo_epi64(xmm2,xmm3); // = [|h|^2(1),|h|^2(2),|h|^2(3),|h|^2(4)]*(2/sqrt(10))

        // Rearrange interfering channel magnitudes
        xmm2 = ch_mag_128i_i[i];   
        xmm3 = ch_mag_128i_i[i+1]; 

        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_int  = _mm_unpacklo_epi64(xmm2,xmm3); 

        // Scale MF output of desired signal
        y0r_over_sqrt10 = _mm_mulhi_epi16(y0r,ONE_OVER_SQRT_10);
        y0i_over_sqrt10 = _mm_mulhi_epi16(y0i,ONE_OVER_SQRT_10);
        y0r_three_over_sqrt10 = _mm_mulhi_epi16(y0r,THREE_OVER_SQRT_10);
        y0i_three_over_sqrt10 = _mm_mulhi_epi16(y0i,THREE_OVER_SQRT_10);
        y0r_three_over_sqrt10 = _mm_slli_epi16(y0r_three_over_sqrt10,1);
        y0i_three_over_sqrt10 = _mm_slli_epi16(y0i_three_over_sqrt10,1);

        // Compute necessary combination of required terms
        y0_p_1_1 = _mm_adds_epi16(y0r_over_sqrt10,y0i_over_sqrt10);
        y0_m_1_1 = _mm_subs_epi16(y0r_over_sqrt10,y0i_over_sqrt10);  

        y0_p_1_3 = _mm_adds_epi16(y0r_over_sqrt10,y0i_three_over_sqrt10);
        y0_m_1_3 = _mm_subs_epi16(y0r_over_sqrt10,y0i_three_over_sqrt10);

        y0_p_3_1 = _mm_adds_epi16(y0r_three_over_sqrt10,y0i_over_sqrt10);
        y0_m_3_1 = _mm_subs_epi16(y0r_three_over_sqrt10,y0i_over_sqrt10);

        y0_p_3_3 = _mm_adds_epi16(y0r_three_over_sqrt10,y0i_three_over_sqrt10);
        y0_m_3_3 = _mm_subs_epi16(y0r_three_over_sqrt10,y0i_three_over_sqrt10);

        // Compute optimal interfering symbol magnitude
        ch_mag_int_with_sigma2       = _mm_srai_epi16(ch_mag_int, 1); // *2
        two_ch_mag_int_with_sigma2   = ch_mag_int; // *4
        three_ch_mag_int_with_sigma2 = _mm_adds_epi16(ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2); // *6

        interference_abs_64qam_epi16(psi_r_p1_p1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_p1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_p1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_p1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_p3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_p3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_p3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_p3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_m1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_m1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_m1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_m1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_m3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_m3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_m3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_m3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_p1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_p1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_p1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_p1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_p3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_p3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_p3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_p3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_m1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_m1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_m1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_m1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_m3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_m3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_m3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_m3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_p1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_p1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_p1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_p1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_p3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_p3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_p3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_p3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_m1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_m1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_m1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_m1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_m3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_m3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_m3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_m3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_p1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_p1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_p1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_p1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_p3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_p3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_p3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_p3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_m1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_m1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_m1 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_m1,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_m3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_m3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_m3 ,ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_m3,ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42,FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        
        // Calculation of groups of two terms in the bit metric involving product of psi and interference magnitude
        prodsum_psi_a_epi16(psi_r_p1_p1,a_r_p1_p1,psi_i_p1_p1,a_i_p1_p1,psi_a_p1_p1);
        prodsum_psi_a_epi16(psi_r_p1_p3,a_r_p1_p3,psi_i_p1_p3,a_i_p1_p3,psi_a_p1_p3);
        prodsum_psi_a_epi16(psi_r_p3_p1,a_r_p3_p1,psi_i_p3_p1,a_i_p3_p1,psi_a_p3_p1);
        prodsum_psi_a_epi16(psi_r_p3_p3,a_r_p3_p3,psi_i_p3_p3,a_i_p3_p3,psi_a_p3_p3);
        prodsum_psi_a_epi16(psi_r_p1_m1,a_r_p1_m1,psi_i_p1_m1,a_i_p1_m1,psi_a_p1_m1);
        prodsum_psi_a_epi16(psi_r_p1_m3,a_r_p1_m3,psi_i_p1_m3,a_i_p1_m3,psi_a_p1_m3);
        prodsum_psi_a_epi16(psi_r_p3_m1,a_r_p3_m1,psi_i_p3_m1,a_i_p3_m1,psi_a_p3_m1);
        prodsum_psi_a_epi16(psi_r_p3_m3,a_r_p3_m3,psi_i_p3_m3,a_i_p3_m3,psi_a_p3_m3);
        prodsum_psi_a_epi16(psi_r_m1_p1,a_r_m1_p1,psi_i_m1_p1,a_i_m1_p1,psi_a_m1_p1);
        prodsum_psi_a_epi16(psi_r_m1_p3,a_r_m1_p3,psi_i_m1_p3,a_i_m1_p3,psi_a_m1_p3);
        prodsum_psi_a_epi16(psi_r_m3_p1,a_r_m3_p1,psi_i_m3_p1,a_i_m3_p1,psi_a_m3_p1);
        prodsum_psi_a_epi16(psi_r_m3_p3,a_r_m3_p3,psi_i_m3_p3,a_i_m3_p3,psi_a_m3_p3);
        prodsum_psi_a_epi16(psi_r_m1_m1,a_r_m1_m1,psi_i_m1_m1,a_i_m1_m1,psi_a_m1_m1);
        prodsum_psi_a_epi16(psi_r_m1_m3,a_r_m1_m3,psi_i_m1_m3,a_i_m1_m3,psi_a_m1_m3);
        prodsum_psi_a_epi16(psi_r_m3_m1,a_r_m3_m1,psi_i_m3_m1,a_i_m3_m1,psi_a_m3_m1);
        prodsum_psi_a_epi16(psi_r_m3_m3,a_r_m3_m3,psi_i_m3_m3,a_i_m3_m3,psi_a_m3_m3);

        // Multiply by sqrt(2)
        psi_a_p1_p1 = _mm_mulhi_epi16(psi_a_p1_p1, ONE_OVER_SQRT_2); psi_a_p1_p1 = _mm_slli_epi16(psi_a_p1_p1, 2);
        psi_a_p1_p3 = _mm_mulhi_epi16(psi_a_p1_p3, ONE_OVER_SQRT_2); psi_a_p1_p3 = _mm_slli_epi16(psi_a_p1_p3, 2);
        psi_a_p3_p1 = _mm_mulhi_epi16(psi_a_p3_p1, ONE_OVER_SQRT_2); psi_a_p3_p1 = _mm_slli_epi16(psi_a_p3_p1, 2);
        psi_a_p3_p3 = _mm_mulhi_epi16(psi_a_p3_p3, ONE_OVER_SQRT_2); psi_a_p3_p3 = _mm_slli_epi16(psi_a_p3_p3, 2);
        psi_a_p1_m1 = _mm_mulhi_epi16(psi_a_p1_m1, ONE_OVER_SQRT_2); psi_a_p1_m1 = _mm_slli_epi16(psi_a_p1_m1, 2);
        psi_a_p1_m3 = _mm_mulhi_epi16(psi_a_p1_m3, ONE_OVER_SQRT_2); psi_a_p1_m3 = _mm_slli_epi16(psi_a_p1_m3, 2);
        psi_a_p3_m1 = _mm_mulhi_epi16(psi_a_p3_m1, ONE_OVER_SQRT_2); psi_a_p3_m1 = _mm_slli_epi16(psi_a_p3_m1, 2);
        psi_a_p3_m3 = _mm_mulhi_epi16(psi_a_p3_m3, ONE_OVER_SQRT_2); psi_a_p3_m3 = _mm_slli_epi16(psi_a_p3_m3, 2);
        psi_a_m1_p1 = _mm_mulhi_epi16(psi_a_m1_p1, ONE_OVER_SQRT_2); psi_a_m1_p1 = _mm_slli_epi16(psi_a_m1_p1, 2);
        psi_a_m1_p3 = _mm_mulhi_epi16(psi_a_m1_p3, ONE_OVER_SQRT_2); psi_a_m1_p3 = _mm_slli_epi16(psi_a_m1_p3, 2);
        psi_a_m3_p1 = _mm_mulhi_epi16(psi_a_m3_p1, ONE_OVER_SQRT_2); psi_a_m3_p1 = _mm_slli_epi16(psi_a_m3_p1, 2);
        psi_a_m3_p3 = _mm_mulhi_epi16(psi_a_m3_p3, ONE_OVER_SQRT_2); psi_a_m3_p3 = _mm_slli_epi16(psi_a_m3_p3, 2);
        psi_a_m1_m1 = _mm_mulhi_epi16(psi_a_m1_m1, ONE_OVER_SQRT_2); psi_a_m1_m1 = _mm_slli_epi16(psi_a_m1_m1, 2);
        psi_a_m1_m3 = _mm_mulhi_epi16(psi_a_m1_m3, ONE_OVER_SQRT_2); psi_a_m1_m3 = _mm_slli_epi16(psi_a_m1_m3, 2);
        psi_a_m3_m1 = _mm_mulhi_epi16(psi_a_m3_m1, ONE_OVER_SQRT_2); psi_a_m3_m1 = _mm_slli_epi16(psi_a_m3_m1, 2);
        psi_a_m3_m3 = _mm_mulhi_epi16(psi_a_m3_m3, ONE_OVER_SQRT_2); psi_a_m3_m3 = _mm_slli_epi16(psi_a_m3_m3, 2);
        
        // squared interference magnitude times int. ch. power
        square_a_64qam_epi16(a_r_p1_p1,a_i_p1_p1,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_p1_p1);
        square_a_64qam_epi16(a_r_p1_p3,a_i_p1_p3,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_p1_p3);
        square_a_64qam_epi16(a_r_p3_p1,a_i_p3_p1,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_p3_p1);
        square_a_64qam_epi16(a_r_p3_p3,a_i_p3_p3,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_p3_p3);
        square_a_64qam_epi16(a_r_p1_m1,a_i_p1_m1,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_p1_m1);
        square_a_64qam_epi16(a_r_p1_m3,a_i_p1_m3,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_p1_m3);
        square_a_64qam_epi16(a_r_p3_m1,a_i_p3_m1,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_p3_m1);
        square_a_64qam_epi16(a_r_p3_m3,a_i_p3_m3,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_p3_m3);
        square_a_64qam_epi16(a_r_m1_p1,a_i_m1_p1,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_m1_p1);
        square_a_64qam_epi16(a_r_m1_p3,a_i_m1_p3,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_m1_p3);
        square_a_64qam_epi16(a_r_m3_p1,a_i_m3_p1,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_m3_p1);
        square_a_64qam_epi16(a_r_m3_p3,a_i_m3_p3,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_m3_p3);
        square_a_64qam_epi16(a_r_m1_m1,a_i_m1_m1,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_m1_m1);
        square_a_64qam_epi16(a_r_m1_m3,a_i_m1_m3,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_m1_m3);
        square_a_64qam_epi16(a_r_m3_m1,a_i_m3_m1,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_m3_m1);
        square_a_64qam_epi16(a_r_m3_m3,a_i_m3_m3,ch_mag_int,SQRT_42_OVER_FOUR,a_sq_m3_m3);   

        // Computing different multiples of channel norms
        ch_mag_over_10=_mm_mulhi_epi16(ch_mag_des, ONE_OVER_TWO_SQRT_10);
        ch_mag_over_2=_mm_mulhi_epi16(ch_mag_des, SQRT_10_OVER_FOUR);
        ch_mag_over_2=_mm_slli_epi16(ch_mag_over_2, 1);
        ch_mag_9_over_10=_mm_mulhi_epi16(ch_mag_des, NINE_OVER_TWO_SQRT_10);
        ch_mag_9_over_10=_mm_slli_epi16(ch_mag_9_over_10, 2);                  

        // Computing Metrics
        xmm0 = _mm_subs_epi16(psi_a_p1_p1,a_sq_p1_p1);
        xmm1 = _mm_adds_epi16(xmm0,y0_p_1_1);
        bit_met_p1_p1= _mm_subs_epi16(xmm1,ch_mag_over_10);

        xmm0 = _mm_subs_epi16(psi_a_p1_p3,a_sq_p1_p3);
        xmm1 = _mm_adds_epi16(xmm0,y0_p_1_3);
        bit_met_p1_p3= _mm_subs_epi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_epi16(psi_a_p1_m1,a_sq_p1_m1);
        xmm1 = _mm_adds_epi16(xmm0,y0_m_1_1);
        bit_met_p1_m1= _mm_subs_epi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_p1_m3,a_sq_p1_m3);
        xmm1 = _mm_adds_epi16(xmm0,y0_m_1_3);
        bit_met_p1_m3= _mm_subs_epi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_epi16(psi_a_p3_p1,a_sq_p3_p1);
        xmm1 = _mm_adds_epi16(xmm0,y0_p_3_1);
        bit_met_p3_p1= _mm_subs_epi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_epi16(psi_a_p3_p3,a_sq_p3_p3);
        xmm1 = _mm_adds_epi16(xmm0,y0_p_3_3);
        bit_met_p3_p3= _mm_subs_epi16(xmm1,ch_mag_9_over_10);

        xmm0 = _mm_subs_epi16(psi_a_p3_m1,a_sq_p3_m1);
        xmm1 = _mm_adds_epi16(xmm0,y0_m_3_1);
        bit_met_p3_m1= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_p3_m3,a_sq_p3_m3);
        xmm1 = _mm_adds_epi16(xmm0,y0_m_3_3);
        bit_met_p3_m3= _mm_subs_epi16(xmm1,ch_mag_9_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_p1,a_sq_m1_p1);
        xmm1 = _mm_subs_epi16(xmm0,y0_m_1_1);
        bit_met_m1_p1= _mm_subs_epi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_p3,a_sq_m1_p3);
        xmm1 = _mm_subs_epi16(xmm0,y0_m_1_3);
        bit_met_m1_p3= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_m1,a_sq_m1_m1);
        xmm1 = _mm_subs_epi16(xmm0,y0_p_1_1);
        bit_met_m1_m1= _mm_subs_epi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_m1_m3,a_sq_m1_m3);
        xmm1 = _mm_subs_epi16(xmm0,y0_p_1_3);
        bit_met_m1_m3= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_m3_p1,a_sq_m3_p1);
        xmm1 = _mm_subs_epi16(xmm0,y0_m_3_1);
        bit_met_m3_p1= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_m3_p3,a_sq_m3_p3);
        xmm1 = _mm_subs_epi16(xmm0,y0_m_3_3);
        bit_met_m3_p3= _mm_subs_epi16(xmm1,ch_mag_9_over_10);
        
        xmm0 = _mm_subs_epi16(psi_a_m3_m1,a_sq_m3_m1);
        xmm1 = _mm_subs_epi16(xmm0,y0_p_3_1);
        bit_met_m3_m1= _mm_subs_epi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_epi16(psi_a_m3_m3,a_sq_m3_m3);
        xmm1 = _mm_subs_epi16(xmm0,y0_p_3_3);
        bit_met_m3_m3= _mm_subs_epi16(xmm1,ch_mag_9_over_10);
        
        // LLR of the first bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_m1_p1,bit_met_m1_p3);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m1_m3);
        xmm2 = _mm_max_epi16(bit_met_m3_p1,bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_m1,bit_met_m3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_re0= _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_p1,bit_met_p1_p3);
        xmm1 = _mm_max_epi16(bit_met_p1_m1,bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_p3_p1,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_m1,bit_met_p3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4,xmm5);

        // LLR of first bit [L1(1), L1(2), L1(3), L1(4), L1(5), L1(6), L1(7), L1(8)]
        y0r = _mm_subs_epi16(logmax_den_re0,logmax_num_re0);    

        // LLR of the second bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_p1_m1,bit_met_p3_m1);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_m3,bit_met_p3_m3);
        xmm3 = _mm_max_epi16(bit_met_m1_m3,bit_met_m3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_re1 = _mm_max_epi16(xmm4,xmm5);
   
        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_p1,bit_met_p3_p1);
        xmm1 = _mm_max_epi16(bit_met_m1_p1,bit_met_m3_p1);
        xmm2 = _mm_max_epi16(bit_met_p1_p3,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p3,bit_met_m3_p3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_re1 = _mm_max_epi16(xmm4,xmm5);

        // LLR of second bit [L2(1), L2(2), L2(3), L2(4)]
        y1r = _mm_subs_epi16(logmax_den_re1,logmax_num_re1);

        // LLR of the third bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_m3_p1,bit_met_m3_p3);
        xmm1 = _mm_max_epi16(bit_met_m3_m1,bit_met_m3_m3);
        xmm2 = _mm_max_epi16(bit_met_p3_p1,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_m1,bit_met_p3_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_im0 = _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_m1_p1,bit_met_m1_p3);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m1_m3);
        xmm2 = _mm_max_epi16(bit_met_p1_p1,bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_m1,bit_met_p1_m3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_im0 = _mm_max_epi16(xmm4,xmm5);

        // LLR of third bit [L3(1), L3(2), L3(3), L3(4)]
        y0i = _mm_subs_epi16(logmax_den_im0,logmax_num_im0);

        // LLR of the fourth bit
        // Bit = 1
        xmm0 = _mm_max_epi16(bit_met_p1_m3,bit_met_p3_m3);
        xmm1 = _mm_max_epi16(bit_met_m1_m3,bit_met_m3_m3);
        xmm2 = _mm_max_epi16(bit_met_p1_p3,bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p3,bit_met_m3_p3);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_num_im1 = _mm_max_epi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_epi16(bit_met_p1_m1,bit_met_p3_m1);
        xmm1 = _mm_max_epi16(bit_met_m1_m1,bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1,bit_met_p3_p1);
        xmm3 = _mm_max_epi16(bit_met_m1_p1,bit_met_m3_p1);
        xmm4 = _mm_max_epi16(xmm0,xmm1);
        xmm5 = _mm_max_epi16(xmm2,xmm3);
        logmax_den_im1 = _mm_max_epi16(xmm4,xmm5);

        // LLR of fourth bit [L4(1), L4(2), L4(3), L4(4)]
        y1i = _mm_subs_epi16(logmax_den_im1,logmax_num_im1);

        // Pack LLRs in output
        // [L1(1), L2(1), L1(2), L2(2), L1(3), L2(3), L1(4), L2(4)]
        xmm0 = _mm_unpacklo_epi16(y0r,y1r);
        // [L1(5), L2(5), L1(6), L2(6), L1(7), L2(7), L1(8), L2(8)]
        xmm1 = _mm_unpackhi_epi16(y0r,y1r);
        // [L3(1), L4(1), L3(2), L4(2), L3(3), L4(3), L3(4), L4(4)]
        xmm2 = _mm_unpacklo_epi16(y0i,y1i);
        // [L3(5), L4(5), L3(6), L4(6), L3(7), L4(7), L3(8), L4(8)]
        xmm3 = _mm_unpackhi_epi16(y0i,y1i);

        stream0_128i_out[2*i+0] = _mm_unpacklo_epi32(xmm0,xmm2); // 8LLRs, 2REs
        stream0_128i_out[2*i+1] = _mm_unpackhi_epi32(xmm0,xmm2);
        stream0_128i_out[2*i+2] = _mm_unpacklo_epi32(xmm1,xmm3);
        stream0_128i_out[2*i+3] = _mm_unpackhi_epi32(xmm1,xmm3);
    }
    _mm_empty();
    _m_empty();
}

int dlsch_16qam_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                          int **rxdataF_comp,
                          int **rxdataF_comp_i,
                          int **dl_ch_mag,   //|h_0|^2*(2/sqrt{10})
                          int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
                          int **rho_i,
                          short *dlsch_llr,
                          unsigned char symbol,
                          unsigned char first_symbol_flag,
                          unsigned short nb_rb,
                          uint16_t pbch_pss_sss_adjust,
                          short **llr16p) {

    int16_t *rxF      = (int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i    = (int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag   = (int16_t*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag_i = (int16_t*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho      = (int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
  
    // first symbol has different structure due to more pilots
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
  

    if (!llr16) {
        msg("dlsch_16qam_64qam_llr: llr is null, symbol %d\n",symbol);
        return(-1);
    }

    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }
    // printf("symbol %d: qam16_llr, len %d (llr16 %p)\n",symbol,len,llr16);
  
    qam16_qam64((short *)rxF,
                (short *)rxF_i,
                (short *)ch_mag,
                (short *)ch_mag_i,
                (short *)llr16,
                (short *)rho,
                len);
  
    llr16 += (len<<2);
    *llr16p = (short *)llr16;

  return(0);
}

//----------------------------------------------------------------------------------------------
// 64-QAM
//----------------------------------------------------------------------------------------------

NOCYGWIN_STATIC __m128i ONE_OVER_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i THREE_OVER_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i FIVE_OVER_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i SEVEN_OVER_SQRT_42 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m128i FORTYNINE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i THIRTYSEVEN_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i TWENTYNINE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i TWENTYFIVE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i SEVENTEEN_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i NINE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i THIRTEEN_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i FIVE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ONE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m128i  y0r_one_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i  y0r_three_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i  y0r_five_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i  y0r_seven_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i  y0i_one_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i  y0i_three_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i  y0i_five_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i  y0i_seven_over_sqrt_21 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m128i ch_mag_98_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_74_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_58_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_50_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_34_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_18_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_26_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_10_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_2_over_42_with_sigma2 __attribute__((aligned(16))); 

void qam64_qpsk(short *stream0_in,
                short *stream1_in,
                short *ch_mag,
                short *stream0_out,
                short *rho01,
                int length
                ) {

/* 
   Author: S. Wagner
   Date: 31-07-12

   Input: 
   stream0_in:  MF filter for 1st stream, i.e., y0=h0'*y
   stream1_in:  MF filter for 2nd stream, i.e., y1=h1'*y
   ch_mag:      4*h0/sqrt(42), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   ch_mag_i:    4*h1/sqrt(42), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   rho01:       Channel cross correlation, i.e., h1'*h0
   
   Output:
   stream0_out: output LLRs for 1st stream
*/

    __m128i *rho01_128i      = (__m128i *)rho01;
    __m128i *stream0_128i_in = (__m128i *)stream0_in;
    __m128i *stream1_128i_in = (__m128i *)stream1_in;
    __m128i *ch_mag_128i     = (__m128i *)ch_mag;

    int i,j;

    ONE_OVER_SQRT_42 = _mm_set1_epi16(10112); // round(1/sqrt(42)*2^16)
    THREE_OVER_SQRT_42 = _mm_set1_epi16(30337); // round(3/sqrt(42)*2^16)
    FIVE_OVER_SQRT_42 = _mm_set1_epi16(25281); // round(5/sqrt(42)*2^15)
    SEVEN_OVER_SQRT_42 = _mm_set1_epi16(17697); // round(5/sqrt(42)*2^15)
    ONE_OVER_SQRT_2 = _mm_set1_epi16(23170); // round(1/sqrt(2)*2^15)
    ONE_OVER_SQRT_2_42 = _mm_set1_epi16(3575); // round(1/sqrt(2*42)*2^15)
    THREE_OVER_SQRT_2_42 = _mm_set1_epi16(10726); // round(3/sqrt(2*42)*2^15)
    FIVE_OVER_SQRT_2_42 = _mm_set1_epi16(17876); // round(5/sqrt(2*42)*2^15)
    SEVEN_OVER_SQRT_2_42 = _mm_set1_epi16(25027); // round(7/sqrt(2*42)*2^15)
    FORTYNINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(30969); // round(49/(4*sqrt(42))*2^14), Q2.14
    THIRTYSEVEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(23385); // round(37/(4*sqrt(42))*2^14), Q2.14
    TWENTYFIVE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(31601); // round(25/(4*sqrt(42))*2^15)
    TWENTYNINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(18329); // round(29/(4*sqrt(42))*2^15), Q2.14
    SEVENTEEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(21489); // round(17/(4*sqrt(42))*2^15)
    NINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(11376); // round(9/(4*sqrt(42))*2^15)
    THIRTEEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(16433); // round(13/(4*sqrt(42))*2^15)
    FIVE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(6320); // round(5/(4*sqrt(42))*2^15)
    ONE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(1264); // round(1/(4*sqrt(42))*2^15)
    SQRT_42_OVER_FOUR = _mm_set1_epi16(13272); // round(sqrt(42)/4*2^13), Q3.12

    for (i=0; i<length>>2; i+=2) {
          
        // Get rho
        xmm0 = rho01_128i[i];
        xmm1 = rho01_128i[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1 = _mm_mulhi_epi16(rho_rpi, ONE_OVER_SQRT_42);   
        rho_rmi_1_1 = _mm_mulhi_epi16(rho_rmi, ONE_OVER_SQRT_42);
        rho_rpi_3_3 = _mm_mulhi_epi16(rho_rpi, THREE_OVER_SQRT_42); 
        rho_rmi_3_3 = _mm_mulhi_epi16(rho_rmi, THREE_OVER_SQRT_42);
        rho_rpi_5_5 = _mm_mulhi_epi16(rho_rpi, FIVE_OVER_SQRT_42);  
        rho_rmi_5_5 = _mm_mulhi_epi16(rho_rmi, FIVE_OVER_SQRT_42); 
        rho_rpi_7_7 = _mm_mulhi_epi16(rho_rpi, SEVEN_OVER_SQRT_42); 
        rho_rmi_7_7 = _mm_mulhi_epi16(rho_rmi, SEVEN_OVER_SQRT_42);

        rho_rpi_5_5 = _mm_slli_epi16(rho_rpi_5_5, 1); 
        rho_rmi_5_5 = _mm_slli_epi16(rho_rmi_5_5, 1);
        rho_rpi_7_7 = _mm_slli_epi16(rho_rpi_7_7, 2); 
        rho_rmi_7_7 = _mm_slli_epi16(rho_rmi_7_7, 2);

        xmm4 = _mm_mulhi_epi16(xmm2, ONE_OVER_SQRT_42);   
        xmm5 = _mm_mulhi_epi16(xmm3, ONE_OVER_SQRT_42);   
        xmm6 = _mm_mulhi_epi16(xmm3, THREE_OVER_SQRT_42); 
        xmm7 = _mm_mulhi_epi16(xmm3, FIVE_OVER_SQRT_42);  
        xmm8 = _mm_mulhi_epi16(xmm3, SEVEN_OVER_SQRT_42); 
        xmm7 = _mm_slli_epi16(xmm7, 1);
        xmm8 = _mm_slli_epi16(xmm8, 2);

        rho_rpi_1_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_1_3 = _mm_subs_epi16(xmm4, xmm6); 
        rho_rpi_1_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_1_5 = _mm_subs_epi16(xmm4, xmm7);
        rho_rpi_1_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_1_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, THREE_OVER_SQRT_42); 
        rho_rpi_3_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_3_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_3_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_3_5 = _mm_subs_epi16(xmm4, xmm7);
        rho_rpi_3_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_3_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, FIVE_OVER_SQRT_42); 
        xmm4 = _mm_slli_epi16(xmm4, 1);
        rho_rpi_5_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_5_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_5_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_5_3 = _mm_subs_epi16(xmm4, xmm6);
        rho_rpi_5_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_5_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, SEVEN_OVER_SQRT_42); 
        xmm4 = _mm_slli_epi16(xmm4, 2);
        rho_rpi_7_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_7_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_7_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_7_3 = _mm_subs_epi16(xmm4, xmm6);
        rho_rpi_7_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_7_5 = _mm_subs_epi16(xmm4, xmm7);

        // Rearrange interfering MF output
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]

        // Psi_r calculation from rho_rpi or rho_rmi
        xmm0 = _mm_xor_si128(xmm0, xmm0); // ZERO for abs_pi16
        xmm2 = _mm_subs_epi16(rho_rpi_7_7, y1r); psi_r_p7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_5, y1r); psi_r_p7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_3, y1r); psi_r_p7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_1, y1r); psi_r_p7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_1, y1r); psi_r_p7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_3, y1r); psi_r_p7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_5, y1r); psi_r_p7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_7, y1r); psi_r_p7_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_7, y1r); psi_r_p5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_5, y1r); psi_r_p5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_3, y1r); psi_r_p5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_1, y1r); psi_r_p5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_1, y1r); psi_r_p5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_3, y1r); psi_r_p5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_5, y1r); psi_r_p5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_7, y1r); psi_r_p5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_7, y1r); psi_r_p3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_5, y1r); psi_r_p3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_3, y1r); psi_r_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_1, y1r); psi_r_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_1, y1r); psi_r_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_3, y1r); psi_r_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_5, y1r); psi_r_p3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_7, y1r); psi_r_p3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_7, y1r); psi_r_p1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_5, y1r); psi_r_p1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_3, y1r); psi_r_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_1, y1r); psi_r_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_1, y1r); psi_r_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_3, y1r); psi_r_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_5, y1r); psi_r_p1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_7, y1r); psi_r_p1_m7 = _mm_abs_epi16(xmm2);
                                                            
        xmm2 = _mm_adds_epi16(rho_rmi_1_7, y1r); psi_r_m1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_5, y1r); psi_r_m1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_3, y1r); psi_r_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_1, y1r); psi_r_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_1, y1r); psi_r_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_3, y1r); psi_r_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_5, y1r); psi_r_m1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_7, y1r); psi_r_m1_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_7, y1r); psi_r_m3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_5, y1r); psi_r_m3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_3, y1r); psi_r_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_1, y1r); psi_r_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_1, y1r); psi_r_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_3, y1r); psi_r_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_5, y1r); psi_r_m3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_7, y1r); psi_r_m3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_7, y1r); psi_r_m5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_5, y1r); psi_r_m5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_3, y1r); psi_r_m5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_1, y1r); psi_r_m5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_1, y1r); psi_r_m5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_3, y1r); psi_r_m5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_5, y1r); psi_r_m5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_7, y1r); psi_r_m5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_7, y1r); psi_r_m7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_5, y1r); psi_r_m7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_3, y1r); psi_r_m7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_1, y1r); psi_r_m7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_1, y1r); psi_r_m7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_3, y1r); psi_r_m7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_5, y1r); psi_r_m7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_7, y1r); psi_r_m7_m7 = _mm_abs_epi16(xmm2);

        // Psi_i calculation from rho_rpi or rho_rmi
        xmm2 = _mm_subs_epi16(rho_rmi_7_7, y1i); psi_i_p7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_7, y1i); psi_i_p7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_7, y1i); psi_i_p7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_7, y1i); psi_i_p7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_7, y1i); psi_i_p7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_7, y1i); psi_i_p7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_7, y1i); psi_i_p7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_7, y1i); psi_i_p7_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_5, y1i); psi_i_p5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_5, y1i); psi_i_p5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_5, y1i); psi_i_p5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_5, y1i); psi_i_p5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_5, y1i); psi_i_p5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_5, y1i); psi_i_p5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_5, y1i); psi_i_p5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_5, y1i); psi_i_p5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_3, y1i); psi_i_p3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_3, y1i); psi_i_p3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_3, y1i); psi_i_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_3, y1i); psi_i_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_3, y1i); psi_i_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_3, y1i); psi_i_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_3, y1i); psi_i_p3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_3, y1i); psi_i_p3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_1, y1i); psi_i_p1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_1, y1i); psi_i_p1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_1, y1i); psi_i_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_1, y1i); psi_i_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_1, y1i); psi_i_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_1, y1i); psi_i_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_1, y1i); psi_i_p1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_1, y1i); psi_i_p1_m7 = _mm_abs_epi16(xmm2);

        xmm2 = _mm_subs_epi16(rho_rpi_7_1, y1i); psi_i_m1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_1, y1i); psi_i_m1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_1, y1i); psi_i_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_1, y1i); psi_i_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_1, y1i); psi_i_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_1, y1i); psi_i_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_1, y1i); psi_i_m1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_1, y1i); psi_i_m1_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_3, y1i); psi_i_m3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_3, y1i); psi_i_m3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_3, y1i); psi_i_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_3, y1i); psi_i_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_3, y1i); psi_i_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_3, y1i); psi_i_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_3, y1i); psi_i_m3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_3, y1i); psi_i_m3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_5, y1i); psi_i_m5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_5, y1i); psi_i_m5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_5, y1i); psi_i_m5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_5, y1i); psi_i_m5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_5, y1i); psi_i_m5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_5, y1i); psi_i_m5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_5, y1i); psi_i_m5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_5, y1i); psi_i_m5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_7, y1i); psi_i_m7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_7, y1i); psi_i_m7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_7, y1i); psi_i_m7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_7, y1i); psi_i_m7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_7, y1i); psi_i_m7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_7, y1i); psi_i_m7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_7, y1i); psi_i_m7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_7, y1i); psi_i_m7_m7 = _mm_abs_epi16(xmm2);


        // Rearrange desired MF output
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 
    
        // Rearrange desired channel magnitudes
        xmm2 = ch_mag_128i[i]; // = [|h|^2(1),|h|^2(1),|h|^2(2),|h|^2(2)]*(2/sqrt(10))
        xmm3 = ch_mag_128i[i+1]; // = [|h|^2(3),|h|^2(3),|h|^2(4),|h|^2(4)]*(2/sqrt(10))
        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_des = _mm_unpacklo_epi64(xmm2,xmm3); 

        y0r_one_over_sqrt_21   = _mm_mulhi_epi16(y0r, ONE_OVER_SQRT_42);   
        y0r_three_over_sqrt_21 = _mm_mulhi_epi16(y0r, THREE_OVER_SQRT_42); 
        y0r_five_over_sqrt_21  = _mm_mulhi_epi16(y0r, FIVE_OVER_SQRT_42);  
        y0r_five_over_sqrt_21  = _mm_slli_epi16(y0r_five_over_sqrt_21, 1);
        y0r_seven_over_sqrt_21 = _mm_mulhi_epi16(y0r, SEVEN_OVER_SQRT_42); 
        y0r_seven_over_sqrt_21 = _mm_slli_epi16(y0r_seven_over_sqrt_21, 2); // Q2.14

        y0i_one_over_sqrt_21   = _mm_mulhi_epi16(y0i, ONE_OVER_SQRT_42);   
        y0i_three_over_sqrt_21 = _mm_mulhi_epi16(y0i, THREE_OVER_SQRT_42); 
        y0i_five_over_sqrt_21  = _mm_mulhi_epi16(y0i, FIVE_OVER_SQRT_42);  
        y0i_five_over_sqrt_21  = _mm_slli_epi16(y0i_five_over_sqrt_21, 1);
        y0i_seven_over_sqrt_21 = _mm_mulhi_epi16(y0i, SEVEN_OVER_SQRT_42); 
        y0i_seven_over_sqrt_21 = _mm_slli_epi16(y0i_seven_over_sqrt_21, 2); // Q2.14

        y0_p_7_1 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_7_3 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_7_5 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_7_7 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_5_1 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_5_3 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_5_5 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_5_7 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_3_1 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_3_3 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_3_5 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_3_7 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_1_1 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_1_3 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_1_5 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_1_7 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_seven_over_sqrt_21);

        y0_m_1_1 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_1_3 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_1_5 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_1_7 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_3_1 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_3_3 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_3_5 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_3_7 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_5_1 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_5_3 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_5_5 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_5_7 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_7_1 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_7_3 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_7_5 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_7_7 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_seven_over_sqrt_21);

        // divide by sqrt(2)
        psi_r_p7_p7 = _mm_mulhi_epi16(psi_r_p7_p7, ONE_OVER_SQRT_2);
        psi_r_p7_p7 = _mm_slli_epi16(psi_r_p7_p7, 1);
        psi_r_p7_p5 = _mm_mulhi_epi16(psi_r_p7_p5, ONE_OVER_SQRT_2);
        psi_r_p7_p5 = _mm_slli_epi16(psi_r_p7_p5, 1);
        psi_r_p7_p3 = _mm_mulhi_epi16(psi_r_p7_p3, ONE_OVER_SQRT_2);
        psi_r_p7_p3 = _mm_slli_epi16(psi_r_p7_p3, 1);
        psi_r_p7_p1 = _mm_mulhi_epi16(psi_r_p7_p1, ONE_OVER_SQRT_2);
        psi_r_p7_p1 = _mm_slli_epi16(psi_r_p7_p1, 1);
        psi_r_p7_m1 = _mm_mulhi_epi16(psi_r_p7_m1, ONE_OVER_SQRT_2);
        psi_r_p7_m1 = _mm_slli_epi16(psi_r_p7_m1, 1);
        psi_r_p7_m3 = _mm_mulhi_epi16(psi_r_p7_m3, ONE_OVER_SQRT_2);
        psi_r_p7_m3 = _mm_slli_epi16(psi_r_p7_m3, 1);
        psi_r_p7_m5 = _mm_mulhi_epi16(psi_r_p7_m5, ONE_OVER_SQRT_2);
        psi_r_p7_m5 = _mm_slli_epi16(psi_r_p7_m5, 1);
        psi_r_p7_m7 = _mm_mulhi_epi16(psi_r_p7_m7, ONE_OVER_SQRT_2);
        psi_r_p7_m7 = _mm_slli_epi16(psi_r_p7_m7, 1);
        psi_r_p5_p7 = _mm_mulhi_epi16(psi_r_p5_p7, ONE_OVER_SQRT_2);
        psi_r_p5_p7 = _mm_slli_epi16(psi_r_p5_p7, 1);
        psi_r_p5_p5 = _mm_mulhi_epi16(psi_r_p5_p5, ONE_OVER_SQRT_2);
        psi_r_p5_p5 = _mm_slli_epi16(psi_r_p5_p5, 1);
        psi_r_p5_p3 = _mm_mulhi_epi16(psi_r_p5_p3, ONE_OVER_SQRT_2);
        psi_r_p5_p3 = _mm_slli_epi16(psi_r_p5_p3, 1);
        psi_r_p5_p1 = _mm_mulhi_epi16(psi_r_p5_p1, ONE_OVER_SQRT_2);
        psi_r_p5_p1 = _mm_slli_epi16(psi_r_p5_p1, 1);
        psi_r_p5_m1 = _mm_mulhi_epi16(psi_r_p5_m1, ONE_OVER_SQRT_2);
        psi_r_p5_m1 = _mm_slli_epi16(psi_r_p5_m1, 1);
        psi_r_p5_m3 = _mm_mulhi_epi16(psi_r_p5_m3, ONE_OVER_SQRT_2);
        psi_r_p5_m3 = _mm_slli_epi16(psi_r_p5_m3, 1);
        psi_r_p5_m5 = _mm_mulhi_epi16(psi_r_p5_m5, ONE_OVER_SQRT_2);
        psi_r_p5_m5 = _mm_slli_epi16(psi_r_p5_m5, 1);
        psi_r_p5_m7 = _mm_mulhi_epi16(psi_r_p5_m7, ONE_OVER_SQRT_2);
        psi_r_p5_m7 = _mm_slli_epi16(psi_r_p5_m7, 1);
        psi_r_p3_p7 = _mm_mulhi_epi16(psi_r_p3_p7, ONE_OVER_SQRT_2);
        psi_r_p3_p7 = _mm_slli_epi16(psi_r_p3_p7, 1);
        psi_r_p3_p5 = _mm_mulhi_epi16(psi_r_p3_p5, ONE_OVER_SQRT_2);
        psi_r_p3_p5 = _mm_slli_epi16(psi_r_p3_p5, 1);
        psi_r_p3_p3 = _mm_mulhi_epi16(psi_r_p3_p3, ONE_OVER_SQRT_2);
        psi_r_p3_p3 = _mm_slli_epi16(psi_r_p3_p3, 1);
        psi_r_p3_p1 = _mm_mulhi_epi16(psi_r_p3_p1, ONE_OVER_SQRT_2);
        psi_r_p3_p1 = _mm_slli_epi16(psi_r_p3_p1, 1);
        psi_r_p3_m1 = _mm_mulhi_epi16(psi_r_p3_m1, ONE_OVER_SQRT_2);
        psi_r_p3_m1 = _mm_slli_epi16(psi_r_p3_m1, 1);
        psi_r_p3_m3 = _mm_mulhi_epi16(psi_r_p3_m3, ONE_OVER_SQRT_2);
        psi_r_p3_m3 = _mm_slli_epi16(psi_r_p3_m3, 1);
        psi_r_p3_m5 = _mm_mulhi_epi16(psi_r_p3_m5, ONE_OVER_SQRT_2);
        psi_r_p3_m5 = _mm_slli_epi16(psi_r_p3_m5, 1);
        psi_r_p3_m7 = _mm_mulhi_epi16(psi_r_p3_m7, ONE_OVER_SQRT_2);
        psi_r_p3_m7 = _mm_slli_epi16(psi_r_p3_m7, 1);
        psi_r_p1_p7 = _mm_mulhi_epi16(psi_r_p1_p7, ONE_OVER_SQRT_2);
        psi_r_p1_p7 = _mm_slli_epi16(psi_r_p1_p7, 1);
        psi_r_p1_p5 = _mm_mulhi_epi16(psi_r_p1_p5, ONE_OVER_SQRT_2);
        psi_r_p1_p5 = _mm_slli_epi16(psi_r_p1_p5, 1);
        psi_r_p1_p3 = _mm_mulhi_epi16(psi_r_p1_p3, ONE_OVER_SQRT_2);
        psi_r_p1_p3 = _mm_slli_epi16(psi_r_p1_p3, 1);
        psi_r_p1_p1 = _mm_mulhi_epi16(psi_r_p1_p1, ONE_OVER_SQRT_2);
        psi_r_p1_p1 = _mm_slli_epi16(psi_r_p1_p1, 1);
        psi_r_p1_m1 = _mm_mulhi_epi16(psi_r_p1_m1, ONE_OVER_SQRT_2);
        psi_r_p1_m1 = _mm_slli_epi16(psi_r_p1_m1, 1);
        psi_r_p1_m3 = _mm_mulhi_epi16(psi_r_p1_m3, ONE_OVER_SQRT_2);
        psi_r_p1_m3 = _mm_slli_epi16(psi_r_p1_m3, 1);
        psi_r_p1_m5 = _mm_mulhi_epi16(psi_r_p1_m5, ONE_OVER_SQRT_2);
        psi_r_p1_m5 = _mm_slli_epi16(psi_r_p1_m5, 1);
        psi_r_p1_m7 = _mm_mulhi_epi16(psi_r_p1_m7, ONE_OVER_SQRT_2);
        psi_r_p1_m7 = _mm_slli_epi16(psi_r_p1_m7, 1);
        psi_r_m1_p7 = _mm_mulhi_epi16(psi_r_m1_p7, ONE_OVER_SQRT_2);
        psi_r_m1_p7 = _mm_slli_epi16(psi_r_m1_p7, 1);
        psi_r_m1_p5 = _mm_mulhi_epi16(psi_r_m1_p5, ONE_OVER_SQRT_2);
        psi_r_m1_p5 = _mm_slli_epi16(psi_r_m1_p5, 1);
        psi_r_m1_p3 = _mm_mulhi_epi16(psi_r_m1_p3, ONE_OVER_SQRT_2);
        psi_r_m1_p3 = _mm_slli_epi16(psi_r_m1_p3, 1);
        psi_r_m1_p1 = _mm_mulhi_epi16(psi_r_m1_p1, ONE_OVER_SQRT_2);
        psi_r_m1_p1 = _mm_slli_epi16(psi_r_m1_p1, 1);
        psi_r_m1_m1 = _mm_mulhi_epi16(psi_r_m1_m1, ONE_OVER_SQRT_2);
        psi_r_m1_m1 = _mm_slli_epi16(psi_r_m1_m1, 1);
        psi_r_m1_m3 = _mm_mulhi_epi16(psi_r_m1_m3, ONE_OVER_SQRT_2);
        psi_r_m1_m3 = _mm_slli_epi16(psi_r_m1_m3, 1);
        psi_r_m1_m5 = _mm_mulhi_epi16(psi_r_m1_m5, ONE_OVER_SQRT_2);
        psi_r_m1_m5 = _mm_slli_epi16(psi_r_m1_m5, 1);
        psi_r_m1_m7 = _mm_mulhi_epi16(psi_r_m1_m7, ONE_OVER_SQRT_2);
        psi_r_m1_m7 = _mm_slli_epi16(psi_r_m1_m7, 1);
        psi_r_m3_p7 = _mm_mulhi_epi16(psi_r_m3_p7, ONE_OVER_SQRT_2);
        psi_r_m3_p7 = _mm_slli_epi16(psi_r_m3_p7, 1);
        psi_r_m3_p5 = _mm_mulhi_epi16(psi_r_m3_p5, ONE_OVER_SQRT_2);
        psi_r_m3_p5 = _mm_slli_epi16(psi_r_m3_p5, 1);
        psi_r_m3_p3 = _mm_mulhi_epi16(psi_r_m3_p3, ONE_OVER_SQRT_2);
        psi_r_m3_p3 = _mm_slli_epi16(psi_r_m3_p3, 1);
        psi_r_m3_p1 = _mm_mulhi_epi16(psi_r_m3_p1, ONE_OVER_SQRT_2);
        psi_r_m3_p1 = _mm_slli_epi16(psi_r_m3_p1, 1);
        psi_r_m3_m1 = _mm_mulhi_epi16(psi_r_m3_m1, ONE_OVER_SQRT_2);
        psi_r_m3_m1 = _mm_slli_epi16(psi_r_m3_m1, 1);
        psi_r_m3_m3 = _mm_mulhi_epi16(psi_r_m3_m3, ONE_OVER_SQRT_2);
        psi_r_m3_m3 = _mm_slli_epi16(psi_r_m3_m3, 1);
        psi_r_m3_m5 = _mm_mulhi_epi16(psi_r_m3_m5, ONE_OVER_SQRT_2);
        psi_r_m3_m5 = _mm_slli_epi16(psi_r_m3_m5, 1);
        psi_r_m3_m7 = _mm_mulhi_epi16(psi_r_m3_m7, ONE_OVER_SQRT_2);
        psi_r_m3_m7 = _mm_slli_epi16(psi_r_m3_m7, 1);
        psi_r_m5_p7 = _mm_mulhi_epi16(psi_r_m5_p7, ONE_OVER_SQRT_2);
        psi_r_m5_p7 = _mm_slli_epi16(psi_r_m5_p7, 1);
        psi_r_m5_p5 = _mm_mulhi_epi16(psi_r_m5_p5, ONE_OVER_SQRT_2);
        psi_r_m5_p5 = _mm_slli_epi16(psi_r_m5_p5, 1);
        psi_r_m5_p3 = _mm_mulhi_epi16(psi_r_m5_p3, ONE_OVER_SQRT_2);
        psi_r_m5_p3 = _mm_slli_epi16(psi_r_m5_p3, 1);
        psi_r_m5_p1 = _mm_mulhi_epi16(psi_r_m5_p1, ONE_OVER_SQRT_2);
        psi_r_m5_p1 = _mm_slli_epi16(psi_r_m5_p1, 1);
        psi_r_m5_m1 = _mm_mulhi_epi16(psi_r_m5_m1, ONE_OVER_SQRT_2);
        psi_r_m5_m1 = _mm_slli_epi16(psi_r_m5_m1, 1);
        psi_r_m5_m3 = _mm_mulhi_epi16(psi_r_m5_m3, ONE_OVER_SQRT_2);
        psi_r_m5_m3 = _mm_slli_epi16(psi_r_m5_m3, 1);
        psi_r_m5_m5 = _mm_mulhi_epi16(psi_r_m5_m5, ONE_OVER_SQRT_2);
        psi_r_m5_m5 = _mm_slli_epi16(psi_r_m5_m5, 1);
        psi_r_m5_m7 = _mm_mulhi_epi16(psi_r_m5_m7, ONE_OVER_SQRT_2);
        psi_r_m5_m7 = _mm_slli_epi16(psi_r_m5_m7, 1);
        psi_r_m7_p7 = _mm_mulhi_epi16(psi_r_m7_p7, ONE_OVER_SQRT_2);
        psi_r_m7_p7 = _mm_slli_epi16(psi_r_m7_p7, 1);
        psi_r_m7_p5 = _mm_mulhi_epi16(psi_r_m7_p5, ONE_OVER_SQRT_2);
        psi_r_m7_p5 = _mm_slli_epi16(psi_r_m7_p5, 1);
        psi_r_m7_p3 = _mm_mulhi_epi16(psi_r_m7_p3, ONE_OVER_SQRT_2);
        psi_r_m7_p3 = _mm_slli_epi16(psi_r_m7_p3, 1);
        psi_r_m7_p1 = _mm_mulhi_epi16(psi_r_m7_p1, ONE_OVER_SQRT_2);
        psi_r_m7_p1 = _mm_slli_epi16(psi_r_m7_p1, 1);
        psi_r_m7_m1 = _mm_mulhi_epi16(psi_r_m7_m1, ONE_OVER_SQRT_2);
        psi_r_m7_m1 = _mm_slli_epi16(psi_r_m7_m1, 1);
        psi_r_m7_m3 = _mm_mulhi_epi16(psi_r_m7_m3, ONE_OVER_SQRT_2);
        psi_r_m7_m3 = _mm_slli_epi16(psi_r_m7_m3, 1);
        psi_r_m7_m5 = _mm_mulhi_epi16(psi_r_m7_m5, ONE_OVER_SQRT_2);
        psi_r_m7_m5 = _mm_slli_epi16(psi_r_m7_m5, 1);
        psi_r_m7_m7 = _mm_mulhi_epi16(psi_r_m7_m7, ONE_OVER_SQRT_2);
        psi_r_m7_m7 = _mm_slli_epi16(psi_r_m7_m7, 1);

        psi_i_p7_p7 = _mm_mulhi_epi16(psi_i_p7_p7, ONE_OVER_SQRT_2);
        psi_i_p7_p7 = _mm_slli_epi16(psi_i_p7_p7, 1);
        psi_i_p7_p5 = _mm_mulhi_epi16(psi_i_p7_p5, ONE_OVER_SQRT_2);
        psi_i_p7_p5 = _mm_slli_epi16(psi_i_p7_p5, 1);
        psi_i_p7_p3 = _mm_mulhi_epi16(psi_i_p7_p3, ONE_OVER_SQRT_2);
        psi_i_p7_p3 = _mm_slli_epi16(psi_i_p7_p3, 1);
        psi_i_p7_p1 = _mm_mulhi_epi16(psi_i_p7_p1, ONE_OVER_SQRT_2);
        psi_i_p7_p1 = _mm_slli_epi16(psi_i_p7_p1, 1);
        psi_i_p7_m1 = _mm_mulhi_epi16(psi_i_p7_m1, ONE_OVER_SQRT_2);
        psi_i_p7_m1 = _mm_slli_epi16(psi_i_p7_m1, 1);
        psi_i_p7_m3 = _mm_mulhi_epi16(psi_i_p7_m3, ONE_OVER_SQRT_2);
        psi_i_p7_m3 = _mm_slli_epi16(psi_i_p7_m3, 1);
        psi_i_p7_m5 = _mm_mulhi_epi16(psi_i_p7_m5, ONE_OVER_SQRT_2);
        psi_i_p7_m5 = _mm_slli_epi16(psi_i_p7_m5, 1);
        psi_i_p7_m7 = _mm_mulhi_epi16(psi_i_p7_m7, ONE_OVER_SQRT_2);
        psi_i_p7_m7 = _mm_slli_epi16(psi_i_p7_m7, 1);
        psi_i_p5_p7 = _mm_mulhi_epi16(psi_i_p5_p7, ONE_OVER_SQRT_2);
        psi_i_p5_p7 = _mm_slli_epi16(psi_i_p5_p7, 1);
        psi_i_p5_p5 = _mm_mulhi_epi16(psi_i_p5_p5, ONE_OVER_SQRT_2);
        psi_i_p5_p5 = _mm_slli_epi16(psi_i_p5_p5, 1);
        psi_i_p5_p3 = _mm_mulhi_epi16(psi_i_p5_p3, ONE_OVER_SQRT_2);
        psi_i_p5_p3 = _mm_slli_epi16(psi_i_p5_p3, 1);
        psi_i_p5_p1 = _mm_mulhi_epi16(psi_i_p5_p1, ONE_OVER_SQRT_2);
        psi_i_p5_p1 = _mm_slli_epi16(psi_i_p5_p1, 1);
        psi_i_p5_m1 = _mm_mulhi_epi16(psi_i_p5_m1, ONE_OVER_SQRT_2);
        psi_i_p5_m1 = _mm_slli_epi16(psi_i_p5_m1, 1);
        psi_i_p5_m3 = _mm_mulhi_epi16(psi_i_p5_m3, ONE_OVER_SQRT_2);
        psi_i_p5_m3 = _mm_slli_epi16(psi_i_p5_m3, 1);
        psi_i_p5_m5 = _mm_mulhi_epi16(psi_i_p5_m5, ONE_OVER_SQRT_2);
        psi_i_p5_m5 = _mm_slli_epi16(psi_i_p5_m5, 1);
        psi_i_p5_m7 = _mm_mulhi_epi16(psi_i_p5_m7, ONE_OVER_SQRT_2);
        psi_i_p5_m7 = _mm_slli_epi16(psi_i_p5_m7, 1);
        psi_i_p3_p7 = _mm_mulhi_epi16(psi_i_p3_p7, ONE_OVER_SQRT_2);
        psi_i_p3_p7 = _mm_slli_epi16(psi_i_p3_p7, 1);
        psi_i_p3_p5 = _mm_mulhi_epi16(psi_i_p3_p5, ONE_OVER_SQRT_2);
        psi_i_p3_p5 = _mm_slli_epi16(psi_i_p3_p5, 1);
        psi_i_p3_p3 = _mm_mulhi_epi16(psi_i_p3_p3, ONE_OVER_SQRT_2);
        psi_i_p3_p3 = _mm_slli_epi16(psi_i_p3_p3, 1);
        psi_i_p3_p1 = _mm_mulhi_epi16(psi_i_p3_p1, ONE_OVER_SQRT_2);
        psi_i_p3_p1 = _mm_slli_epi16(psi_i_p3_p1, 1);
        psi_i_p3_m1 = _mm_mulhi_epi16(psi_i_p3_m1, ONE_OVER_SQRT_2);
        psi_i_p3_m1 = _mm_slli_epi16(psi_i_p3_m1, 1);
        psi_i_p3_m3 = _mm_mulhi_epi16(psi_i_p3_m3, ONE_OVER_SQRT_2);
        psi_i_p3_m3 = _mm_slli_epi16(psi_i_p3_m3, 1);
        psi_i_p3_m5 = _mm_mulhi_epi16(psi_i_p3_m5, ONE_OVER_SQRT_2);
        psi_i_p3_m5 = _mm_slli_epi16(psi_i_p3_m5, 1);
        psi_i_p3_m7 = _mm_mulhi_epi16(psi_i_p3_m7, ONE_OVER_SQRT_2);
        psi_i_p3_m7 = _mm_slli_epi16(psi_i_p3_m7, 1);
        psi_i_p1_p7 = _mm_mulhi_epi16(psi_i_p1_p7, ONE_OVER_SQRT_2);
        psi_i_p1_p7 = _mm_slli_epi16(psi_i_p1_p7, 1);
        psi_i_p1_p5 = _mm_mulhi_epi16(psi_i_p1_p5, ONE_OVER_SQRT_2);
        psi_i_p1_p5 = _mm_slli_epi16(psi_i_p1_p5, 1);
        psi_i_p1_p3 = _mm_mulhi_epi16(psi_i_p1_p3, ONE_OVER_SQRT_2);
        psi_i_p1_p3 = _mm_slli_epi16(psi_i_p1_p3, 1);
        psi_i_p1_p1 = _mm_mulhi_epi16(psi_i_p1_p1, ONE_OVER_SQRT_2);
        psi_i_p1_p1 = _mm_slli_epi16(psi_i_p1_p1, 1);
        psi_i_p1_m1 = _mm_mulhi_epi16(psi_i_p1_m1, ONE_OVER_SQRT_2);
        psi_i_p1_m1 = _mm_slli_epi16(psi_i_p1_m1, 1);
        psi_i_p1_m3 = _mm_mulhi_epi16(psi_i_p1_m3, ONE_OVER_SQRT_2);
        psi_i_p1_m3 = _mm_slli_epi16(psi_i_p1_m3, 1);
        psi_i_p1_m5 = _mm_mulhi_epi16(psi_i_p1_m5, ONE_OVER_SQRT_2);
        psi_i_p1_m5 = _mm_slli_epi16(psi_i_p1_m5, 1);
        psi_i_p1_m7 = _mm_mulhi_epi16(psi_i_p1_m7, ONE_OVER_SQRT_2);
        psi_i_p1_m7 = _mm_slli_epi16(psi_i_p1_m7, 1);
        psi_i_m1_p7 = _mm_mulhi_epi16(psi_i_m1_p7, ONE_OVER_SQRT_2);
        psi_i_m1_p7 = _mm_slli_epi16(psi_i_m1_p7, 1);
        psi_i_m1_p5 = _mm_mulhi_epi16(psi_i_m1_p5, ONE_OVER_SQRT_2);
        psi_i_m1_p5 = _mm_slli_epi16(psi_i_m1_p5, 1);
        psi_i_m1_p3 = _mm_mulhi_epi16(psi_i_m1_p3, ONE_OVER_SQRT_2);
        psi_i_m1_p3 = _mm_slli_epi16(psi_i_m1_p3, 1);
        psi_i_m1_p1 = _mm_mulhi_epi16(psi_i_m1_p1, ONE_OVER_SQRT_2);
        psi_i_m1_p1 = _mm_slli_epi16(psi_i_m1_p1, 1);
        psi_i_m1_m1 = _mm_mulhi_epi16(psi_i_m1_m1, ONE_OVER_SQRT_2);
        psi_i_m1_m1 = _mm_slli_epi16(psi_i_m1_m1, 1);
        psi_i_m1_m3 = _mm_mulhi_epi16(psi_i_m1_m3, ONE_OVER_SQRT_2);
        psi_i_m1_m3 = _mm_slli_epi16(psi_i_m1_m3, 1);
        psi_i_m1_m5 = _mm_mulhi_epi16(psi_i_m1_m5, ONE_OVER_SQRT_2);
        psi_i_m1_m5 = _mm_slli_epi16(psi_i_m1_m5, 1);
        psi_i_m1_m7 = _mm_mulhi_epi16(psi_i_m1_m7, ONE_OVER_SQRT_2);
        psi_i_m1_m7 = _mm_slli_epi16(psi_i_m1_m7, 1);
        psi_i_m3_p7 = _mm_mulhi_epi16(psi_i_m3_p7, ONE_OVER_SQRT_2);
        psi_i_m3_p7 = _mm_slli_epi16(psi_i_m3_p7, 1);
        psi_i_m3_p5 = _mm_mulhi_epi16(psi_i_m3_p5, ONE_OVER_SQRT_2);
        psi_i_m3_p5 = _mm_slli_epi16(psi_i_m3_p5, 1);
        psi_i_m3_p3 = _mm_mulhi_epi16(psi_i_m3_p3, ONE_OVER_SQRT_2);
        psi_i_m3_p3 = _mm_slli_epi16(psi_i_m3_p3, 1);
        psi_i_m3_p1 = _mm_mulhi_epi16(psi_i_m3_p1, ONE_OVER_SQRT_2);
        psi_i_m3_p1 = _mm_slli_epi16(psi_i_m3_p1, 1);
        psi_i_m3_m1 = _mm_mulhi_epi16(psi_i_m3_m1, ONE_OVER_SQRT_2);
        psi_i_m3_m1 = _mm_slli_epi16(psi_i_m3_m1, 1);
        psi_i_m3_m3 = _mm_mulhi_epi16(psi_i_m3_m3, ONE_OVER_SQRT_2);
        psi_i_m3_m3 = _mm_slli_epi16(psi_i_m3_m3, 1);
        psi_i_m3_m5 = _mm_mulhi_epi16(psi_i_m3_m5, ONE_OVER_SQRT_2);
        psi_i_m3_m5 = _mm_slli_epi16(psi_i_m3_m5, 1);
        psi_i_m3_m7 = _mm_mulhi_epi16(psi_i_m3_m7, ONE_OVER_SQRT_2);
        psi_i_m3_m7 = _mm_slli_epi16(psi_i_m3_m7, 1);
        psi_i_m5_p7 = _mm_mulhi_epi16(psi_i_m5_p7, ONE_OVER_SQRT_2);
        psi_i_m5_p7 = _mm_slli_epi16(psi_i_m5_p7, 1);
        psi_i_m5_p5 = _mm_mulhi_epi16(psi_i_m5_p5, ONE_OVER_SQRT_2);
        psi_i_m5_p5 = _mm_slli_epi16(psi_i_m5_p5, 1);
        psi_i_m5_p3 = _mm_mulhi_epi16(psi_i_m5_p3, ONE_OVER_SQRT_2);
        psi_i_m5_p3 = _mm_slli_epi16(psi_i_m5_p3, 1);
        psi_i_m5_p1 = _mm_mulhi_epi16(psi_i_m5_p1, ONE_OVER_SQRT_2);
        psi_i_m5_p1 = _mm_slli_epi16(psi_i_m5_p1, 1);
        psi_i_m5_m1 = _mm_mulhi_epi16(psi_i_m5_m1, ONE_OVER_SQRT_2);
        psi_i_m5_m1 = _mm_slli_epi16(psi_i_m5_m1, 1);
        psi_i_m5_m3 = _mm_mulhi_epi16(psi_i_m5_m3, ONE_OVER_SQRT_2);
        psi_i_m5_m3 = _mm_slli_epi16(psi_i_m5_m3, 1);
        psi_i_m5_m5 = _mm_mulhi_epi16(psi_i_m5_m5, ONE_OVER_SQRT_2);
        psi_i_m5_m5 = _mm_slli_epi16(psi_i_m5_m5, 1);
        psi_i_m5_m7 = _mm_mulhi_epi16(psi_i_m5_m7, ONE_OVER_SQRT_2);
        psi_i_m5_m7 = _mm_slli_epi16(psi_i_m5_m7, 1);
        psi_i_m7_p7 = _mm_mulhi_epi16(psi_i_m7_p7, ONE_OVER_SQRT_2);
        psi_i_m7_p7 = _mm_slli_epi16(psi_i_m7_p7, 1);
        psi_i_m7_p5 = _mm_mulhi_epi16(psi_i_m7_p5, ONE_OVER_SQRT_2);
        psi_i_m7_p5 = _mm_slli_epi16(psi_i_m7_p5, 1);
        psi_i_m7_p3 = _mm_mulhi_epi16(psi_i_m7_p3, ONE_OVER_SQRT_2);
        psi_i_m7_p3 = _mm_slli_epi16(psi_i_m7_p3, 1);
        psi_i_m7_p1 = _mm_mulhi_epi16(psi_i_m7_p1, ONE_OVER_SQRT_2);
        psi_i_m7_p1 = _mm_slli_epi16(psi_i_m7_p1, 1);
        psi_i_m7_m1 = _mm_mulhi_epi16(psi_i_m7_m1, ONE_OVER_SQRT_2);
        psi_i_m7_m1 = _mm_slli_epi16(psi_i_m7_m1, 1);
        psi_i_m7_m3 = _mm_mulhi_epi16(psi_i_m7_m3, ONE_OVER_SQRT_2);
        psi_i_m7_m3 = _mm_slli_epi16(psi_i_m7_m3, 1);
        psi_i_m7_m5 = _mm_mulhi_epi16(psi_i_m7_m5, ONE_OVER_SQRT_2);
        psi_i_m7_m5 = _mm_slli_epi16(psi_i_m7_m5, 1);
        psi_i_m7_m7 = _mm_mulhi_epi16(psi_i_m7_m7, ONE_OVER_SQRT_2);
        psi_i_m7_m7 = _mm_slli_epi16(psi_i_m7_m7, 1);

        psi_a_p7_p7 = _mm_adds_epi16(psi_r_p7_p7, psi_i_p7_p7); 
        psi_a_p7_p5 = _mm_adds_epi16(psi_r_p7_p5, psi_i_p7_p5);
        psi_a_p7_p3 = _mm_adds_epi16(psi_r_p7_p3, psi_i_p7_p3);
        psi_a_p7_p1 = _mm_adds_epi16(psi_r_p7_p1, psi_i_p7_p1);
        psi_a_p7_m1 = _mm_adds_epi16(psi_r_p7_m1, psi_i_p7_m1);
        psi_a_p7_m3 = _mm_adds_epi16(psi_r_p7_m3, psi_i_p7_m3);
        psi_a_p7_m5 = _mm_adds_epi16(psi_r_p7_m5, psi_i_p7_m5);
        psi_a_p7_m7 = _mm_adds_epi16(psi_r_p7_m7, psi_i_p7_m7);
        psi_a_p5_p7 = _mm_adds_epi16(psi_r_p5_p7, psi_i_p5_p7);
        psi_a_p5_p5 = _mm_adds_epi16(psi_r_p5_p5, psi_i_p5_p5);
        psi_a_p5_p3 = _mm_adds_epi16(psi_r_p5_p3, psi_i_p5_p3);
        psi_a_p5_p1 = _mm_adds_epi16(psi_r_p5_p1, psi_i_p5_p1);
        psi_a_p5_m1 = _mm_adds_epi16(psi_r_p5_m1, psi_i_p5_m1);
        psi_a_p5_m3 = _mm_adds_epi16(psi_r_p5_m3, psi_i_p5_m3);
        psi_a_p5_m5 = _mm_adds_epi16(psi_r_p5_m5, psi_i_p5_m5);
        psi_a_p5_m7 = _mm_adds_epi16(psi_r_p5_m7, psi_i_p5_m7);
        psi_a_p3_p7 = _mm_adds_epi16(psi_r_p3_p7, psi_i_p3_p7);
        psi_a_p3_p5 = _mm_adds_epi16(psi_r_p3_p5, psi_i_p3_p5);
        psi_a_p3_p3 = _mm_adds_epi16(psi_r_p3_p3, psi_i_p3_p3);
        psi_a_p3_p1 = _mm_adds_epi16(psi_r_p3_p1, psi_i_p3_p1);
        psi_a_p3_m1 = _mm_adds_epi16(psi_r_p3_m1, psi_i_p3_m1);
        psi_a_p3_m3 = _mm_adds_epi16(psi_r_p3_m3, psi_i_p3_m3);
        psi_a_p3_m5 = _mm_adds_epi16(psi_r_p3_m5, psi_i_p3_m5);
        psi_a_p3_m7 = _mm_adds_epi16(psi_r_p3_m7, psi_i_p3_m7);
        psi_a_p1_p7 = _mm_adds_epi16(psi_r_p1_p7, psi_i_p1_p7);
        psi_a_p1_p5 = _mm_adds_epi16(psi_r_p1_p5, psi_i_p1_p5);
        psi_a_p1_p3 = _mm_adds_epi16(psi_r_p1_p3, psi_i_p1_p3);
        psi_a_p1_p1 = _mm_adds_epi16(psi_r_p1_p1, psi_i_p1_p1);
        psi_a_p1_m1 = _mm_adds_epi16(psi_r_p1_m1, psi_i_p1_m1);
        psi_a_p1_m3 = _mm_adds_epi16(psi_r_p1_m3, psi_i_p1_m3);
        psi_a_p1_m5 = _mm_adds_epi16(psi_r_p1_m5, psi_i_p1_m5);
        psi_a_p1_m7 = _mm_adds_epi16(psi_r_p1_m7, psi_i_p1_m7);           
        psi_a_m1_p7 = _mm_adds_epi16(psi_r_m1_p7, psi_i_m1_p7);
        psi_a_m1_p5 = _mm_adds_epi16(psi_r_m1_p5, psi_i_m1_p5);
        psi_a_m1_p3 = _mm_adds_epi16(psi_r_m1_p3, psi_i_m1_p3);
        psi_a_m1_p1 = _mm_adds_epi16(psi_r_m1_p1, psi_i_m1_p1);
        psi_a_m1_m1 = _mm_adds_epi16(psi_r_m1_m1, psi_i_m1_m1);
        psi_a_m1_m3 = _mm_adds_epi16(psi_r_m1_m3, psi_i_m1_m3);
        psi_a_m1_m5 = _mm_adds_epi16(psi_r_m1_m5, psi_i_m1_m5);
        psi_a_m1_m7 = _mm_adds_epi16(psi_r_m1_m7, psi_i_m1_m7);
        psi_a_m3_p7 = _mm_adds_epi16(psi_r_m3_p7, psi_i_m3_p7);
        psi_a_m3_p5 = _mm_adds_epi16(psi_r_m3_p5, psi_i_m3_p5);
        psi_a_m3_p3 = _mm_adds_epi16(psi_r_m3_p3, psi_i_m3_p3);
        psi_a_m3_p1 = _mm_adds_epi16(psi_r_m3_p1, psi_i_m3_p1);
        psi_a_m3_m1 = _mm_adds_epi16(psi_r_m3_m1, psi_i_m3_m1);
        psi_a_m3_m3 = _mm_adds_epi16(psi_r_m3_m3, psi_i_m3_m3);
        psi_a_m3_m5 = _mm_adds_epi16(psi_r_m3_m5, psi_i_m3_m5);
        psi_a_m3_m7 = _mm_adds_epi16(psi_r_m3_m7, psi_i_m3_m7);
        psi_a_m5_p7 = _mm_adds_epi16(psi_r_m5_p7, psi_i_m5_p7);
        psi_a_m5_p5 = _mm_adds_epi16(psi_r_m5_p5, psi_i_m5_p5);
        psi_a_m5_p3 = _mm_adds_epi16(psi_r_m5_p3, psi_i_m5_p3);
        psi_a_m5_p1 = _mm_adds_epi16(psi_r_m5_p1, psi_i_m5_p1);
        psi_a_m5_m1 = _mm_adds_epi16(psi_r_m5_m1, psi_i_m5_m1);
        psi_a_m5_m3 = _mm_adds_epi16(psi_r_m5_m3, psi_i_m5_m3);
        psi_a_m5_m5 = _mm_adds_epi16(psi_r_m5_m5, psi_i_m5_m5);
        psi_a_m5_m7 = _mm_adds_epi16(psi_r_m5_m7, psi_i_m5_m7);
        psi_a_m7_p7 = _mm_adds_epi16(psi_r_m7_p7, psi_i_m7_p7);
        psi_a_m7_p5 = _mm_adds_epi16(psi_r_m7_p5, psi_i_m7_p5);
        psi_a_m7_p3 = _mm_adds_epi16(psi_r_m7_p3, psi_i_m7_p3);
        psi_a_m7_p1 = _mm_adds_epi16(psi_r_m7_p1, psi_i_m7_p1);
        psi_a_m7_m1 = _mm_adds_epi16(psi_r_m7_m1, psi_i_m7_m1);
        psi_a_m7_m3 = _mm_adds_epi16(psi_r_m7_m3, psi_i_m7_m3);
        psi_a_m7_m5 = _mm_adds_epi16(psi_r_m7_m5, psi_i_m7_m5);
        psi_a_m7_m7 = _mm_adds_epi16(psi_r_m7_m7, psi_i_m7_m7); 

        // Computing different multiples of ||h0||^2
        // x=1, y=1
        ch_mag_2_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,ONE_OVER_FOUR_SQRT_42);
        ch_mag_2_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_2_over_42_with_sigma2,1);
        // x=1, y=3
        ch_mag_10_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,FIVE_OVER_FOUR_SQRT_42);
        ch_mag_10_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_10_over_42_with_sigma2,1);
        // x=1, x=5
        ch_mag_26_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,THIRTEEN_OVER_FOUR_SQRT_42);
        ch_mag_26_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_26_over_42_with_sigma2,1);
        // x=1, y=7
        ch_mag_50_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYFIVE_OVER_FOUR_SQRT_42);
        ch_mag_50_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_50_over_42_with_sigma2,1);
        // x=3, y=3
        ch_mag_18_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,NINE_OVER_FOUR_SQRT_42);
        ch_mag_18_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_18_over_42_with_sigma2,1);
        // x=3, y=5
        ch_mag_34_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,SEVENTEEN_OVER_FOUR_SQRT_42);
        ch_mag_34_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_34_over_42_with_sigma2,1);
        // x=3, y=7
        ch_mag_58_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYNINE_OVER_FOUR_SQRT_42);
        ch_mag_58_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_58_over_42_with_sigma2,2);
        // x=5, y=5
        ch_mag_50_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYFIVE_OVER_FOUR_SQRT_42);
        ch_mag_50_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_50_over_42_with_sigma2,1);
        // x=5, y=7
        ch_mag_74_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,THIRTYSEVEN_OVER_FOUR_SQRT_42);
        ch_mag_74_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_74_over_42_with_sigma2,2);
        // x=7, y=7
        ch_mag_98_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,FORTYNINE_OVER_FOUR_SQRT_42);
        ch_mag_98_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_98_over_42_with_sigma2,2);
        
        // Computing Metrics
        xmm1 = _mm_adds_epi16(psi_a_p7_p7, y0_p_7_7); bit_met_p7_p7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p7_p5, y0_p_7_5); bit_met_p7_p5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p7_p3, y0_p_7_3); bit_met_p7_p3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p7_p1, y0_p_7_1); bit_met_p7_p1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p7_m1, y0_m_7_1); bit_met_p7_m1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p7_m3, y0_m_7_3); bit_met_p7_m3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p7_m5, y0_m_7_5); bit_met_p7_m5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p7_m7, y0_m_7_7); bit_met_p7_m7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p5_p7, y0_p_5_7); bit_met_p5_p7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p5_p5, y0_p_5_5); bit_met_p5_p5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p5_p3, y0_p_5_3); bit_met_p5_p3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p5_p1, y0_p_5_1); bit_met_p5_p1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p5_m1, y0_m_5_1); bit_met_p5_m1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p5_m3, y0_m_5_3); bit_met_p5_m3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p5_m5, y0_m_5_5); bit_met_p5_m5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p5_m7, y0_m_5_7); bit_met_p5_m7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p3_p7, y0_p_3_7); bit_met_p3_p7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p3_p5, y0_p_3_5); bit_met_p3_p5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p3_p3, y0_p_3_3); bit_met_p3_p3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p3_p1, y0_p_3_1); bit_met_p3_p1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p3_m1, y0_m_3_1); bit_met_p3_m1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p3_m3, y0_m_3_3); bit_met_p3_m3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p3_m5, y0_m_3_5); bit_met_p3_m5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p3_m7, y0_m_3_7); bit_met_p3_m7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p1_p7, y0_p_1_7); bit_met_p1_p7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p1_p5, y0_p_1_5); bit_met_p1_p5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p1_p3, y0_p_1_3); bit_met_p1_p3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p1_p1, y0_p_1_1); bit_met_p1_p1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p1_m1, y0_m_1_1); bit_met_p1_m1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p1_m3, y0_m_1_3); bit_met_p1_m3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p1_m5, y0_m_1_5); bit_met_p1_m5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm1 = _mm_adds_epi16(psi_a_p1_m7, y0_m_1_7); bit_met_p1_m7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);

        xmm1 = _mm_subs_epi16(psi_a_m1_p7, y0_m_1_7); bit_met_m1_p7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m1_p5, y0_m_1_5); bit_met_m1_p5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m1_p3, y0_m_1_3); bit_met_m1_p3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m1_p1, y0_m_1_1); bit_met_m1_p1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m1_m1, y0_p_1_1); bit_met_m1_m1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m1_m3, y0_p_1_3); bit_met_m1_m3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m1_m5, y0_p_1_5); bit_met_m1_m5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m1_m7, y0_p_1_7); bit_met_m1_m7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m3_p7, y0_m_3_7); bit_met_m3_p7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m3_p5, y0_m_3_5); bit_met_m3_p5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m3_p3, y0_m_3_3); bit_met_m3_p3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m3_p1, y0_m_3_1); bit_met_m3_p1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m3_m1, y0_p_3_1); bit_met_m3_m1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m3_m3, y0_p_3_3); bit_met_m3_m3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m3_m5, y0_p_3_5); bit_met_m3_m5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m3_m7, y0_p_3_7); bit_met_m3_m7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m5_p7, y0_m_5_7); bit_met_m5_p7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m5_p5, y0_m_5_5); bit_met_m5_p5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m5_p3, y0_m_5_3); bit_met_m5_p3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m5_p1, y0_m_5_1); bit_met_m5_p1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m5_m1, y0_p_5_1); bit_met_m5_m1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m5_m3, y0_p_5_3); bit_met_m5_m3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m5_m5, y0_p_5_5); bit_met_m5_m5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m5_m7, y0_p_5_7); bit_met_m5_m7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m7_p7, y0_m_7_7); bit_met_m7_p7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m7_p5, y0_m_7_5); bit_met_m7_p5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m7_p3, y0_m_7_3); bit_met_m7_p3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m7_p1, y0_m_7_1); bit_met_m7_p1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m7_m1, y0_p_7_1); bit_met_m7_m1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m7_m3, y0_p_7_3); bit_met_m7_m3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m7_m5, y0_p_7_5); bit_met_m7_m5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm1 = _mm_subs_epi16(psi_a_m7_m7, y0_p_7_7); bit_met_m7_m7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);

        // Detection for 1st bit (LTE mapping)
        // bit = 1 
        xmm0 = _mm_max_epi16(bit_met_m7_p7, bit_met_m7_p5);
        xmm1 = _mm_max_epi16(bit_met_m7_p3, bit_met_m7_p1);
        xmm2 = _mm_max_epi16(bit_met_m7_m1, bit_met_m7_m3);
        xmm3 = _mm_max_epi16(bit_met_m7_m5, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);        
        xmm0 = _mm_max_epi16(bit_met_m5_p7, bit_met_m5_p5);
        xmm1 = _mm_max_epi16(bit_met_m5_p3, bit_met_m5_p1);
        xmm2 = _mm_max_epi16(bit_met_m5_m1, bit_met_m5_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m5_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m3_p7, bit_met_m3_p5);
        xmm1 = _mm_max_epi16(bit_met_m3_p3, bit_met_m3_p1);
        xmm2 = _mm_max_epi16(bit_met_m3_m1, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m3_m5, bit_met_m3_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_p7, bit_met_m1_p5);
        xmm1 = _mm_max_epi16(bit_met_m1_p3, bit_met_m1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m1_m3);
        xmm3 = _mm_max_epi16(bit_met_m1_m5, bit_met_m1_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        
        // bit = 0
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p7_p5);
        xmm1 = _mm_max_epi16(bit_met_p7_p3, bit_met_p7_p1);
        xmm2 = _mm_max_epi16(bit_met_p7_m1, bit_met_p7_m3);
        xmm3 = _mm_max_epi16(bit_met_p7_m5, bit_met_p7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_p7, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p5_p3, bit_met_p5_p1);
        xmm2 = _mm_max_epi16(bit_met_p5_m1, bit_met_p5_m3);
        xmm3 = _mm_max_epi16(bit_met_p5_m5, bit_met_p5_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_p7, bit_met_p3_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p3_p1);
        xmm2 = _mm_max_epi16(bit_met_p3_m1, bit_met_p3_m3);
        xmm3 = _mm_max_epi16(bit_met_p3_m5, bit_met_p3_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_p7, bit_met_p1_p5);
        xmm1 = _mm_max_epi16(bit_met_p1_p3, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_p1_m1, bit_met_p1_m3);
        xmm3 = _mm_max_epi16(bit_met_p1_m5, bit_met_p1_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        
        y0r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);
	 
        // Detection for 2nd bit (LTE mapping)
        // bit = 1
        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        
        // bit = 0
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m7_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m7_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y1r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);
     
        // Detection for 3rd bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_m7_m7, bit_met_m7_m5);
        xmm1 = _mm_max_epi16(bit_met_m7_m3, bit_met_m7_m1);
        xmm2 = _mm_max_epi16(bit_met_m7_p1, bit_met_m7_p3);
        xmm3 = _mm_max_epi16(bit_met_m7_p5, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m5_m7, bit_met_m5_m5);
        xmm1 = _mm_max_epi16(bit_met_m5_m3, bit_met_m5_m1);
        xmm2 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_m7, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p5_m3, bit_met_p5_m1);
        xmm2 = _mm_max_epi16(bit_met_p5_p1, bit_met_p5_p3);
        xmm3 = _mm_max_epi16(bit_met_p5_p5, bit_met_p5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p7_m5);
        xmm1 = _mm_max_epi16(bit_met_p7_m3, bit_met_p7_m1);
        xmm2 = _mm_max_epi16(bit_met_p7_p1, bit_met_p7_p3);
        xmm3 = _mm_max_epi16(bit_met_p7_p5, bit_met_p7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_m3_m7, bit_met_m3_m5);
        xmm1 = _mm_max_epi16(bit_met_m3_m3, bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_m3_p1, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_p5, bit_met_m3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_m7, bit_met_m1_m5);
        xmm1 = _mm_max_epi16(bit_met_m1_m3, bit_met_m1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m1_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p5, bit_met_m1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_m7, bit_met_p1_m5);
        xmm1 = _mm_max_epi16(bit_met_p1_m3, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1, bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_p5, bit_met_p1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_m7, bit_met_p3_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p3_m1);
        xmm2 = _mm_max_epi16(bit_met_p3_p1, bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_p5, bit_met_p3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y2r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // Detection for 4th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m7_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y0i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);


        // Detection for 5th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_m7_m7, bit_met_m7_m5);
        xmm1 = _mm_max_epi16(bit_met_m7_m3, bit_met_m7_m1);
        xmm2 = _mm_max_epi16(bit_met_m7_p1, bit_met_m7_p3);
        xmm3 = _mm_max_epi16(bit_met_m7_p5, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_m7, bit_met_m1_m5);
        xmm1 = _mm_max_epi16(bit_met_m1_m3, bit_met_m1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m1_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p5, bit_met_m1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_m7, bit_met_p1_m5);
        xmm1 = _mm_max_epi16(bit_met_p1_m3, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1, bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_p5, bit_met_p1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p7_m5);
        xmm1 = _mm_max_epi16(bit_met_p7_m3, bit_met_p7_m1);
        xmm2 = _mm_max_epi16(bit_met_p7_p1, bit_met_p7_p3);
        xmm3 = _mm_max_epi16(bit_met_p7_p5, bit_met_p7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_m5_m7, bit_met_m5_m5);
        xmm1 = _mm_max_epi16(bit_met_m5_m3, bit_met_m5_m1);
        xmm2 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m3_m7, bit_met_m3_m5);
        xmm1 = _mm_max_epi16(bit_met_m3_m3, bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_m3_p1, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_p5, bit_met_m3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_m7, bit_met_p3_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p3_m1);
        xmm2 = _mm_max_epi16(bit_met_p3_p1, bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_p5, bit_met_p3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_m7, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p5_m3, bit_met_p5_m1);
        xmm2 = _mm_max_epi16(bit_met_p5_p1, bit_met_p5_p3);
        xmm3 = _mm_max_epi16(bit_met_p5_p5, bit_met_p5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y1i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // Detection for 6th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m7_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y2i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // map to output stream, difficult to do in SIMD since we have 6 16bit LLRs
        // RE 1
        j = 24*i;
        stream0_out[j + 0] = ((short *)&y0r)[0];
        stream0_out[j + 1] = ((short *)&y1r)[0];
        stream0_out[j + 2] = ((short *)&y2r)[0];
        stream0_out[j + 3] = ((short *)&y0i)[0];
        stream0_out[j + 4] = ((short *)&y1i)[0];
        stream0_out[j + 5] = ((short *)&y2i)[0];
        // RE 2
        stream0_out[j + 6] = ((short *)&y0r)[1];
        stream0_out[j + 7] = ((short *)&y1r)[1];
        stream0_out[j + 8] = ((short *)&y2r)[1];
        stream0_out[j + 9] = ((short *)&y0i)[1];
        stream0_out[j + 10] = ((short *)&y1i)[1];
        stream0_out[j + 11] = ((short *)&y2i)[1];
        // RE 3
        stream0_out[j + 12] = ((short *)&y0r)[2];
        stream0_out[j + 13] = ((short *)&y1r)[2];
        stream0_out[j + 14] = ((short *)&y2r)[2];
        stream0_out[j + 15] = ((short *)&y0i)[2];
        stream0_out[j + 16] = ((short *)&y1i)[2];
        stream0_out[j + 17] = ((short *)&y2i)[2];
        // RE 4
        stream0_out[j + 18] = ((short *)&y0r)[3];
        stream0_out[j + 19] = ((short *)&y1r)[3];
        stream0_out[j + 20] = ((short *)&y2r)[3];
        stream0_out[j + 21] = ((short *)&y0i)[3];
        stream0_out[j + 22] = ((short *)&y1i)[3];
        stream0_out[j + 23] = ((short *)&y2i)[3];       
        // RE 5
        stream0_out[j + 24] = ((short *)&y0r)[4];
        stream0_out[j + 25] = ((short *)&y1r)[4];
        stream0_out[j + 26] = ((short *)&y2r)[4];
        stream0_out[j + 27] = ((short *)&y0i)[4];
        stream0_out[j + 28] = ((short *)&y1i)[4];
        stream0_out[j + 29] = ((short *)&y2i)[4];       
        // RE 6
        stream0_out[j + 30] = ((short *)&y0r)[5];
        stream0_out[j + 31] = ((short *)&y1r)[5];
        stream0_out[j + 32] = ((short *)&y2r)[5];
        stream0_out[j + 33] = ((short *)&y0i)[5];
        stream0_out[j + 34] = ((short *)&y1i)[5];
        stream0_out[j + 35] = ((short *)&y2i)[5];       
        // RE 7
        stream0_out[j + 36] = ((short *)&y0r)[6];
        stream0_out[j + 37] = ((short *)&y1r)[6];
        stream0_out[j + 38] = ((short *)&y2r)[6];
        stream0_out[j + 39] = ((short *)&y0i)[6];
        stream0_out[j + 40] = ((short *)&y1i)[6];
        stream0_out[j + 41] = ((short *)&y2i)[6];       
        // RE 8
        stream0_out[j + 42] = ((short *)&y0r)[7];
        stream0_out[j + 43] = ((short *)&y1r)[7];
        stream0_out[j + 44] = ((short *)&y2r)[7];
        stream0_out[j + 45] = ((short *)&y0i)[7];
        stream0_out[j + 46] = ((short *)&y1i)[7];
        stream0_out[j + 47] = ((short *)&y2i)[7];       
  }
  _mm_empty();
  _m_empty();

}


int dlsch_64qam_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
                         int **rxdataF_comp,
                         int **rxdataF_comp_i,
                         int **dl_ch_mag,
                         int **rho_i,
                         short *dlsch_llr,
                         unsigned char symbol,
                         unsigned char first_symbol_flag,
                         unsigned short nb_rb,
                         uint16_t pbch_pss_sss_adjust,
                         short **llr16p) {

    int16_t *rxF      = (int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i    = (int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag   = (int16_t*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho      = (int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;

    //first symbol has different structure due to more pilots
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
  
    if (!llr16) {
        msg("dlsch_64qam_64qam_llr: llr is null, symbol %d\n",symbol);
        return(-1);
    }

    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }

    qam64_qpsk((short *)rxF,
               (short *)rxF_i,
               (short *)ch_mag,
               (short *)llr16,
               (short *)rho,
               len);

    llr16 += (6*len);
    *llr16p = (short *)llr16;
    return(0);
}



void qam64_qam16(short *stream0_in,
                 short *stream1_in,
                 short *ch_mag,
                 short *ch_mag_i,
                 short *stream0_out,
                 short *rho01,
                 int length
                 ) {

/* 
   Author: S. Wagner
   Date: 31-07-12

   Input: 
   stream0_in:  MF filter for 1st stream, i.e., y0=h0'*y
   stream1_in:  MF filter for 2nd stream, i.e., y1=h1'*y
   ch_mag:      4*h0/sqrt(42), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   ch_mag_i:    4*h1/sqrt(42), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   rho01:       Channel cross correlation, i.e., h1'*h0
   
   Output:
   stream0_out: output LLRs for 1st stream
*/

    __m128i *rho01_128i      = (__m128i *)rho01;
    __m128i *stream0_128i_in = (__m128i *)stream0_in;
    __m128i *stream1_128i_in = (__m128i *)stream1_in;
    __m128i *ch_mag_128i     = (__m128i *)ch_mag;
    __m128i *ch_mag_128i_i   = (__m128i *)ch_mag_i;

    int i,j;

    ONE_OVER_SQRT_42 = _mm_set1_epi16(10112); // round(1/sqrt(42)*2^16)
    THREE_OVER_SQRT_42 = _mm_set1_epi16(30337); // round(3/sqrt(42)*2^16)
    FIVE_OVER_SQRT_42 = _mm_set1_epi16(25281); // round(5/sqrt(42)*2^15)
    SEVEN_OVER_SQRT_42 = _mm_set1_epi16(17697); // round(5/sqrt(42)*2^15)
    ONE_OVER_SQRT_2 = _mm_set1_epi16(23170); // round(1/sqrt(2)*2^15)
    ONE_OVER_SQRT_2_42 = _mm_set1_epi16(3575); // round(1/sqrt(2*42)*2^15)
    THREE_OVER_SQRT_2_42 = _mm_set1_epi16(10726); // round(3/sqrt(2*42)*2^15)
    FIVE_OVER_SQRT_2_42 = _mm_set1_epi16(17876); // round(5/sqrt(2*42)*2^15)
    SEVEN_OVER_SQRT_2_42 = _mm_set1_epi16(25027); // round(7/sqrt(2*42)*2^15)
    FORTYNINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(30969); // round(49/(4*sqrt(42))*2^14), Q2.14
    THIRTYSEVEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(23385); // round(37/(4*sqrt(42))*2^14), Q2.14
    TWENTYFIVE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(31601); // round(25/(4*sqrt(42))*2^15)
    TWENTYNINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(18329); // round(29/(4*sqrt(42))*2^15), Q2.14
    SEVENTEEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(21489); // round(17/(4*sqrt(42))*2^15)
    NINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(11376); // round(9/(4*sqrt(42))*2^15)
    THIRTEEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(16433); // round(13/(4*sqrt(42))*2^15)
    FIVE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(6320); // round(5/(4*sqrt(42))*2^15)
    ONE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(1264); // round(1/(4*sqrt(42))*2^15)
    SQRT_42_OVER_FOUR = _mm_set1_epi16(13272); // round(sqrt(42)/4*2^13), Q3.12
    ONE_OVER_SQRT_10_Q15 = _mm_set1_epi16(10362); // round(1/sqrt(10)*2^15)
    THREE_OVER_SQRT_10 = _mm_set1_epi16(31086); // round(3/sqrt(10)*2^15)
    SQRT_10_OVER_FOUR = _mm_set1_epi16(25905); // round(sqrt(10)/4*2^15)
    //    ONE_OVER_SQRT_10_Q15 = _mm_set1_epi16(7327); // round(1/sqrt(10)*2^15)
    //    THREE_OVER_SQRT_10 = _mm_set1_epi16(21981); // round(3/sqrt(10)*2^15)
    //    SQRT_10_OVER_FOUR = _mm_set1_epi16(25905); // round(sqrt(10)/4*2^15) Q3.13


    for (i=0; i<length>>2; i+=2) {
          
        // Get rho
        xmm0 = rho01_128i[i];
        xmm1 = rho01_128i[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1 = _mm_mulhi_epi16(rho_rpi, ONE_OVER_SQRT_42);   
        rho_rmi_1_1 = _mm_mulhi_epi16(rho_rmi, ONE_OVER_SQRT_42);
        rho_rpi_3_3 = _mm_mulhi_epi16(rho_rpi, THREE_OVER_SQRT_42); 
        rho_rmi_3_3 = _mm_mulhi_epi16(rho_rmi, THREE_OVER_SQRT_42);
        rho_rpi_5_5 = _mm_mulhi_epi16(rho_rpi, FIVE_OVER_SQRT_42);  
        rho_rmi_5_5 = _mm_mulhi_epi16(rho_rmi, FIVE_OVER_SQRT_42); 
        rho_rpi_7_7 = _mm_mulhi_epi16(rho_rpi, SEVEN_OVER_SQRT_42); 
        rho_rmi_7_7 = _mm_mulhi_epi16(rho_rmi, SEVEN_OVER_SQRT_42);

        rho_rpi_5_5 = _mm_slli_epi16(rho_rpi_5_5, 1); 
        rho_rmi_5_5 = _mm_slli_epi16(rho_rmi_5_5, 1);
        rho_rpi_7_7 = _mm_slli_epi16(rho_rpi_7_7, 2); 
        rho_rmi_7_7 = _mm_slli_epi16(rho_rmi_7_7, 2);

        xmm4 = _mm_mulhi_epi16(xmm2, ONE_OVER_SQRT_42);   
        xmm5 = _mm_mulhi_epi16(xmm3, ONE_OVER_SQRT_42);   
        xmm6 = _mm_mulhi_epi16(xmm3, THREE_OVER_SQRT_42); 
        xmm7 = _mm_mulhi_epi16(xmm3, FIVE_OVER_SQRT_42);  
        xmm8 = _mm_mulhi_epi16(xmm3, SEVEN_OVER_SQRT_42); 
        xmm7 = _mm_slli_epi16(xmm7, 1);
        xmm8 = _mm_slli_epi16(xmm8, 2);

        rho_rpi_1_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_1_3 = _mm_subs_epi16(xmm4, xmm6); 
        rho_rpi_1_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_1_5 = _mm_subs_epi16(xmm4, xmm7);
        rho_rpi_1_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_1_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, THREE_OVER_SQRT_42); 
        rho_rpi_3_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_3_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_3_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_3_5 = _mm_subs_epi16(xmm4, xmm7);
        rho_rpi_3_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_3_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, FIVE_OVER_SQRT_42); 
        xmm4 = _mm_slli_epi16(xmm4, 1);
        rho_rpi_5_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_5_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_5_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_5_3 = _mm_subs_epi16(xmm4, xmm6);
        rho_rpi_5_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_5_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, SEVEN_OVER_SQRT_42); 
        xmm4 = _mm_slli_epi16(xmm4, 2);
        rho_rpi_7_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_7_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_7_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_7_3 = _mm_subs_epi16(xmm4, xmm6);
        rho_rpi_7_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_7_5 = _mm_subs_epi16(xmm4, xmm7);

        // Rearrange interfering MF output
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]

        // Psi_r calculation from rho_rpi or rho_rmi
        xmm0 = _mm_xor_si128(xmm0, xmm0); // ZERO for abs_pi16
        xmm2 = _mm_subs_epi16(rho_rpi_7_7, y1r); psi_r_p7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_5, y1r); psi_r_p7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_3, y1r); psi_r_p7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_1, y1r); psi_r_p7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_1, y1r); psi_r_p7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_3, y1r); psi_r_p7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_5, y1r); psi_r_p7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_7, y1r); psi_r_p7_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_7, y1r); psi_r_p5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_5, y1r); psi_r_p5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_3, y1r); psi_r_p5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_1, y1r); psi_r_p5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_1, y1r); psi_r_p5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_3, y1r); psi_r_p5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_5, y1r); psi_r_p5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_7, y1r); psi_r_p5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_7, y1r); psi_r_p3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_5, y1r); psi_r_p3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_3, y1r); psi_r_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_1, y1r); psi_r_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_1, y1r); psi_r_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_3, y1r); psi_r_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_5, y1r); psi_r_p3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_7, y1r); psi_r_p3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_7, y1r); psi_r_p1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_5, y1r); psi_r_p1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_3, y1r); psi_r_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_1, y1r); psi_r_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_1, y1r); psi_r_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_3, y1r); psi_r_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_5, y1r); psi_r_p1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_7, y1r); psi_r_p1_m7 = _mm_abs_epi16(xmm2);
                                                            
        xmm2 = _mm_adds_epi16(rho_rmi_1_7, y1r); psi_r_m1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_5, y1r); psi_r_m1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_3, y1r); psi_r_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_1, y1r); psi_r_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_1, y1r); psi_r_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_3, y1r); psi_r_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_5, y1r); psi_r_m1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_7, y1r); psi_r_m1_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_7, y1r); psi_r_m3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_5, y1r); psi_r_m3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_3, y1r); psi_r_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_1, y1r); psi_r_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_1, y1r); psi_r_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_3, y1r); psi_r_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_5, y1r); psi_r_m3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_7, y1r); psi_r_m3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_7, y1r); psi_r_m5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_5, y1r); psi_r_m5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_3, y1r); psi_r_m5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_1, y1r); psi_r_m5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_1, y1r); psi_r_m5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_3, y1r); psi_r_m5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_5, y1r); psi_r_m5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_7, y1r); psi_r_m5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_7, y1r); psi_r_m7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_5, y1r); psi_r_m7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_3, y1r); psi_r_m7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_1, y1r); psi_r_m7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_1, y1r); psi_r_m7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_3, y1r); psi_r_m7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_5, y1r); psi_r_m7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_7, y1r); psi_r_m7_m7 = _mm_abs_epi16(xmm2);

        // Psi_i calculation from rho_rpi or rho_rmi
        xmm2 = _mm_subs_epi16(rho_rmi_7_7, y1i); psi_i_p7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_7, y1i); psi_i_p7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_7, y1i); psi_i_p7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_7, y1i); psi_i_p7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_7, y1i); psi_i_p7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_7, y1i); psi_i_p7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_7, y1i); psi_i_p7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_7, y1i); psi_i_p7_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_5, y1i); psi_i_p5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_5, y1i); psi_i_p5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_5, y1i); psi_i_p5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_5, y1i); psi_i_p5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_5, y1i); psi_i_p5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_5, y1i); psi_i_p5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_5, y1i); psi_i_p5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_5, y1i); psi_i_p5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_3, y1i); psi_i_p3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_3, y1i); psi_i_p3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_3, y1i); psi_i_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_3, y1i); psi_i_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_3, y1i); psi_i_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_3, y1i); psi_i_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_3, y1i); psi_i_p3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_3, y1i); psi_i_p3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_1, y1i); psi_i_p1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_1, y1i); psi_i_p1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_1, y1i); psi_i_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_1, y1i); psi_i_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_1, y1i); psi_i_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_1, y1i); psi_i_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_1, y1i); psi_i_p1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_1, y1i); psi_i_p1_m7 = _mm_abs_epi16(xmm2);

        xmm2 = _mm_subs_epi16(rho_rpi_7_1, y1i); psi_i_m1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_1, y1i); psi_i_m1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_1, y1i); psi_i_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_1, y1i); psi_i_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_1, y1i); psi_i_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_1, y1i); psi_i_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_1, y1i); psi_i_m1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_1, y1i); psi_i_m1_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_3, y1i); psi_i_m3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_3, y1i); psi_i_m3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_3, y1i); psi_i_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_3, y1i); psi_i_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_3, y1i); psi_i_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_3, y1i); psi_i_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_3, y1i); psi_i_m3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_3, y1i); psi_i_m3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_5, y1i); psi_i_m5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_5, y1i); psi_i_m5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_5, y1i); psi_i_m5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_5, y1i); psi_i_m5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_5, y1i); psi_i_m5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_5, y1i); psi_i_m5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_5, y1i); psi_i_m5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_5, y1i); psi_i_m5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_7, y1i); psi_i_m7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_7, y1i); psi_i_m7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_7, y1i); psi_i_m7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_7, y1i); psi_i_m7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_7, y1i); psi_i_m7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_7, y1i); psi_i_m7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_7, y1i); psi_i_m7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_7, y1i); psi_i_m7_m7 = _mm_abs_epi16(xmm2);


        // Rearrange desired MF output
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 
    
        // Rearrange desired channel magnitudes
        xmm2 = ch_mag_128i[i]; // = [|h|^2(1),|h|^2(1),|h|^2(2),|h|^2(2)]*(2/sqrt(10))
        xmm3 = ch_mag_128i[i+1]; // = [|h|^2(3),|h|^2(3),|h|^2(4),|h|^2(4)]*(2/sqrt(10))
        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_des = _mm_unpacklo_epi64(xmm2,xmm3); 

        // Rearrange interfering channel magnitudes
        xmm2 = ch_mag_128i_i[i];   
        xmm3 = ch_mag_128i_i[i+1]; 
        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_int  = _mm_unpacklo_epi64(xmm2,xmm3); 

        y0r_one_over_sqrt_21   = _mm_mulhi_epi16(y0r, ONE_OVER_SQRT_42);   
        y0r_three_over_sqrt_21 = _mm_mulhi_epi16(y0r, THREE_OVER_SQRT_42); 
        y0r_five_over_sqrt_21  = _mm_mulhi_epi16(y0r, FIVE_OVER_SQRT_42);  
        y0r_five_over_sqrt_21  = _mm_slli_epi16(y0r_five_over_sqrt_21, 1);
        y0r_seven_over_sqrt_21 = _mm_mulhi_epi16(y0r, SEVEN_OVER_SQRT_42); 
        y0r_seven_over_sqrt_21 = _mm_slli_epi16(y0r_seven_over_sqrt_21, 2); // Q2.14

        y0i_one_over_sqrt_21   = _mm_mulhi_epi16(y0i, ONE_OVER_SQRT_42);   
        y0i_three_over_sqrt_21 = _mm_mulhi_epi16(y0i, THREE_OVER_SQRT_42); 
        y0i_five_over_sqrt_21  = _mm_mulhi_epi16(y0i, FIVE_OVER_SQRT_42);  
        y0i_five_over_sqrt_21  = _mm_slli_epi16(y0i_five_over_sqrt_21, 1);
        y0i_seven_over_sqrt_21 = _mm_mulhi_epi16(y0i, SEVEN_OVER_SQRT_42); 
        y0i_seven_over_sqrt_21 = _mm_slli_epi16(y0i_seven_over_sqrt_21, 2); // Q2.14

        y0_p_7_1 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_7_3 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_7_5 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_7_7 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_5_1 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_5_3 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_5_5 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_5_7 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_3_1 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_3_3 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_3_5 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_3_7 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_1_1 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_1_3 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_1_5 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_1_7 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_seven_over_sqrt_21);

        y0_m_1_1 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_1_3 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_1_5 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_1_7 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_3_1 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_3_3 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_3_5 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_3_7 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_5_1 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_5_3 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_5_5 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_5_7 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_7_1 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_7_3 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_7_5 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_7_7 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_seven_over_sqrt_21);

        interference_abs_epi16(psi_r_p7_p7, ch_mag_int, a_r_p7_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p7_p5, ch_mag_int, a_r_p7_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p7_p3, ch_mag_int, a_r_p7_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p7_p1, ch_mag_int, a_r_p7_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p7_m1, ch_mag_int, a_r_p7_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p7_m3, ch_mag_int, a_r_p7_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p7_m5, ch_mag_int, a_r_p7_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p7_m7, ch_mag_int, a_r_p7_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p5_p7, ch_mag_int, a_r_p5_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p5_p5, ch_mag_int, a_r_p5_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p5_p3, ch_mag_int, a_r_p5_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p5_p1, ch_mag_int, a_r_p5_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p5_m1, ch_mag_int, a_r_p5_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p5_m3, ch_mag_int, a_r_p5_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p5_m5, ch_mag_int, a_r_p5_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p5_m7, ch_mag_int, a_r_p5_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_p7, ch_mag_int, a_r_p3_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_p5, ch_mag_int, a_r_p3_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_p3, ch_mag_int, a_r_p3_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_p1, ch_mag_int, a_r_p3_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_m1, ch_mag_int, a_r_p3_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_m3, ch_mag_int, a_r_p3_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_m5, ch_mag_int, a_r_p3_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p3_m7, ch_mag_int, a_r_p3_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_p7, ch_mag_int, a_r_p1_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_p5, ch_mag_int, a_r_p1_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_p3, ch_mag_int, a_r_p1_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_p1, ch_mag_int, a_r_p1_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_m1, ch_mag_int, a_r_p1_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_m3, ch_mag_int, a_r_p1_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_m5, ch_mag_int, a_r_p1_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_p1_m7, ch_mag_int, a_r_p1_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_p7, ch_mag_int, a_r_m1_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_p5, ch_mag_int, a_r_m1_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_p3, ch_mag_int, a_r_m1_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_p1, ch_mag_int, a_r_m1_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_m1, ch_mag_int, a_r_m1_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_m3, ch_mag_int, a_r_m1_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_m5, ch_mag_int, a_r_m1_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m1_m7, ch_mag_int, a_r_m1_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_p7, ch_mag_int, a_r_m3_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_p5, ch_mag_int, a_r_m3_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_p3, ch_mag_int, a_r_m3_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_p1, ch_mag_int, a_r_m3_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_m1, ch_mag_int, a_r_m3_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_m3, ch_mag_int, a_r_m3_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_m5, ch_mag_int, a_r_m3_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m3_m7, ch_mag_int, a_r_m3_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m5_p7, ch_mag_int, a_r_m5_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m5_p5, ch_mag_int, a_r_m5_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m5_p3, ch_mag_int, a_r_m5_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m5_p1, ch_mag_int, a_r_m5_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m5_m1, ch_mag_int, a_r_m5_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m5_m3, ch_mag_int, a_r_m5_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m5_m5, ch_mag_int, a_r_m5_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m5_m7, ch_mag_int, a_r_m5_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m7_p7, ch_mag_int, a_r_m7_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m7_p5, ch_mag_int, a_r_m7_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m7_p3, ch_mag_int, a_r_m7_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m7_p1, ch_mag_int, a_r_m7_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m7_m1, ch_mag_int, a_r_m7_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m7_m3, ch_mag_int, a_r_m7_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m7_m5, ch_mag_int, a_r_m7_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_r_m7_m7, ch_mag_int, a_r_m7_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);

        interference_abs_epi16(psi_i_p7_p7, ch_mag_int, a_i_p7_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p7_p5, ch_mag_int, a_i_p7_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p7_p3, ch_mag_int, a_i_p7_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p7_p1, ch_mag_int, a_i_p7_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p7_m1, ch_mag_int, a_i_p7_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p7_m3, ch_mag_int, a_i_p7_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p7_m5, ch_mag_int, a_i_p7_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p7_m7, ch_mag_int, a_i_p7_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p5_p7, ch_mag_int, a_i_p5_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p5_p5, ch_mag_int, a_i_p5_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p5_p3, ch_mag_int, a_i_p5_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p5_p1, ch_mag_int, a_i_p5_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p5_m1, ch_mag_int, a_i_p5_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p5_m3, ch_mag_int, a_i_p5_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p5_m5, ch_mag_int, a_i_p5_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p5_m7, ch_mag_int, a_i_p5_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_p7, ch_mag_int, a_i_p3_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_p5, ch_mag_int, a_i_p3_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_p3, ch_mag_int, a_i_p3_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_p1, ch_mag_int, a_i_p3_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_m1, ch_mag_int, a_i_p3_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_m3, ch_mag_int, a_i_p3_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_m5, ch_mag_int, a_i_p3_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p3_m7, ch_mag_int, a_i_p3_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_p7, ch_mag_int, a_i_p1_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_p5, ch_mag_int, a_i_p1_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_p3, ch_mag_int, a_i_p1_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_p1, ch_mag_int, a_i_p1_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_m1, ch_mag_int, a_i_p1_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_m3, ch_mag_int, a_i_p1_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_m5, ch_mag_int, a_i_p1_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_p1_m7, ch_mag_int, a_i_p1_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_p7, ch_mag_int, a_i_m1_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_p5, ch_mag_int, a_i_m1_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_p3, ch_mag_int, a_i_m1_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_p1, ch_mag_int, a_i_m1_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_m1, ch_mag_int, a_i_m1_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_m3, ch_mag_int, a_i_m1_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_m5, ch_mag_int, a_i_m1_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m1_m7, ch_mag_int, a_i_m1_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_p7, ch_mag_int, a_i_m3_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_p5, ch_mag_int, a_i_m3_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_p3, ch_mag_int, a_i_m3_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_p1, ch_mag_int, a_i_m3_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_m1, ch_mag_int, a_i_m3_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_m3, ch_mag_int, a_i_m3_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_m5, ch_mag_int, a_i_m3_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m3_m7, ch_mag_int, a_i_m3_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m5_p7, ch_mag_int, a_i_m5_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m5_p5, ch_mag_int, a_i_m5_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m5_p3, ch_mag_int, a_i_m5_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m5_p1, ch_mag_int, a_i_m5_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m5_m1, ch_mag_int, a_i_m5_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m5_m3, ch_mag_int, a_i_m5_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m5_m5, ch_mag_int, a_i_m5_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m5_m7, ch_mag_int, a_i_m5_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m7_p7, ch_mag_int, a_i_m7_p7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m7_p5, ch_mag_int, a_i_m7_p5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m7_p3, ch_mag_int, a_i_m7_p3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m7_p1, ch_mag_int, a_i_m7_p1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m7_m1, ch_mag_int, a_i_m7_m1, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m7_m3, ch_mag_int, a_i_m7_m3, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m7_m5, ch_mag_int, a_i_m7_m5, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);
        interference_abs_epi16(psi_i_m7_m7, ch_mag_int, a_i_m7_m7, ONE_OVER_SQRT_10_Q15, THREE_OVER_SQRT_10);

        // Calculation of a group of two terms in the bit metric involving product of psi and interference     
        prodsum_psi_a_epi16(psi_r_p7_p7, a_r_p7_p7, psi_i_p7_p7, a_i_p7_p7, psi_a_p7_p7);
        prodsum_psi_a_epi16(psi_r_p7_p5, a_r_p7_p5, psi_i_p7_p5, a_i_p7_p5, psi_a_p7_p5); 
        prodsum_psi_a_epi16(psi_r_p7_p3, a_r_p7_p3, psi_i_p7_p3, a_i_p7_p3, psi_a_p7_p3); 
        prodsum_psi_a_epi16(psi_r_p7_p1, a_r_p7_p1, psi_i_p7_p1, a_i_p7_p1, psi_a_p7_p1); 
        prodsum_psi_a_epi16(psi_r_p7_m1, a_r_p7_m1, psi_i_p7_m1, a_i_p7_m1, psi_a_p7_m1); 
        prodsum_psi_a_epi16(psi_r_p7_m3, a_r_p7_m3, psi_i_p7_m3, a_i_p7_m3, psi_a_p7_m3); 
        prodsum_psi_a_epi16(psi_r_p7_m5, a_r_p7_m5, psi_i_p7_m5, a_i_p7_m5, psi_a_p7_m5); 
        prodsum_psi_a_epi16(psi_r_p7_m7, a_r_p7_m7, psi_i_p7_m7, a_i_p7_m7, psi_a_p7_m7); 
        prodsum_psi_a_epi16(psi_r_p5_p7, a_r_p5_p7, psi_i_p5_p7, a_i_p5_p7, psi_a_p5_p7); 
        prodsum_psi_a_epi16(psi_r_p5_p5, a_r_p5_p5, psi_i_p5_p5, a_i_p5_p5, psi_a_p5_p5); 
        prodsum_psi_a_epi16(psi_r_p5_p3, a_r_p5_p3, psi_i_p5_p3, a_i_p5_p3, psi_a_p5_p3); 
        prodsum_psi_a_epi16(psi_r_p5_p1, a_r_p5_p1, psi_i_p5_p1, a_i_p5_p1, psi_a_p5_p1); 
        prodsum_psi_a_epi16(psi_r_p5_m1, a_r_p5_m1, psi_i_p5_m1, a_i_p5_m1, psi_a_p5_m1); 
        prodsum_psi_a_epi16(psi_r_p5_m3, a_r_p5_m3, psi_i_p5_m3, a_i_p5_m3, psi_a_p5_m3); 
        prodsum_psi_a_epi16(psi_r_p5_m5, a_r_p5_m5, psi_i_p5_m5, a_i_p5_m5, psi_a_p5_m5); 
        prodsum_psi_a_epi16(psi_r_p5_m7, a_r_p5_m7, psi_i_p5_m7, a_i_p5_m7, psi_a_p5_m7); 
        prodsum_psi_a_epi16(psi_r_p3_p7, a_r_p3_p7, psi_i_p3_p7, a_i_p3_p7, psi_a_p3_p7);
        prodsum_psi_a_epi16(psi_r_p3_p5, a_r_p3_p5, psi_i_p3_p5, a_i_p3_p5, psi_a_p3_p5);
        prodsum_psi_a_epi16(psi_r_p3_p3, a_r_p3_p3, psi_i_p3_p3, a_i_p3_p3, psi_a_p3_p3);
        prodsum_psi_a_epi16(psi_r_p3_p1, a_r_p3_p1, psi_i_p3_p1, a_i_p3_p1, psi_a_p3_p1);
        prodsum_psi_a_epi16(psi_r_p3_m1, a_r_p3_m1, psi_i_p3_m1, a_i_p3_m1, psi_a_p3_m1);
        prodsum_psi_a_epi16(psi_r_p3_m3, a_r_p3_m3, psi_i_p3_m3, a_i_p3_m3, psi_a_p3_m3);
        prodsum_psi_a_epi16(psi_r_p3_m5, a_r_p3_m5, psi_i_p3_m5, a_i_p3_m5, psi_a_p3_m5);
        prodsum_psi_a_epi16(psi_r_p3_m7, a_r_p3_m7, psi_i_p3_m7, a_i_p3_m7, psi_a_p3_m7);
        prodsum_psi_a_epi16(psi_r_p1_p7, a_r_p1_p7, psi_i_p1_p7, a_i_p1_p7, psi_a_p1_p7);
        prodsum_psi_a_epi16(psi_r_p1_p5, a_r_p1_p5, psi_i_p1_p5, a_i_p1_p5, psi_a_p1_p5);
        prodsum_psi_a_epi16(psi_r_p1_p3, a_r_p1_p3, psi_i_p1_p3, a_i_p1_p3, psi_a_p1_p3);
        prodsum_psi_a_epi16(psi_r_p1_p1, a_r_p1_p1, psi_i_p1_p1, a_i_p1_p1, psi_a_p1_p1);
        prodsum_psi_a_epi16(psi_r_p1_m1, a_r_p1_m1, psi_i_p1_m1, a_i_p1_m1, psi_a_p1_m1);
        prodsum_psi_a_epi16(psi_r_p1_m3, a_r_p1_m3, psi_i_p1_m3, a_i_p1_m3, psi_a_p1_m3);
        prodsum_psi_a_epi16(psi_r_p1_m5, a_r_p1_m5, psi_i_p1_m5, a_i_p1_m5, psi_a_p1_m5);
        prodsum_psi_a_epi16(psi_r_p1_m7, a_r_p1_m7, psi_i_p1_m7, a_i_p1_m7, psi_a_p1_m7);
        prodsum_psi_a_epi16(psi_r_m1_p7, a_r_m1_p7, psi_i_m1_p7, a_i_m1_p7, psi_a_m1_p7);
        prodsum_psi_a_epi16(psi_r_m1_p5, a_r_m1_p5, psi_i_m1_p5, a_i_m1_p5, psi_a_m1_p5);
        prodsum_psi_a_epi16(psi_r_m1_p3, a_r_m1_p3, psi_i_m1_p3, a_i_m1_p3, psi_a_m1_p3);
        prodsum_psi_a_epi16(psi_r_m1_p1, a_r_m1_p1, psi_i_m1_p1, a_i_m1_p1, psi_a_m1_p1);
        prodsum_psi_a_epi16(psi_r_m1_m1, a_r_m1_m1, psi_i_m1_m1, a_i_m1_m1, psi_a_m1_m1);
        prodsum_psi_a_epi16(psi_r_m1_m3, a_r_m1_m3, psi_i_m1_m3, a_i_m1_m3, psi_a_m1_m3);
        prodsum_psi_a_epi16(psi_r_m1_m5, a_r_m1_m5, psi_i_m1_m5, a_i_m1_m5, psi_a_m1_m5);
        prodsum_psi_a_epi16(psi_r_m1_m7, a_r_m1_m7, psi_i_m1_m7, a_i_m1_m7, psi_a_m1_m7);
        prodsum_psi_a_epi16(psi_r_m3_p7, a_r_m3_p7, psi_i_m3_p7, a_i_m3_p7, psi_a_m3_p7);
        prodsum_psi_a_epi16(psi_r_m3_p5, a_r_m3_p5, psi_i_m3_p5, a_i_m3_p5, psi_a_m3_p5);
        prodsum_psi_a_epi16(psi_r_m3_p3, a_r_m3_p3, psi_i_m3_p3, a_i_m3_p3, psi_a_m3_p3);
        prodsum_psi_a_epi16(psi_r_m3_p1, a_r_m3_p1, psi_i_m3_p1, a_i_m3_p1, psi_a_m3_p1);
        prodsum_psi_a_epi16(psi_r_m3_m1, a_r_m3_m1, psi_i_m3_m1, a_i_m3_m1, psi_a_m3_m1);
        prodsum_psi_a_epi16(psi_r_m3_m3, a_r_m3_m3, psi_i_m3_m3, a_i_m3_m3, psi_a_m3_m3);
        prodsum_psi_a_epi16(psi_r_m3_m5, a_r_m3_m5, psi_i_m3_m5, a_i_m3_m5, psi_a_m3_m5);
        prodsum_psi_a_epi16(psi_r_m3_m7, a_r_m3_m7, psi_i_m3_m7, a_i_m3_m7, psi_a_m3_m7);
        prodsum_psi_a_epi16(psi_r_m5_p7, a_r_m5_p7, psi_i_m5_p7, a_i_m5_p7, psi_a_m5_p7);
        prodsum_psi_a_epi16(psi_r_m5_p5, a_r_m5_p5, psi_i_m5_p5, a_i_m5_p5, psi_a_m5_p5);
        prodsum_psi_a_epi16(psi_r_m5_p3, a_r_m5_p3, psi_i_m5_p3, a_i_m5_p3, psi_a_m5_p3);
        prodsum_psi_a_epi16(psi_r_m5_p1, a_r_m5_p1, psi_i_m5_p1, a_i_m5_p1, psi_a_m5_p1);
        prodsum_psi_a_epi16(psi_r_m5_m1, a_r_m5_m1, psi_i_m5_m1, a_i_m5_m1, psi_a_m5_m1); 
        prodsum_psi_a_epi16(psi_r_m5_m3, a_r_m5_m3, psi_i_m5_m3, a_i_m5_m3, psi_a_m5_m3);
        prodsum_psi_a_epi16(psi_r_m5_m5, a_r_m5_m5, psi_i_m5_m5, a_i_m5_m5, psi_a_m5_m5);
        prodsum_psi_a_epi16(psi_r_m5_m7, a_r_m5_m7, psi_i_m5_m7, a_i_m5_m7, psi_a_m5_m7);
        prodsum_psi_a_epi16(psi_r_m7_p7, a_r_m7_p7, psi_i_m7_p7, a_i_m7_p7, psi_a_m7_p7);
        prodsum_psi_a_epi16(psi_r_m7_p5, a_r_m7_p5, psi_i_m7_p5, a_i_m7_p5, psi_a_m7_p5);
        prodsum_psi_a_epi16(psi_r_m7_p3, a_r_m7_p3, psi_i_m7_p3, a_i_m7_p3, psi_a_m7_p3); 
        prodsum_psi_a_epi16(psi_r_m7_p1, a_r_m7_p1, psi_i_m7_p1, a_i_m7_p1, psi_a_m7_p1);
        prodsum_psi_a_epi16(psi_r_m7_m1, a_r_m7_m1, psi_i_m7_m1, a_i_m7_m1, psi_a_m7_m1); 
        prodsum_psi_a_epi16(psi_r_m7_m3, a_r_m7_m3, psi_i_m7_m3, a_i_m7_m3, psi_a_m7_m3);
        prodsum_psi_a_epi16(psi_r_m7_m5, a_r_m7_m5, psi_i_m7_m5, a_i_m7_m5, psi_a_m7_m5);
        prodsum_psi_a_epi16(psi_r_m7_m7, a_r_m7_m7, psi_i_m7_m7, a_i_m7_m7, psi_a_m7_m7);

        // Calculation of a group of two terms in the bit metric involving squares of interference
        square_a_epi16(a_r_p7_p7, a_i_p7_p7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p7_p7);
        square_a_epi16(a_r_p7_p5, a_i_p7_p5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p7_p5);
        square_a_epi16(a_r_p7_p3, a_i_p7_p3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p7_p3);
        square_a_epi16(a_r_p7_p1, a_i_p7_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p7_p1);
        square_a_epi16(a_r_p7_m1, a_i_p7_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p7_m1);
        square_a_epi16(a_r_p7_m3, a_i_p7_m3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p7_m3);
        square_a_epi16(a_r_p7_m5, a_i_p7_m5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p7_m5);
        square_a_epi16(a_r_p7_m7, a_i_p7_m7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p7_m7);
        square_a_epi16(a_r_p5_p7, a_i_p5_p7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p5_p7);
        square_a_epi16(a_r_p5_p5, a_i_p5_p5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p5_p5);
        square_a_epi16(a_r_p5_p3, a_i_p5_p3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p5_p3);
        square_a_epi16(a_r_p5_p1, a_i_p5_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p5_p1);
        square_a_epi16(a_r_p5_m1, a_i_p5_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p5_m1);
        square_a_epi16(a_r_p5_m3, a_i_p5_m3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p5_m3);
        square_a_epi16(a_r_p5_m5, a_i_p5_m5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p5_m5);
        square_a_epi16(a_r_p5_m7, a_i_p5_m7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p5_m7);
        square_a_epi16(a_r_p3_p7, a_i_p3_p7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p3_p7);
        square_a_epi16(a_r_p3_p5, a_i_p3_p5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p3_p5);
        square_a_epi16(a_r_p3_p3, a_i_p3_p3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p3_p3);
        square_a_epi16(a_r_p3_p1, a_i_p3_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p3_p1);
        square_a_epi16(a_r_p3_m1, a_i_p3_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p3_m1);
        square_a_epi16(a_r_p3_m3, a_i_p3_m3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p3_m3);
        square_a_epi16(a_r_p3_m5, a_i_p3_m5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p3_m5);
        square_a_epi16(a_r_p3_m7, a_i_p3_m7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p3_m7);
        square_a_epi16(a_r_p1_p7, a_i_p1_p7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_p7);
        square_a_epi16(a_r_p1_p5, a_i_p1_p5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_p5);
        square_a_epi16(a_r_p1_p3, a_i_p1_p3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_p3);
        square_a_epi16(a_r_p1_p1, a_i_p1_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_p1);
        square_a_epi16(a_r_p1_m1, a_i_p1_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_m1);
        square_a_epi16(a_r_p1_m3, a_i_p1_m3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_m3);
        square_a_epi16(a_r_p1_m5, a_i_p1_m5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_m5);
        square_a_epi16(a_r_p1_m7, a_i_p1_m7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_p1_m7);
        square_a_epi16(a_r_m1_p7, a_i_m1_p7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_p7);
        square_a_epi16(a_r_m1_p5, a_i_m1_p5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_p5);
        square_a_epi16(a_r_m1_p3, a_i_m1_p3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_p3);
        square_a_epi16(a_r_m1_p1, a_i_m1_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_p1);
        square_a_epi16(a_r_m1_m1, a_i_m1_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_m1);
        square_a_epi16(a_r_m1_m3, a_i_m1_m3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_m3);
        square_a_epi16(a_r_m1_m5, a_i_m1_m5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_m5);
        square_a_epi16(a_r_m1_m7, a_i_m1_m7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m1_m7);
        square_a_epi16(a_r_m3_p7, a_i_m3_p7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m3_p7);
        square_a_epi16(a_r_m3_p5, a_i_m3_p5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m3_p5);
        square_a_epi16(a_r_m3_p3, a_i_m3_p3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m3_p3);
        square_a_epi16(a_r_m3_p1, a_i_m3_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m3_p1);
        square_a_epi16(a_r_m3_m1, a_i_m3_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m3_m1);
        square_a_epi16(a_r_m3_m3, a_i_m3_m3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m3_m3);
        square_a_epi16(a_r_m3_m5, a_i_m3_m5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m3_m5);
        square_a_epi16(a_r_m3_m7, a_i_m3_m7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m3_m7);
        square_a_epi16(a_r_m5_p7, a_i_m5_p7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m5_p7);
        square_a_epi16(a_r_m5_p5, a_i_m5_p5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m5_p5);
        square_a_epi16(a_r_m5_p3, a_i_m5_p3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m5_p3);
        square_a_epi16(a_r_m5_p1, a_i_m5_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m5_p1);
        square_a_epi16(a_r_m5_m1, a_i_m5_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m5_m1);
        square_a_epi16(a_r_m5_m3, a_i_m5_m3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m5_m3);
        square_a_epi16(a_r_m5_m5, a_i_m5_m5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m5_m5);
        square_a_epi16(a_r_m5_m7, a_i_m5_m7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m5_m7);
        square_a_epi16(a_r_m7_p7, a_i_m7_p7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m7_p7);
        square_a_epi16(a_r_m7_p5, a_i_m7_p5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m7_p5);
        square_a_epi16(a_r_m7_p3, a_i_m7_p3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m7_p3);
        square_a_epi16(a_r_m7_p1, a_i_m7_p1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m7_p1);
        square_a_epi16(a_r_m7_m1, a_i_m7_m1, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m7_m1);
        square_a_epi16(a_r_m7_m3, a_i_m7_m3, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m7_m3);
        square_a_epi16(a_r_m7_m5, a_i_m7_m5, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m7_m5);
        square_a_epi16(a_r_m7_m7, a_i_m7_m7, ch_mag_int, SQRT_10_OVER_FOUR, a_sq_m7_m7);

        // Computing different multiples of ||h0||^2
        // x=1, y=1
        ch_mag_2_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,ONE_OVER_FOUR_SQRT_42);
        ch_mag_2_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_2_over_42_with_sigma2,1);
        // x=1, y=3
        ch_mag_10_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,FIVE_OVER_FOUR_SQRT_42);
        ch_mag_10_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_10_over_42_with_sigma2,1);
        // x=1, x=5
        ch_mag_26_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,THIRTEEN_OVER_FOUR_SQRT_42);
        ch_mag_26_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_26_over_42_with_sigma2,1);
        // x=1, y=7
        ch_mag_50_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYFIVE_OVER_FOUR_SQRT_42);
        ch_mag_50_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_50_over_42_with_sigma2,1);
        // x=3, y=3
        ch_mag_18_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,NINE_OVER_FOUR_SQRT_42);
        ch_mag_18_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_18_over_42_with_sigma2,1);
        // x=3, y=5
        ch_mag_34_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,SEVENTEEN_OVER_FOUR_SQRT_42);
        ch_mag_34_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_34_over_42_with_sigma2,1);
        // x=3, y=7
        ch_mag_58_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYNINE_OVER_FOUR_SQRT_42);
        ch_mag_58_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_58_over_42_with_sigma2,2);
        // x=5, y=5
        ch_mag_50_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYFIVE_OVER_FOUR_SQRT_42);
        ch_mag_50_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_50_over_42_with_sigma2,1);
        // x=5, y=7
        ch_mag_74_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,THIRTYSEVEN_OVER_FOUR_SQRT_42);
        ch_mag_74_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_74_over_42_with_sigma2,2);
        // x=7, y=7
        ch_mag_98_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,FORTYNINE_OVER_FOUR_SQRT_42);
        ch_mag_98_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_98_over_42_with_sigma2,2);
        
        // Computing Metrics
        xmm0 = _mm_subs_epi16(psi_a_p7_p7, a_sq_p7_p7); xmm1 = _mm_adds_epi16(xmm0, y0_p_7_7); bit_met_p7_p7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_p5, a_sq_p7_p5); xmm1 = _mm_adds_epi16(xmm0, y0_p_7_5); bit_met_p7_p5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_p3, a_sq_p7_p3); xmm1 = _mm_adds_epi16(xmm0, y0_p_7_3); bit_met_p7_p3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_p1, a_sq_p7_p1); xmm1 = _mm_adds_epi16(xmm0, y0_p_7_1); bit_met_p7_p1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_m1, a_sq_p7_m1); xmm1 = _mm_adds_epi16(xmm0, y0_m_7_1); bit_met_p7_m1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_m3, a_sq_p7_m3); xmm1 = _mm_adds_epi16(xmm0, y0_m_7_3); bit_met_p7_m3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_m5, a_sq_p7_m5); xmm1 = _mm_adds_epi16(xmm0, y0_m_7_5); bit_met_p7_m5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_m7, a_sq_p7_m7); xmm1 = _mm_adds_epi16(xmm0, y0_m_7_7); bit_met_p7_m7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_p7, a_sq_p5_p7); xmm1 = _mm_adds_epi16(xmm0, y0_p_5_7); bit_met_p5_p7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_p5, a_sq_p5_p5); xmm1 = _mm_adds_epi16(xmm0, y0_p_5_5); bit_met_p5_p5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_p3, a_sq_p5_p3); xmm1 = _mm_adds_epi16(xmm0, y0_p_5_3); bit_met_p5_p3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_p1, a_sq_p5_p1); xmm1 = _mm_adds_epi16(xmm0, y0_p_5_1); bit_met_p5_p1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_m1, a_sq_p5_m1); xmm1 = _mm_adds_epi16(xmm0, y0_m_5_1); bit_met_p5_m1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_m3, a_sq_p5_m3); xmm1 = _mm_adds_epi16(xmm0, y0_m_5_3); bit_met_p5_m3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_m5, a_sq_p5_m5); xmm1 = _mm_adds_epi16(xmm0, y0_m_5_5); bit_met_p5_m5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_m7, a_sq_p5_m7); xmm1 = _mm_adds_epi16(xmm0, y0_m_5_7); bit_met_p5_m7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_p7, a_sq_p3_p7); xmm1 = _mm_adds_epi16(xmm0, y0_p_3_7); bit_met_p3_p7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_p5, a_sq_p3_p5); xmm1 = _mm_adds_epi16(xmm0, y0_p_3_5); bit_met_p3_p5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_p3, a_sq_p3_p3); xmm1 = _mm_adds_epi16(xmm0, y0_p_3_3); bit_met_p3_p3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_p1, a_sq_p3_p1); xmm1 = _mm_adds_epi16(xmm0, y0_p_3_1); bit_met_p3_p1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_m1, a_sq_p3_m1); xmm1 = _mm_adds_epi16(xmm0, y0_m_3_1); bit_met_p3_m1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_m3, a_sq_p3_m3); xmm1 = _mm_adds_epi16(xmm0, y0_m_3_3); bit_met_p3_m3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_m5, a_sq_p3_m5); xmm1 = _mm_adds_epi16(xmm0, y0_m_3_5); bit_met_p3_m5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_m7, a_sq_p3_m7); xmm1 = _mm_adds_epi16(xmm0, y0_m_3_7); bit_met_p3_m7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_p7, a_sq_p1_p7); xmm1 = _mm_adds_epi16(xmm0, y0_p_1_7); bit_met_p1_p7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_p5, a_sq_p1_p5); xmm1 = _mm_adds_epi16(xmm0, y0_p_1_5); bit_met_p1_p5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_p3, a_sq_p1_p3); xmm1 = _mm_adds_epi16(xmm0, y0_p_1_3); bit_met_p1_p3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_p1, a_sq_p1_p1); xmm1 = _mm_adds_epi16(xmm0, y0_p_1_1); bit_met_p1_p1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_m1, a_sq_p1_m1); xmm1 = _mm_adds_epi16(xmm0, y0_m_1_1); bit_met_p1_m1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_m3, a_sq_p1_m3); xmm1 = _mm_adds_epi16(xmm0, y0_m_1_3); bit_met_p1_m3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_m5, a_sq_p1_m5); xmm1 = _mm_adds_epi16(xmm0, y0_m_1_5); bit_met_p1_m5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_m7, a_sq_p1_m7); xmm1 = _mm_adds_epi16(xmm0, y0_m_1_7); bit_met_p1_m7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);

        xmm0 = _mm_subs_epi16(psi_a_m1_p7, a_sq_m1_p7); xmm1 = _mm_subs_epi16(xmm0, y0_m_1_7); bit_met_m1_p7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_p5, a_sq_m1_p5); xmm1 = _mm_subs_epi16(xmm0, y0_m_1_5); bit_met_m1_p5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_p3, a_sq_m1_p3); xmm1 = _mm_subs_epi16(xmm0, y0_m_1_3); bit_met_m1_p3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_p1, a_sq_m1_p1); xmm1 = _mm_subs_epi16(xmm0, y0_m_1_1); bit_met_m1_p1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_m1, a_sq_m1_m1); xmm1 = _mm_subs_epi16(xmm0, y0_p_1_1); bit_met_m1_m1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_m3, a_sq_m1_m3); xmm1 = _mm_subs_epi16(xmm0, y0_p_1_3); bit_met_m1_m3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_m5, a_sq_m1_m5); xmm1 = _mm_subs_epi16(xmm0, y0_p_1_5); bit_met_m1_m5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_m7, a_sq_m1_m7); xmm1 = _mm_subs_epi16(xmm0, y0_p_1_7); bit_met_m1_m7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_p7, a_sq_m3_p7); xmm1 = _mm_subs_epi16(xmm0, y0_m_3_7); bit_met_m3_p7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_p5, a_sq_m3_p5); xmm1 = _mm_subs_epi16(xmm0, y0_m_3_5); bit_met_m3_p5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_p3, a_sq_m3_p3); xmm1 = _mm_subs_epi16(xmm0, y0_m_3_3); bit_met_m3_p3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_p1, a_sq_m3_p1); xmm1 = _mm_subs_epi16(xmm0, y0_m_3_1); bit_met_m3_p1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_m1, a_sq_m3_m1); xmm1 = _mm_subs_epi16(xmm0, y0_p_3_1); bit_met_m3_m1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_m3, a_sq_m3_m3); xmm1 = _mm_subs_epi16(xmm0, y0_p_3_3); bit_met_m3_m3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_m5, a_sq_m3_m5); xmm1 = _mm_subs_epi16(xmm0, y0_p_3_5); bit_met_m3_m5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_m7, a_sq_m3_m7); xmm1 = _mm_subs_epi16(xmm0, y0_p_3_7); bit_met_m3_m7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_p7, a_sq_m5_p7); xmm1 = _mm_subs_epi16(xmm0, y0_m_5_7); bit_met_m5_p7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_p5, a_sq_m5_p5); xmm1 = _mm_subs_epi16(xmm0, y0_m_5_5); bit_met_m5_p5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_p3, a_sq_m5_p3); xmm1 = _mm_subs_epi16(xmm0, y0_m_5_3); bit_met_m5_p3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_p1, a_sq_m5_p1); xmm1 = _mm_subs_epi16(xmm0, y0_m_5_1); bit_met_m5_p1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_m1, a_sq_m5_m1); xmm1 = _mm_subs_epi16(xmm0, y0_p_5_1); bit_met_m5_m1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_m3, a_sq_m5_m3); xmm1 = _mm_subs_epi16(xmm0, y0_p_5_3); bit_met_m5_m3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_m5, a_sq_m5_m5); xmm1 = _mm_subs_epi16(xmm0, y0_p_5_5); bit_met_m5_m5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_m7, a_sq_m5_m7); xmm1 = _mm_subs_epi16(xmm0, y0_p_5_7); bit_met_m5_m7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_p7, a_sq_m7_p7); xmm1 = _mm_subs_epi16(xmm0, y0_m_7_7); bit_met_m7_p7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_p5, a_sq_m7_p5); xmm1 = _mm_subs_epi16(xmm0, y0_m_7_5); bit_met_m7_p5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_p3, a_sq_m7_p3); xmm1 = _mm_subs_epi16(xmm0, y0_m_7_3); bit_met_m7_p3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_p1, a_sq_m7_p1); xmm1 = _mm_subs_epi16(xmm0, y0_m_7_1); bit_met_m7_p1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_m1, a_sq_m7_m1); xmm1 = _mm_subs_epi16(xmm0, y0_p_7_1); bit_met_m7_m1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_m3, a_sq_m7_m3); xmm1 = _mm_subs_epi16(xmm0, y0_p_7_3); bit_met_m7_m3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_m5, a_sq_m7_m5); xmm1 = _mm_subs_epi16(xmm0, y0_p_7_5); bit_met_m7_m5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_m7, a_sq_m7_m7); xmm1 = _mm_subs_epi16(xmm0, y0_p_7_7); bit_met_m7_m7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);

        // Detection for 1st bit (LTE mapping)
        // bit = 1 
        xmm0 = _mm_max_epi16(bit_met_m7_p7, bit_met_m7_p5);
        xmm1 = _mm_max_epi16(bit_met_m7_p3, bit_met_m7_p1);
        xmm2 = _mm_max_epi16(bit_met_m7_m1, bit_met_m7_m3);
        xmm3 = _mm_max_epi16(bit_met_m7_m5, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);        
        xmm0 = _mm_max_epi16(bit_met_m5_p7, bit_met_m5_p5);
        xmm1 = _mm_max_epi16(bit_met_m5_p3, bit_met_m5_p1);
        xmm2 = _mm_max_epi16(bit_met_m5_m1, bit_met_m5_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m5_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m3_p7, bit_met_m3_p5);
        xmm1 = _mm_max_epi16(bit_met_m3_p3, bit_met_m3_p1);
        xmm2 = _mm_max_epi16(bit_met_m3_m1, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m3_m5, bit_met_m3_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_p7, bit_met_m1_p5);
        xmm1 = _mm_max_epi16(bit_met_m1_p3, bit_met_m1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m1_m3);
        xmm3 = _mm_max_epi16(bit_met_m1_m5, bit_met_m1_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        
        // bit = 0
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p7_p5);
        xmm1 = _mm_max_epi16(bit_met_p7_p3, bit_met_p7_p1);
        xmm2 = _mm_max_epi16(bit_met_p7_m1, bit_met_p7_m3);
        xmm3 = _mm_max_epi16(bit_met_p7_m5, bit_met_p7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_p7, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p5_p3, bit_met_p5_p1);
        xmm2 = _mm_max_epi16(bit_met_p5_m1, bit_met_p5_m3);
        xmm3 = _mm_max_epi16(bit_met_p5_m5, bit_met_p5_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_p7, bit_met_p3_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p3_p1);
        xmm2 = _mm_max_epi16(bit_met_p3_m1, bit_met_p3_m3);
        xmm3 = _mm_max_epi16(bit_met_p3_m5, bit_met_p3_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_p7, bit_met_p1_p5);
        xmm1 = _mm_max_epi16(bit_met_p1_p3, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_p1_m1, bit_met_p1_m3);
        xmm3 = _mm_max_epi16(bit_met_p1_m5, bit_met_p1_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        
        y0r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);
	 
        // Detection for 2nd bit (LTE mapping)
        // bit = 1
        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        
        // bit = 0
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m7_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m7_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y1r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);
     
        // Detection for 3rd bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_m7_m7, bit_met_m7_m5);
        xmm1 = _mm_max_epi16(bit_met_m7_m3, bit_met_m7_m1);
        xmm2 = _mm_max_epi16(bit_met_m7_p1, bit_met_m7_p3);
        xmm3 = _mm_max_epi16(bit_met_m7_p5, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m5_m7, bit_met_m5_m5);
        xmm1 = _mm_max_epi16(bit_met_m5_m3, bit_met_m5_m1);
        xmm2 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_m7, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p5_m3, bit_met_p5_m1);
        xmm2 = _mm_max_epi16(bit_met_p5_p1, bit_met_p5_p3);
        xmm3 = _mm_max_epi16(bit_met_p5_p5, bit_met_p5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p7_m5);
        xmm1 = _mm_max_epi16(bit_met_p7_m3, bit_met_p7_m1);
        xmm2 = _mm_max_epi16(bit_met_p7_p1, bit_met_p7_p3);
        xmm3 = _mm_max_epi16(bit_met_p7_p5, bit_met_p7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_m3_m7, bit_met_m3_m5);
        xmm1 = _mm_max_epi16(bit_met_m3_m3, bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_m3_p1, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_p5, bit_met_m3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_m7, bit_met_m1_m5);
        xmm1 = _mm_max_epi16(bit_met_m1_m3, bit_met_m1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m1_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p5, bit_met_m1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_m7, bit_met_p1_m5);
        xmm1 = _mm_max_epi16(bit_met_p1_m3, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1, bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_p5, bit_met_p1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_m7, bit_met_p3_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p3_m1);
        xmm2 = _mm_max_epi16(bit_met_p3_p1, bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_p5, bit_met_p3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y2r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // Detection for 4th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m7_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y0i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);


        // Detection for 5th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_m7_m7, bit_met_m7_m5);
        xmm1 = _mm_max_epi16(bit_met_m7_m3, bit_met_m7_m1);
        xmm2 = _mm_max_epi16(bit_met_m7_p1, bit_met_m7_p3);
        xmm3 = _mm_max_epi16(bit_met_m7_p5, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_m7, bit_met_m1_m5);
        xmm1 = _mm_max_epi16(bit_met_m1_m3, bit_met_m1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m1_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p5, bit_met_m1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_m7, bit_met_p1_m5);
        xmm1 = _mm_max_epi16(bit_met_p1_m3, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1, bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_p5, bit_met_p1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p7_m5);
        xmm1 = _mm_max_epi16(bit_met_p7_m3, bit_met_p7_m1);
        xmm2 = _mm_max_epi16(bit_met_p7_p1, bit_met_p7_p3);
        xmm3 = _mm_max_epi16(bit_met_p7_p5, bit_met_p7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_m5_m7, bit_met_m5_m5);
        xmm1 = _mm_max_epi16(bit_met_m5_m3, bit_met_m5_m1);
        xmm2 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m3_m7, bit_met_m3_m5);
        xmm1 = _mm_max_epi16(bit_met_m3_m3, bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_m3_p1, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_p5, bit_met_m3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_m7, bit_met_p3_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p3_m1);
        xmm2 = _mm_max_epi16(bit_met_p3_p1, bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_p5, bit_met_p3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_m7, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p5_m3, bit_met_p5_m1);
        xmm2 = _mm_max_epi16(bit_met_p5_p1, bit_met_p5_p3);
        xmm3 = _mm_max_epi16(bit_met_p5_p5, bit_met_p5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y1i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // Detection for 6th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m7_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y2i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // map to output stream, difficult to do in SIMD since we have 6 16bit LLRs
        // RE 1
        j = 24*i;
        stream0_out[j + 0] = ((short *)&y0r)[0];
        stream0_out[j + 1] = ((short *)&y1r)[0];
        stream0_out[j + 2] = ((short *)&y2r)[0];
        stream0_out[j + 3] = ((short *)&y0i)[0];
        stream0_out[j + 4] = ((short *)&y1i)[0];
        stream0_out[j + 5] = ((short *)&y2i)[0];
        // RE 2
        stream0_out[j + 6] = ((short *)&y0r)[1];
        stream0_out[j + 7] = ((short *)&y1r)[1];
        stream0_out[j + 8] = ((short *)&y2r)[1];
        stream0_out[j + 9] = ((short *)&y0i)[1];
        stream0_out[j + 10] = ((short *)&y1i)[1];
        stream0_out[j + 11] = ((short *)&y2i)[1];
        // RE 3
        stream0_out[j + 12] = ((short *)&y0r)[2];
        stream0_out[j + 13] = ((short *)&y1r)[2];
        stream0_out[j + 14] = ((short *)&y2r)[2];
        stream0_out[j + 15] = ((short *)&y0i)[2];
        stream0_out[j + 16] = ((short *)&y1i)[2];
        stream0_out[j + 17] = ((short *)&y2i)[2];
        // RE 4
        stream0_out[j + 18] = ((short *)&y0r)[3];
        stream0_out[j + 19] = ((short *)&y1r)[3];
        stream0_out[j + 20] = ((short *)&y2r)[3];
        stream0_out[j + 21] = ((short *)&y0i)[3];
        stream0_out[j + 22] = ((short *)&y1i)[3];
        stream0_out[j + 23] = ((short *)&y2i)[3];       
        // RE 5
        stream0_out[j + 24] = ((short *)&y0r)[4];
        stream0_out[j + 25] = ((short *)&y1r)[4];
        stream0_out[j + 26] = ((short *)&y2r)[4];
        stream0_out[j + 27] = ((short *)&y0i)[4];
        stream0_out[j + 28] = ((short *)&y1i)[4];
        stream0_out[j + 29] = ((short *)&y2i)[4];       
        // RE 6
        stream0_out[j + 30] = ((short *)&y0r)[5];
        stream0_out[j + 31] = ((short *)&y1r)[5];
        stream0_out[j + 32] = ((short *)&y2r)[5];
        stream0_out[j + 33] = ((short *)&y0i)[5];
        stream0_out[j + 34] = ((short *)&y1i)[5];
        stream0_out[j + 35] = ((short *)&y2i)[5];       
        // RE 7
        stream0_out[j + 36] = ((short *)&y0r)[6];
        stream0_out[j + 37] = ((short *)&y1r)[6];
        stream0_out[j + 38] = ((short *)&y2r)[6];
        stream0_out[j + 39] = ((short *)&y0i)[6];
        stream0_out[j + 40] = ((short *)&y1i)[6];
        stream0_out[j + 41] = ((short *)&y2i)[6];       
        // RE 8
        stream0_out[j + 42] = ((short *)&y0r)[7];
        stream0_out[j + 43] = ((short *)&y1r)[7];
        stream0_out[j + 44] = ((short *)&y2r)[7];
        stream0_out[j + 45] = ((short *)&y0i)[7];
        stream0_out[j + 46] = ((short *)&y1i)[7];
        stream0_out[j + 47] = ((short *)&y2i)[7];       
  }
  _mm_empty();
  _m_empty();

}


int dlsch_64qam_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                          int **rxdataF_comp,
                          int **rxdataF_comp_i,
                          int **dl_ch_mag,
                          int **dl_ch_mag_i,
                          int **rho_i,
                          short *dlsch_llr,
                          unsigned char symbol,
                          unsigned char first_symbol_flag,
                          unsigned short nb_rb,
                          uint16_t pbch_pss_sss_adjust,
                          short **llr16p) {

    int16_t *rxF      = (int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i    = (int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag   = (int16_t*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag_i = (int16_t*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho      = (int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;

    //first symbol has different structure due to more pilots
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
  
    if (!llr16) {
        msg("dlsch_64qam_16qam_llr: llr is null, symbol %d\n",symbol);
        return(-1);
    }

    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }

    qam64_qam16((short *)rxF,
                (short *)rxF_i,
                (short *)ch_mag,
                (short *)ch_mag_i,
                (short *)llr16,
                (short *)rho,
                len);

    llr16 += (6*len);
    *llr16p = (short *)llr16;
    return(0);
}

void qam64_qam64(short *stream0_in,
                 short *stream1_in,
                 short *ch_mag,
                 short *ch_mag_i,
                 short *stream0_out,
                 short *rho01,
                 int length
                 ) {

/* 
   Author: S. Wagner
   Date: 31-07-12

   Input: 
   stream0_in:  MF filter for 1st stream, i.e., y0=h0'*y
   stream1_in:  MF filter for 2nd stream, i.e., y1=h1'*y
   ch_mag:      4*h0/sqrt(42), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   ch_mag_i:    4*h1/sqrt(42), [Re0 Im0 Re1 Im1] s.t. Im0=Re0, Im1=Re1, etc
   rho01:       Channel cross correlation, i.e., h1'*h0
   
   Output:
   stream0_out: output LLRs for 1st stream
*/

    __m128i *rho01_128i      = (__m128i *)rho01;
    __m128i *stream0_128i_in = (__m128i *)stream0_in;
    __m128i *stream1_128i_in = (__m128i *)stream1_in;
    __m128i *ch_mag_128i     = (__m128i *)ch_mag;
    __m128i *ch_mag_128i_i   = (__m128i *)ch_mag_i;

    int i,j;

    ONE_OVER_SQRT_42 = _mm_set1_epi16(10112); // round(1/sqrt(42)*2^16)
    THREE_OVER_SQRT_42 = _mm_set1_epi16(30337); // round(3/sqrt(42)*2^16)
    FIVE_OVER_SQRT_42 = _mm_set1_epi16(25281); // round(5/sqrt(42)*2^15)
    SEVEN_OVER_SQRT_42 = _mm_set1_epi16(17697); // round(7/sqrt(42)*2^14) Q2.14 
    ONE_OVER_SQRT_2 = _mm_set1_epi16(23170); // round(1/sqrt(2)*2^15)
    ONE_OVER_SQRT_2_42 = _mm_set1_epi16(3575); // round(1/sqrt(2*42)*2^15)
    THREE_OVER_SQRT_2_42 = _mm_set1_epi16(10726); // round(3/sqrt(2*42)*2^15)
    FIVE_OVER_SQRT_2_42 = _mm_set1_epi16(17876); // round(5/sqrt(2*42)*2^15)
    SEVEN_OVER_SQRT_2_42 = _mm_set1_epi16(25027); // round(7/sqrt(2*42)*2^15)
    FORTYNINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(30969); // round(49/(4*sqrt(42))*2^14), Q2.14
    THIRTYSEVEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(23385); // round(37/(4*sqrt(42))*2^14), Q2.14
    TWENTYFIVE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(31601); // round(25/(4*sqrt(42))*2^15)
    TWENTYNINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(18329); // round(29/(4*sqrt(42))*2^15), Q2.14
    SEVENTEEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(21489); // round(17/(4*sqrt(42))*2^15)
    NINE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(11376); // round(9/(4*sqrt(42))*2^15)
    THIRTEEN_OVER_FOUR_SQRT_42 = _mm_set1_epi16(16433); // round(13/(4*sqrt(42))*2^15)
    FIVE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(6320); // round(5/(4*sqrt(42))*2^15)
    ONE_OVER_FOUR_SQRT_42 = _mm_set1_epi16(1264); // round(1/(4*sqrt(42))*2^15)
    SQRT_42_OVER_FOUR = _mm_set1_epi16(13272); // round(sqrt(42)/4*2^13), Q3.12

    for (i=0; i<length>>2; i+=2) {
          
        // Get rho
        xmm0 = rho01_128i[i];
        xmm1 = rho01_128i[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2 = _mm_unpacklo_epi64(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_epi64(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_epi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_epi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1 = _mm_mulhi_epi16(rho_rpi, ONE_OVER_SQRT_42);   
        rho_rmi_1_1 = _mm_mulhi_epi16(rho_rmi, ONE_OVER_SQRT_42);
        rho_rpi_3_3 = _mm_mulhi_epi16(rho_rpi, THREE_OVER_SQRT_42); 
        rho_rmi_3_3 = _mm_mulhi_epi16(rho_rmi, THREE_OVER_SQRT_42);
        rho_rpi_5_5 = _mm_mulhi_epi16(rho_rpi, FIVE_OVER_SQRT_42);  
        rho_rmi_5_5 = _mm_mulhi_epi16(rho_rmi, FIVE_OVER_SQRT_42); 
        rho_rpi_7_7 = _mm_mulhi_epi16(rho_rpi, SEVEN_OVER_SQRT_42); 
        rho_rmi_7_7 = _mm_mulhi_epi16(rho_rmi, SEVEN_OVER_SQRT_42);

        rho_rpi_5_5 = _mm_slli_epi16(rho_rpi_5_5, 1); 
        rho_rmi_5_5 = _mm_slli_epi16(rho_rmi_5_5, 1);
        rho_rpi_7_7 = _mm_slli_epi16(rho_rpi_7_7, 2); 
        rho_rmi_7_7 = _mm_slli_epi16(rho_rmi_7_7, 2);

        xmm4 = _mm_mulhi_epi16(xmm2, ONE_OVER_SQRT_42);   
        xmm5 = _mm_mulhi_epi16(xmm3, ONE_OVER_SQRT_42);   
        xmm6 = _mm_mulhi_epi16(xmm3, THREE_OVER_SQRT_42); 
        xmm7 = _mm_mulhi_epi16(xmm3, FIVE_OVER_SQRT_42);  
        xmm8 = _mm_mulhi_epi16(xmm3, SEVEN_OVER_SQRT_42); 
        xmm7 = _mm_slli_epi16(xmm7, 1);
        xmm8 = _mm_slli_epi16(xmm8, 2);

        rho_rpi_1_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_1_3 = _mm_subs_epi16(xmm4, xmm6); 
        rho_rpi_1_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_1_5 = _mm_subs_epi16(xmm4, xmm7);
        rho_rpi_1_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_1_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, THREE_OVER_SQRT_42); 
        rho_rpi_3_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_3_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_3_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_3_5 = _mm_subs_epi16(xmm4, xmm7);
        rho_rpi_3_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_3_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, FIVE_OVER_SQRT_42); 
        xmm4 = _mm_slli_epi16(xmm4, 1);
        rho_rpi_5_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_5_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_5_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_5_3 = _mm_subs_epi16(xmm4, xmm6);
        rho_rpi_5_7 = _mm_adds_epi16(xmm4, xmm8); 
        rho_rmi_5_7 = _mm_subs_epi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_epi16(xmm2, SEVEN_OVER_SQRT_42); 
        xmm4 = _mm_slli_epi16(xmm4, 2);
        rho_rpi_7_1 = _mm_adds_epi16(xmm4, xmm5); 
        rho_rmi_7_1 = _mm_subs_epi16(xmm4, xmm5); 
        rho_rpi_7_3 = _mm_adds_epi16(xmm4, xmm6); 
        rho_rmi_7_3 = _mm_subs_epi16(xmm4, xmm6);
        rho_rpi_7_5 = _mm_adds_epi16(xmm4, xmm7); 
        rho_rmi_7_5 = _mm_subs_epi16(xmm4, xmm7);

        // Rearrange interfering MF output
        xmm0 = stream1_128i_in[i];
        xmm1 = stream1_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r = _mm_unpacklo_epi64(xmm0,xmm1); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i = _mm_unpackhi_epi64(xmm0,xmm1); //[y1i(1),y1i(2),y1i(3),y1i(4)]

        // Psi_r calculation from rho_rpi or rho_rmi
        xmm0 = _mm_xor_si128(xmm0, xmm0); // ZERO for abs_pi16
        xmm2 = _mm_subs_epi16(rho_rpi_7_7, y1r); psi_r_p7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_5, y1r); psi_r_p7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_3, y1r); psi_r_p7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_1, y1r); psi_r_p7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_1, y1r); psi_r_p7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_3, y1r); psi_r_p7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_5, y1r); psi_r_p7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_7, y1r); psi_r_p7_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_7, y1r); psi_r_p5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_5, y1r); psi_r_p5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_3, y1r); psi_r_p5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_1, y1r); psi_r_p5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_1, y1r); psi_r_p5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_3, y1r); psi_r_p5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_5, y1r); psi_r_p5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_7, y1r); psi_r_p5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_7, y1r); psi_r_p3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_5, y1r); psi_r_p3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_3, y1r); psi_r_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_1, y1r); psi_r_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_1, y1r); psi_r_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_3, y1r); psi_r_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_5, y1r); psi_r_p3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_7, y1r); psi_r_p3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_7, y1r); psi_r_p1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_5, y1r); psi_r_p1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_3, y1r); psi_r_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_1, y1r); psi_r_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_1, y1r); psi_r_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_3, y1r); psi_r_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_5, y1r); psi_r_p1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_7, y1r); psi_r_p1_m7 = _mm_abs_epi16(xmm2);
                                                            
        xmm2 = _mm_adds_epi16(rho_rmi_1_7, y1r); psi_r_m1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_5, y1r); psi_r_m1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_3, y1r); psi_r_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_1, y1r); psi_r_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_1, y1r); psi_r_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_3, y1r); psi_r_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_5, y1r); psi_r_m1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_7, y1r); psi_r_m1_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_7, y1r); psi_r_m3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_5, y1r); psi_r_m3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_3, y1r); psi_r_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_1, y1r); psi_r_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_1, y1r); psi_r_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_3, y1r); psi_r_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_5, y1r); psi_r_m3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_7, y1r); psi_r_m3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_7, y1r); psi_r_m5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_5, y1r); psi_r_m5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_3, y1r); psi_r_m5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_1, y1r); psi_r_m5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_1, y1r); psi_r_m5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_3, y1r); psi_r_m5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_5, y1r); psi_r_m5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_7, y1r); psi_r_m5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_7, y1r); psi_r_m7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_5, y1r); psi_r_m7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_3, y1r); psi_r_m7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_1, y1r); psi_r_m7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_1, y1r); psi_r_m7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_3, y1r); psi_r_m7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_5, y1r); psi_r_m7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_7, y1r); psi_r_m7_m7 = _mm_abs_epi16(xmm2);

        // Psi_i calculation from rho_rpi or rho_rmi
        xmm2 = _mm_subs_epi16(rho_rmi_7_7, y1i); psi_i_p7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_7, y1i); psi_i_p7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_7, y1i); psi_i_p7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_7, y1i); psi_i_p7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_7, y1i); psi_i_p7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_7, y1i); psi_i_p7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_7, y1i); psi_i_p7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_7, y1i); psi_i_p7_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_5, y1i); psi_i_p5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_5, y1i); psi_i_p5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_5, y1i); psi_i_p5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_5, y1i); psi_i_p5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_5, y1i); psi_i_p5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_5, y1i); psi_i_p5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_5, y1i); psi_i_p5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_5, y1i); psi_i_p5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_3, y1i); psi_i_p3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_3, y1i); psi_i_p3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_3, y1i); psi_i_p3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_3, y1i); psi_i_p3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_3, y1i); psi_i_p3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_3, y1i); psi_i_p3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_3, y1i); psi_i_p3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_3, y1i); psi_i_p3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_7_1, y1i); psi_i_p1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_5_1, y1i); psi_i_p1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_3_1, y1i); psi_i_p1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rmi_1_1, y1i); psi_i_p1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_1_1, y1i); psi_i_p1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_3_1, y1i); psi_i_p1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_5_1, y1i); psi_i_p1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rpi_7_1, y1i); psi_i_p1_m7 = _mm_abs_epi16(xmm2);

        xmm2 = _mm_subs_epi16(rho_rpi_7_1, y1i); psi_i_m1_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_1, y1i); psi_i_m1_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_1, y1i); psi_i_m1_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_1, y1i); psi_i_m1_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_1, y1i); psi_i_m1_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_1, y1i); psi_i_m1_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_1, y1i); psi_i_m1_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_1, y1i); psi_i_m1_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_3, y1i); psi_i_m3_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_3, y1i); psi_i_m3_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_3, y1i); psi_i_m3_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_3, y1i); psi_i_m3_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_3, y1i); psi_i_m3_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_3, y1i); psi_i_m3_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_3, y1i); psi_i_m3_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_3, y1i); psi_i_m3_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_5, y1i); psi_i_m5_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_5, y1i); psi_i_m5_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_5, y1i); psi_i_m5_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_5, y1i); psi_i_m5_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_5, y1i); psi_i_m5_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_5, y1i); psi_i_m5_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_5, y1i); psi_i_m5_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_5, y1i); psi_i_m5_m7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_7_7, y1i); psi_i_m7_p7 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_5_7, y1i); psi_i_m7_p5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_3_7, y1i); psi_i_m7_p3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_subs_epi16(rho_rpi_1_7, y1i); psi_i_m7_p1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_1_7, y1i); psi_i_m7_m1 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_3_7, y1i); psi_i_m7_m3 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_5_7, y1i); psi_i_m7_m5 = _mm_abs_epi16(xmm2);
        xmm2 = _mm_adds_epi16(rho_rmi_7_7, y1i); psi_i_m7_m7 = _mm_abs_epi16(xmm2);


        // Rearrange desired MF output
        xmm0 = stream0_128i_in[i];
        xmm1 = stream0_128i_in[i+1];
        xmm0 = _mm_shufflelo_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shufflehi_epi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0 = _mm_shuffle_epi32(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflelo_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shufflehi_epi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_epi32(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0 = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1 = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r = _mm_unpacklo_epi64(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i = _mm_unpackhi_epi64(xmm0,xmm1); 
    
        // Rearrange desired channel magnitudes
        xmm2 = ch_mag_128i[i]; // = [|h|^2(1),|h|^2(1),|h|^2(2),|h|^2(2)]*(2/sqrt(10))
        xmm3 = ch_mag_128i[i+1]; // = [|h|^2(3),|h|^2(3),|h|^2(4),|h|^2(4)]*(2/sqrt(10))
        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_des = _mm_unpacklo_epi64(xmm2,xmm3); 

        // Rearrange interfering channel magnitudes
        xmm2 = ch_mag_128i_i[i];   
        xmm3 = ch_mag_128i_i[i+1]; 
        xmm2 = _mm_shufflelo_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shufflehi_epi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_shuffle_epi32(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflelo_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shufflehi_epi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_epi32(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_int  = _mm_unpacklo_epi64(xmm2,xmm3); 

        y0r_one_over_sqrt_21   = _mm_mulhi_epi16(y0r, ONE_OVER_SQRT_42);   
        y0r_three_over_sqrt_21 = _mm_mulhi_epi16(y0r, THREE_OVER_SQRT_42); 
        y0r_five_over_sqrt_21  = _mm_mulhi_epi16(y0r, FIVE_OVER_SQRT_42);  
        y0r_five_over_sqrt_21  = _mm_slli_epi16(y0r_five_over_sqrt_21, 1);
        y0r_seven_over_sqrt_21 = _mm_mulhi_epi16(y0r, SEVEN_OVER_SQRT_42); 
        y0r_seven_over_sqrt_21 = _mm_slli_epi16(y0r_seven_over_sqrt_21, 2); // Q2.14

        y0i_one_over_sqrt_21   = _mm_mulhi_epi16(y0i, ONE_OVER_SQRT_42);   
        y0i_three_over_sqrt_21 = _mm_mulhi_epi16(y0i, THREE_OVER_SQRT_42); 
        y0i_five_over_sqrt_21  = _mm_mulhi_epi16(y0i, FIVE_OVER_SQRT_42);  
        y0i_five_over_sqrt_21  = _mm_slli_epi16(y0i_five_over_sqrt_21, 1);
        y0i_seven_over_sqrt_21 = _mm_mulhi_epi16(y0i, SEVEN_OVER_SQRT_42); 
        y0i_seven_over_sqrt_21 = _mm_slli_epi16(y0i_seven_over_sqrt_21, 2); // Q2.14

        y0_p_7_1 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_7_3 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_7_5 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_7_7 = _mm_adds_epi16(y0r_seven_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_5_1 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_5_3 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_5_5 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_5_7 = _mm_adds_epi16(y0r_five_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_3_1 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_3_3 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_3_5 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_3_7 = _mm_adds_epi16(y0r_three_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_1_1 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_1_3 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_1_5 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_1_7 = _mm_adds_epi16(y0r_one_over_sqrt_21, y0i_seven_over_sqrt_21);

        y0_m_1_1 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_1_3 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_1_5 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_1_7 = _mm_subs_epi16(y0r_one_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_3_1 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_3_3 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_3_5 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_3_7 = _mm_subs_epi16(y0r_three_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_5_1 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_5_3 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_5_5 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_5_7 = _mm_subs_epi16(y0r_five_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_7_1 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_7_3 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_7_5 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_7_7 = _mm_subs_epi16(y0r_seven_over_sqrt_21, y0i_seven_over_sqrt_21);

        // Detection of interference term
        ch_mag_int_with_sigma2       = _mm_srai_epi16(ch_mag_int, 1); // *2
        two_ch_mag_int_with_sigma2   = ch_mag_int; // *4
        three_ch_mag_int_with_sigma2 = _mm_adds_epi16(ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2); // *6

        interference_abs_64qam_epi16(psi_r_p7_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p7_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p7_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p7_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p7_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p7_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p7_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p7_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p7_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p7_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p7_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p7_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p7_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p7_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p7_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p7_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p5_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p5_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p5_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p5_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p5_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p5_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p5_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p5_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p5_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p5_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p5_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p5_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p5_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p5_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p5_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p5_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p3_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p3_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_p1_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_p1_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m1_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m1_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m3_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m3_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m5_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m5_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m5_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m5_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m5_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m5_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m5_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m5_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m5_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m5_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m5_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m5_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m5_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m5_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m5_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m5_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m7_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m7_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m7_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m7_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m7_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m7_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m7_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m7_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m7_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m7_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m7_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m7_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m7_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m7_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_r_m7_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_r_m7_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);

        interference_abs_64qam_epi16(psi_i_p7_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p7_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p7_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p7_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p7_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p7_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p7_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p7_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p7_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p7_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p7_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p7_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p7_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p7_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p7_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p7_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p5_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p5_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p5_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p5_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p5_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p5_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p5_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p5_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p5_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p5_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p5_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p5_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p5_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p5_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p5_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p5_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p3_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p3_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_p1_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_p1_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m1_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m1_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m3_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m3_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m5_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m5_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m5_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m5_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m5_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m5_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m5_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m5_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m5_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m5_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m5_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m5_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m5_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m5_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m5_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m5_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m7_p7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m7_p7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m7_p5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m7_p5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m7_p3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m7_p3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m7_p1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m7_p1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m7_m1, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m7_m1, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m7_m3, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m7_m3, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m7_m5, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m7_m5, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_epi16(psi_i_m7_m7, ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2, three_ch_mag_int_with_sigma2, a_i_m7_m7, ONE_OVER_SQRT_2_42, THREE_OVER_SQRT_2_42, FIVE_OVER_SQRT_2_42, SEVEN_OVER_SQRT_2_42);

        // Calculation of a group of two terms in the bit metric involving product of psi and interference     
        prodsum_psi_a_epi16(psi_r_p7_p7, a_r_p7_p7, psi_i_p7_p7, a_i_p7_p7, psi_a_p7_p7);
        prodsum_psi_a_epi16(psi_r_p7_p5, a_r_p7_p5, psi_i_p7_p5, a_i_p7_p5, psi_a_p7_p5); 
        prodsum_psi_a_epi16(psi_r_p7_p3, a_r_p7_p3, psi_i_p7_p3, a_i_p7_p3, psi_a_p7_p3); 
        prodsum_psi_a_epi16(psi_r_p7_p1, a_r_p7_p1, psi_i_p7_p1, a_i_p7_p1, psi_a_p7_p1); 
        prodsum_psi_a_epi16(psi_r_p7_m1, a_r_p7_m1, psi_i_p7_m1, a_i_p7_m1, psi_a_p7_m1); 
        prodsum_psi_a_epi16(psi_r_p7_m3, a_r_p7_m3, psi_i_p7_m3, a_i_p7_m3, psi_a_p7_m3); 
        prodsum_psi_a_epi16(psi_r_p7_m5, a_r_p7_m5, psi_i_p7_m5, a_i_p7_m5, psi_a_p7_m5); 
        prodsum_psi_a_epi16(psi_r_p7_m7, a_r_p7_m7, psi_i_p7_m7, a_i_p7_m7, psi_a_p7_m7); 
        prodsum_psi_a_epi16(psi_r_p5_p7, a_r_p5_p7, psi_i_p5_p7, a_i_p5_p7, psi_a_p5_p7); 
        prodsum_psi_a_epi16(psi_r_p5_p5, a_r_p5_p5, psi_i_p5_p5, a_i_p5_p5, psi_a_p5_p5); 
        prodsum_psi_a_epi16(psi_r_p5_p3, a_r_p5_p3, psi_i_p5_p3, a_i_p5_p3, psi_a_p5_p3); 
        prodsum_psi_a_epi16(psi_r_p5_p1, a_r_p5_p1, psi_i_p5_p1, a_i_p5_p1, psi_a_p5_p1); 
        prodsum_psi_a_epi16(psi_r_p5_m1, a_r_p5_m1, psi_i_p5_m1, a_i_p5_m1, psi_a_p5_m1); 
        prodsum_psi_a_epi16(psi_r_p5_m3, a_r_p5_m3, psi_i_p5_m3, a_i_p5_m3, psi_a_p5_m3); 
        prodsum_psi_a_epi16(psi_r_p5_m5, a_r_p5_m5, psi_i_p5_m5, a_i_p5_m5, psi_a_p5_m5); 
        prodsum_psi_a_epi16(psi_r_p5_m7, a_r_p5_m7, psi_i_p5_m7, a_i_p5_m7, psi_a_p5_m7); 
        prodsum_psi_a_epi16(psi_r_p3_p7, a_r_p3_p7, psi_i_p3_p7, a_i_p3_p7, psi_a_p3_p7);
        prodsum_psi_a_epi16(psi_r_p3_p5, a_r_p3_p5, psi_i_p3_p5, a_i_p3_p5, psi_a_p3_p5);
        prodsum_psi_a_epi16(psi_r_p3_p3, a_r_p3_p3, psi_i_p3_p3, a_i_p3_p3, psi_a_p3_p3);
        prodsum_psi_a_epi16(psi_r_p3_p1, a_r_p3_p1, psi_i_p3_p1, a_i_p3_p1, psi_a_p3_p1);
        prodsum_psi_a_epi16(psi_r_p3_m1, a_r_p3_m1, psi_i_p3_m1, a_i_p3_m1, psi_a_p3_m1);
        prodsum_psi_a_epi16(psi_r_p3_m3, a_r_p3_m3, psi_i_p3_m3, a_i_p3_m3, psi_a_p3_m3);
        prodsum_psi_a_epi16(psi_r_p3_m5, a_r_p3_m5, psi_i_p3_m5, a_i_p3_m5, psi_a_p3_m5);
        prodsum_psi_a_epi16(psi_r_p3_m7, a_r_p3_m7, psi_i_p3_m7, a_i_p3_m7, psi_a_p3_m7);
        prodsum_psi_a_epi16(psi_r_p1_p7, a_r_p1_p7, psi_i_p1_p7, a_i_p1_p7, psi_a_p1_p7);
        prodsum_psi_a_epi16(psi_r_p1_p5, a_r_p1_p5, psi_i_p1_p5, a_i_p1_p5, psi_a_p1_p5);
        prodsum_psi_a_epi16(psi_r_p1_p3, a_r_p1_p3, psi_i_p1_p3, a_i_p1_p3, psi_a_p1_p3);
        prodsum_psi_a_epi16(psi_r_p1_p1, a_r_p1_p1, psi_i_p1_p1, a_i_p1_p1, psi_a_p1_p1);
        prodsum_psi_a_epi16(psi_r_p1_m1, a_r_p1_m1, psi_i_p1_m1, a_i_p1_m1, psi_a_p1_m1);
        prodsum_psi_a_epi16(psi_r_p1_m3, a_r_p1_m3, psi_i_p1_m3, a_i_p1_m3, psi_a_p1_m3);
        prodsum_psi_a_epi16(psi_r_p1_m5, a_r_p1_m5, psi_i_p1_m5, a_i_p1_m5, psi_a_p1_m5);
        prodsum_psi_a_epi16(psi_r_p1_m7, a_r_p1_m7, psi_i_p1_m7, a_i_p1_m7, psi_a_p1_m7);
        prodsum_psi_a_epi16(psi_r_m1_p7, a_r_m1_p7, psi_i_m1_p7, a_i_m1_p7, psi_a_m1_p7);
        prodsum_psi_a_epi16(psi_r_m1_p5, a_r_m1_p5, psi_i_m1_p5, a_i_m1_p5, psi_a_m1_p5);
        prodsum_psi_a_epi16(psi_r_m1_p3, a_r_m1_p3, psi_i_m1_p3, a_i_m1_p3, psi_a_m1_p3);
        prodsum_psi_a_epi16(psi_r_m1_p1, a_r_m1_p1, psi_i_m1_p1, a_i_m1_p1, psi_a_m1_p1);
        prodsum_psi_a_epi16(psi_r_m1_m1, a_r_m1_m1, psi_i_m1_m1, a_i_m1_m1, psi_a_m1_m1);
        prodsum_psi_a_epi16(psi_r_m1_m3, a_r_m1_m3, psi_i_m1_m3, a_i_m1_m3, psi_a_m1_m3);
        prodsum_psi_a_epi16(psi_r_m1_m5, a_r_m1_m5, psi_i_m1_m5, a_i_m1_m5, psi_a_m1_m5);
        prodsum_psi_a_epi16(psi_r_m1_m7, a_r_m1_m7, psi_i_m1_m7, a_i_m1_m7, psi_a_m1_m7);
        prodsum_psi_a_epi16(psi_r_m3_p7, a_r_m3_p7, psi_i_m3_p7, a_i_m3_p7, psi_a_m3_p7);
        prodsum_psi_a_epi16(psi_r_m3_p5, a_r_m3_p5, psi_i_m3_p5, a_i_m3_p5, psi_a_m3_p5);
        prodsum_psi_a_epi16(psi_r_m3_p3, a_r_m3_p3, psi_i_m3_p3, a_i_m3_p3, psi_a_m3_p3);
        prodsum_psi_a_epi16(psi_r_m3_p1, a_r_m3_p1, psi_i_m3_p1, a_i_m3_p1, psi_a_m3_p1);
        prodsum_psi_a_epi16(psi_r_m3_m1, a_r_m3_m1, psi_i_m3_m1, a_i_m3_m1, psi_a_m3_m1);
        prodsum_psi_a_epi16(psi_r_m3_m3, a_r_m3_m3, psi_i_m3_m3, a_i_m3_m3, psi_a_m3_m3);
        prodsum_psi_a_epi16(psi_r_m3_m5, a_r_m3_m5, psi_i_m3_m5, a_i_m3_m5, psi_a_m3_m5);
        prodsum_psi_a_epi16(psi_r_m3_m7, a_r_m3_m7, psi_i_m3_m7, a_i_m3_m7, psi_a_m3_m7);
        prodsum_psi_a_epi16(psi_r_m5_p7, a_r_m5_p7, psi_i_m5_p7, a_i_m5_p7, psi_a_m5_p7);
        prodsum_psi_a_epi16(psi_r_m5_p5, a_r_m5_p5, psi_i_m5_p5, a_i_m5_p5, psi_a_m5_p5);
        prodsum_psi_a_epi16(psi_r_m5_p3, a_r_m5_p3, psi_i_m5_p3, a_i_m5_p3, psi_a_m5_p3);
        prodsum_psi_a_epi16(psi_r_m5_p1, a_r_m5_p1, psi_i_m5_p1, a_i_m5_p1, psi_a_m5_p1);
        prodsum_psi_a_epi16(psi_r_m5_m1, a_r_m5_m1, psi_i_m5_m1, a_i_m5_m1, psi_a_m5_m1); 
        prodsum_psi_a_epi16(psi_r_m5_m3, a_r_m5_m3, psi_i_m5_m3, a_i_m5_m3, psi_a_m5_m3);
        prodsum_psi_a_epi16(psi_r_m5_m5, a_r_m5_m5, psi_i_m5_m5, a_i_m5_m5, psi_a_m5_m5);
        prodsum_psi_a_epi16(psi_r_m5_m7, a_r_m5_m7, psi_i_m5_m7, a_i_m5_m7, psi_a_m5_m7);
        prodsum_psi_a_epi16(psi_r_m7_p7, a_r_m7_p7, psi_i_m7_p7, a_i_m7_p7, psi_a_m7_p7);
        prodsum_psi_a_epi16(psi_r_m7_p5, a_r_m7_p5, psi_i_m7_p5, a_i_m7_p5, psi_a_m7_p5);
        prodsum_psi_a_epi16(psi_r_m7_p3, a_r_m7_p3, psi_i_m7_p3, a_i_m7_p3, psi_a_m7_p3); 
        prodsum_psi_a_epi16(psi_r_m7_p1, a_r_m7_p1, psi_i_m7_p1, a_i_m7_p1, psi_a_m7_p1);
        prodsum_psi_a_epi16(psi_r_m7_m1, a_r_m7_m1, psi_i_m7_m1, a_i_m7_m1, psi_a_m7_m1); 
        prodsum_psi_a_epi16(psi_r_m7_m3, a_r_m7_m3, psi_i_m7_m3, a_i_m7_m3, psi_a_m7_m3);
        prodsum_psi_a_epi16(psi_r_m7_m5, a_r_m7_m5, psi_i_m7_m5, a_i_m7_m5, psi_a_m7_m5);
        prodsum_psi_a_epi16(psi_r_m7_m7, a_r_m7_m7, psi_i_m7_m7, a_i_m7_m7, psi_a_m7_m7);

        // Multiply by sqrt(2)
        psi_a_p7_p7 = _mm_mulhi_epi16(psi_a_p7_p7, ONE_OVER_SQRT_2);
        psi_a_p7_p7 = _mm_slli_epi16(psi_a_p7_p7, 2);
        psi_a_p7_p5 = _mm_mulhi_epi16(psi_a_p7_p5, ONE_OVER_SQRT_2);
        psi_a_p7_p5 = _mm_slli_epi16(psi_a_p7_p5, 2);
        psi_a_p7_p3 = _mm_mulhi_epi16(psi_a_p7_p3, ONE_OVER_SQRT_2);
        psi_a_p7_p3 = _mm_slli_epi16(psi_a_p7_p3, 2);
        psi_a_p7_p1 = _mm_mulhi_epi16(psi_a_p7_p1, ONE_OVER_SQRT_2);
        psi_a_p7_p1 = _mm_slli_epi16(psi_a_p7_p1, 2);
        psi_a_p7_m1 = _mm_mulhi_epi16(psi_a_p7_m1, ONE_OVER_SQRT_2);
        psi_a_p7_m1 = _mm_slli_epi16(psi_a_p7_m1, 2);
        psi_a_p7_m3 = _mm_mulhi_epi16(psi_a_p7_m3, ONE_OVER_SQRT_2);
        psi_a_p7_m3 = _mm_slli_epi16(psi_a_p7_m3, 2);
        psi_a_p7_m5 = _mm_mulhi_epi16(psi_a_p7_m5, ONE_OVER_SQRT_2);
        psi_a_p7_m5 = _mm_slli_epi16(psi_a_p7_m5, 2);
        psi_a_p7_m7 = _mm_mulhi_epi16(psi_a_p7_m7, ONE_OVER_SQRT_2);
        psi_a_p7_m7 = _mm_slli_epi16(psi_a_p7_m7, 2);
        psi_a_p5_p7 = _mm_mulhi_epi16(psi_a_p5_p7, ONE_OVER_SQRT_2);
        psi_a_p5_p7 = _mm_slli_epi16(psi_a_p5_p7, 2);
        psi_a_p5_p5 = _mm_mulhi_epi16(psi_a_p5_p5, ONE_OVER_SQRT_2);
        psi_a_p5_p5 = _mm_slli_epi16(psi_a_p5_p5, 2);
        psi_a_p5_p3 = _mm_mulhi_epi16(psi_a_p5_p3, ONE_OVER_SQRT_2);
        psi_a_p5_p3 = _mm_slli_epi16(psi_a_p5_p3, 2);
        psi_a_p5_p1 = _mm_mulhi_epi16(psi_a_p5_p1, ONE_OVER_SQRT_2);
        psi_a_p5_p1 = _mm_slli_epi16(psi_a_p5_p1, 2);
        psi_a_p5_m1 = _mm_mulhi_epi16(psi_a_p5_m1, ONE_OVER_SQRT_2);
        psi_a_p5_m1 = _mm_slli_epi16(psi_a_p5_m1, 2);
        psi_a_p5_m3 = _mm_mulhi_epi16(psi_a_p5_m3, ONE_OVER_SQRT_2);
        psi_a_p5_m3 = _mm_slli_epi16(psi_a_p5_m3, 2);
        psi_a_p5_m5 = _mm_mulhi_epi16(psi_a_p5_m5, ONE_OVER_SQRT_2);
        psi_a_p5_m5 = _mm_slli_epi16(psi_a_p5_m5, 2);
        psi_a_p5_m7 = _mm_mulhi_epi16(psi_a_p5_m7, ONE_OVER_SQRT_2);
        psi_a_p5_m7 = _mm_slli_epi16(psi_a_p5_m7, 2);
        psi_a_p3_p7 = _mm_mulhi_epi16(psi_a_p3_p7, ONE_OVER_SQRT_2);
        psi_a_p3_p7 = _mm_slli_epi16(psi_a_p3_p7, 2);
        psi_a_p3_p5 = _mm_mulhi_epi16(psi_a_p3_p5, ONE_OVER_SQRT_2);
        psi_a_p3_p5 = _mm_slli_epi16(psi_a_p3_p5, 2);
        psi_a_p3_p3 = _mm_mulhi_epi16(psi_a_p3_p3, ONE_OVER_SQRT_2);
        psi_a_p3_p3 = _mm_slli_epi16(psi_a_p3_p3, 2);
        psi_a_p3_p1 = _mm_mulhi_epi16(psi_a_p3_p1, ONE_OVER_SQRT_2);
        psi_a_p3_p1 = _mm_slli_epi16(psi_a_p3_p1, 2);
        psi_a_p3_m1 = _mm_mulhi_epi16(psi_a_p3_m1, ONE_OVER_SQRT_2);
        psi_a_p3_m1 = _mm_slli_epi16(psi_a_p3_m1, 2);
        psi_a_p3_m3 = _mm_mulhi_epi16(psi_a_p3_m3, ONE_OVER_SQRT_2);
        psi_a_p3_m3 = _mm_slli_epi16(psi_a_p3_m3, 2);
        psi_a_p3_m5 = _mm_mulhi_epi16(psi_a_p3_m5, ONE_OVER_SQRT_2);
        psi_a_p3_m5 = _mm_slli_epi16(psi_a_p3_m5, 2);
        psi_a_p3_m7 = _mm_mulhi_epi16(psi_a_p3_m7, ONE_OVER_SQRT_2);
        psi_a_p3_m7 = _mm_slli_epi16(psi_a_p3_m7, 2);
        psi_a_p1_p7 = _mm_mulhi_epi16(psi_a_p1_p7, ONE_OVER_SQRT_2);
        psi_a_p1_p7 = _mm_slli_epi16(psi_a_p1_p7, 2);
        psi_a_p1_p5 = _mm_mulhi_epi16(psi_a_p1_p5, ONE_OVER_SQRT_2);
        psi_a_p1_p5 = _mm_slli_epi16(psi_a_p1_p5, 2);
        psi_a_p1_p3 = _mm_mulhi_epi16(psi_a_p1_p3, ONE_OVER_SQRT_2);
        psi_a_p1_p3 = _mm_slli_epi16(psi_a_p1_p3, 2);
        psi_a_p1_p1 = _mm_mulhi_epi16(psi_a_p1_p1, ONE_OVER_SQRT_2);
        psi_a_p1_p1 = _mm_slli_epi16(psi_a_p1_p1, 2);
        psi_a_p1_m1 = _mm_mulhi_epi16(psi_a_p1_m1, ONE_OVER_SQRT_2);
        psi_a_p1_m1 = _mm_slli_epi16(psi_a_p1_m1, 2);
        psi_a_p1_m3 = _mm_mulhi_epi16(psi_a_p1_m3, ONE_OVER_SQRT_2);
        psi_a_p1_m3 = _mm_slli_epi16(psi_a_p1_m3, 2);
        psi_a_p1_m5 = _mm_mulhi_epi16(psi_a_p1_m5, ONE_OVER_SQRT_2);
        psi_a_p1_m5 = _mm_slli_epi16(psi_a_p1_m5, 2);
        psi_a_p1_m7 = _mm_mulhi_epi16(psi_a_p1_m7, ONE_OVER_SQRT_2);
        psi_a_p1_m7 = _mm_slli_epi16(psi_a_p1_m7, 2);
        psi_a_m1_p7 = _mm_mulhi_epi16(psi_a_m1_p7, ONE_OVER_SQRT_2);
        psi_a_m1_p7 = _mm_slli_epi16(psi_a_m1_p7, 2);
        psi_a_m1_p5 = _mm_mulhi_epi16(psi_a_m1_p5, ONE_OVER_SQRT_2);
        psi_a_m1_p5 = _mm_slli_epi16(psi_a_m1_p5, 2);
        psi_a_m1_p3 = _mm_mulhi_epi16(psi_a_m1_p3, ONE_OVER_SQRT_2);
        psi_a_m1_p3 = _mm_slli_epi16(psi_a_m1_p3, 2);
        psi_a_m1_p1 = _mm_mulhi_epi16(psi_a_m1_p1, ONE_OVER_SQRT_2);
        psi_a_m1_p1 = _mm_slli_epi16(psi_a_m1_p1, 2);
        psi_a_m1_m1 = _mm_mulhi_epi16(psi_a_m1_m1, ONE_OVER_SQRT_2);
        psi_a_m1_m1 = _mm_slli_epi16(psi_a_m1_m1, 2);
        psi_a_m1_m3 = _mm_mulhi_epi16(psi_a_m1_m3, ONE_OVER_SQRT_2);
        psi_a_m1_m3 = _mm_slli_epi16(psi_a_m1_m3, 2);
        psi_a_m1_m5 = _mm_mulhi_epi16(psi_a_m1_m5, ONE_OVER_SQRT_2);
        psi_a_m1_m5 = _mm_slli_epi16(psi_a_m1_m5, 2);
        psi_a_m1_m7 = _mm_mulhi_epi16(psi_a_m1_m7, ONE_OVER_SQRT_2);
        psi_a_m1_m7 = _mm_slli_epi16(psi_a_m1_m7, 2);
        psi_a_m3_p7 = _mm_mulhi_epi16(psi_a_m3_p7, ONE_OVER_SQRT_2);
        psi_a_m3_p7 = _mm_slli_epi16(psi_a_m3_p7, 2);
        psi_a_m3_p5 = _mm_mulhi_epi16(psi_a_m3_p5, ONE_OVER_SQRT_2);
        psi_a_m3_p5 = _mm_slli_epi16(psi_a_m3_p5, 2);
        psi_a_m3_p3 = _mm_mulhi_epi16(psi_a_m3_p3, ONE_OVER_SQRT_2);
        psi_a_m3_p3 = _mm_slli_epi16(psi_a_m3_p3, 2);
        psi_a_m3_p1 = _mm_mulhi_epi16(psi_a_m3_p1, ONE_OVER_SQRT_2);
        psi_a_m3_p1 = _mm_slli_epi16(psi_a_m3_p1, 2);
        psi_a_m3_m1 = _mm_mulhi_epi16(psi_a_m3_m1, ONE_OVER_SQRT_2);
        psi_a_m3_m1 = _mm_slli_epi16(psi_a_m3_m1, 2);
        psi_a_m3_m3 = _mm_mulhi_epi16(psi_a_m3_m3, ONE_OVER_SQRT_2);
        psi_a_m3_m3 = _mm_slli_epi16(psi_a_m3_m3, 2);
        psi_a_m3_m5 = _mm_mulhi_epi16(psi_a_m3_m5, ONE_OVER_SQRT_2);
        psi_a_m3_m5 = _mm_slli_epi16(psi_a_m3_m5, 2);
        psi_a_m3_m7 = _mm_mulhi_epi16(psi_a_m3_m7, ONE_OVER_SQRT_2);
        psi_a_m3_m7 = _mm_slli_epi16(psi_a_m3_m7, 2);
        psi_a_m5_p7 = _mm_mulhi_epi16(psi_a_m5_p7, ONE_OVER_SQRT_2);
        psi_a_m5_p7 = _mm_slli_epi16(psi_a_m5_p7, 2);
        psi_a_m5_p5 = _mm_mulhi_epi16(psi_a_m5_p5, ONE_OVER_SQRT_2);
        psi_a_m5_p5 = _mm_slli_epi16(psi_a_m5_p5, 2);
        psi_a_m5_p3 = _mm_mulhi_epi16(psi_a_m5_p3, ONE_OVER_SQRT_2);
        psi_a_m5_p3 = _mm_slli_epi16(psi_a_m5_p3, 2);
        psi_a_m5_p1 = _mm_mulhi_epi16(psi_a_m5_p1, ONE_OVER_SQRT_2);
        psi_a_m5_p1 = _mm_slli_epi16(psi_a_m5_p1, 2);
        psi_a_m5_m1 = _mm_mulhi_epi16(psi_a_m5_m1, ONE_OVER_SQRT_2);
        psi_a_m5_m1 = _mm_slli_epi16(psi_a_m5_m1, 2);
        psi_a_m5_m3 = _mm_mulhi_epi16(psi_a_m5_m3, ONE_OVER_SQRT_2);
        psi_a_m5_m3 = _mm_slli_epi16(psi_a_m5_m3, 2);
        psi_a_m5_m5 = _mm_mulhi_epi16(psi_a_m5_m5, ONE_OVER_SQRT_2);
        psi_a_m5_m5 = _mm_slli_epi16(psi_a_m5_m5, 2);
        psi_a_m5_m7 = _mm_mulhi_epi16(psi_a_m5_m7, ONE_OVER_SQRT_2);
        psi_a_m5_m7 = _mm_slli_epi16(psi_a_m5_m7, 2);
        psi_a_m7_p7 = _mm_mulhi_epi16(psi_a_m7_p7, ONE_OVER_SQRT_2);
        psi_a_m7_p7 = _mm_slli_epi16(psi_a_m7_p7, 2);
        psi_a_m7_p5 = _mm_mulhi_epi16(psi_a_m7_p5, ONE_OVER_SQRT_2);
        psi_a_m7_p5 = _mm_slli_epi16(psi_a_m7_p5, 2);
        psi_a_m7_p3 = _mm_mulhi_epi16(psi_a_m7_p3, ONE_OVER_SQRT_2);
        psi_a_m7_p3 = _mm_slli_epi16(psi_a_m7_p3, 2);
        psi_a_m7_p1 = _mm_mulhi_epi16(psi_a_m7_p1, ONE_OVER_SQRT_2);
        psi_a_m7_p1 = _mm_slli_epi16(psi_a_m7_p1, 2);
        psi_a_m7_m1 = _mm_mulhi_epi16(psi_a_m7_m1, ONE_OVER_SQRT_2);
        psi_a_m7_m1 = _mm_slli_epi16(psi_a_m7_m1, 2);
        psi_a_m7_m3 = _mm_mulhi_epi16(psi_a_m7_m3, ONE_OVER_SQRT_2);
        psi_a_m7_m3 = _mm_slli_epi16(psi_a_m7_m3, 2);
        psi_a_m7_m5 = _mm_mulhi_epi16(psi_a_m7_m5, ONE_OVER_SQRT_2);
        psi_a_m7_m5 = _mm_slli_epi16(psi_a_m7_m5, 2);
        psi_a_m7_m7 = _mm_mulhi_epi16(psi_a_m7_m7, ONE_OVER_SQRT_2);
        psi_a_m7_m7 = _mm_slli_epi16(psi_a_m7_m7, 2);

        // Calculation of a group of two terms in the bit metric involving squares of interference
        square_a_64qam_epi16(a_r_p7_p7, a_i_p7_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_p7);
        square_a_64qam_epi16(a_r_p7_p5, a_i_p7_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_p5);
        square_a_64qam_epi16(a_r_p7_p3, a_i_p7_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_p3);
        square_a_64qam_epi16(a_r_p7_p1, a_i_p7_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_p1);
        square_a_64qam_epi16(a_r_p7_m1, a_i_p7_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_m1);
        square_a_64qam_epi16(a_r_p7_m3, a_i_p7_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_m3);
        square_a_64qam_epi16(a_r_p7_m5, a_i_p7_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_m5);
        square_a_64qam_epi16(a_r_p7_m7, a_i_p7_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_m7);
        square_a_64qam_epi16(a_r_p5_p7, a_i_p5_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_p7);
        square_a_64qam_epi16(a_r_p5_p5, a_i_p5_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_p5);
        square_a_64qam_epi16(a_r_p5_p3, a_i_p5_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_p3);
        square_a_64qam_epi16(a_r_p5_p1, a_i_p5_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_p1);
        square_a_64qam_epi16(a_r_p5_m1, a_i_p5_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_m1);
        square_a_64qam_epi16(a_r_p5_m3, a_i_p5_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_m3);
        square_a_64qam_epi16(a_r_p5_m5, a_i_p5_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_m5);
        square_a_64qam_epi16(a_r_p5_m7, a_i_p5_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_m7);
        square_a_64qam_epi16(a_r_p3_p7, a_i_p3_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_p7);
        square_a_64qam_epi16(a_r_p3_p5, a_i_p3_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_p5);
        square_a_64qam_epi16(a_r_p3_p3, a_i_p3_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_p3);
        square_a_64qam_epi16(a_r_p3_p1, a_i_p3_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_p1);
        square_a_64qam_epi16(a_r_p3_m1, a_i_p3_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_m1);
        square_a_64qam_epi16(a_r_p3_m3, a_i_p3_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_m3);
        square_a_64qam_epi16(a_r_p3_m5, a_i_p3_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_m5);
        square_a_64qam_epi16(a_r_p3_m7, a_i_p3_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_m7);
        square_a_64qam_epi16(a_r_p1_p7, a_i_p1_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p7);
        square_a_64qam_epi16(a_r_p1_p5, a_i_p1_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p5);
        square_a_64qam_epi16(a_r_p1_p3, a_i_p1_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p3);
        square_a_64qam_epi16(a_r_p1_p1, a_i_p1_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p1);
        square_a_64qam_epi16(a_r_p1_m1, a_i_p1_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m1);
        square_a_64qam_epi16(a_r_p1_m3, a_i_p1_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m3);
        square_a_64qam_epi16(a_r_p1_m5, a_i_p1_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m5);
        square_a_64qam_epi16(a_r_p1_m7, a_i_p1_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m7);
        square_a_64qam_epi16(a_r_m1_p7, a_i_m1_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p7);
        square_a_64qam_epi16(a_r_m1_p5, a_i_m1_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p5);
        square_a_64qam_epi16(a_r_m1_p3, a_i_m1_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p3);
        square_a_64qam_epi16(a_r_m1_p1, a_i_m1_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p1);
        square_a_64qam_epi16(a_r_m1_m1, a_i_m1_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m1);
        square_a_64qam_epi16(a_r_m1_m3, a_i_m1_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m3);
        square_a_64qam_epi16(a_r_m1_m5, a_i_m1_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m5);
        square_a_64qam_epi16(a_r_m1_m7, a_i_m1_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m7);
        square_a_64qam_epi16(a_r_m3_p7, a_i_m3_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_p7);
        square_a_64qam_epi16(a_r_m3_p5, a_i_m3_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_p5);
        square_a_64qam_epi16(a_r_m3_p3, a_i_m3_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_p3);
        square_a_64qam_epi16(a_r_m3_p1, a_i_m3_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_p1);
        square_a_64qam_epi16(a_r_m3_m1, a_i_m3_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_m1);
        square_a_64qam_epi16(a_r_m3_m3, a_i_m3_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_m3);
        square_a_64qam_epi16(a_r_m3_m5, a_i_m3_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_m5);
        square_a_64qam_epi16(a_r_m3_m7, a_i_m3_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_m7);
        square_a_64qam_epi16(a_r_m5_p7, a_i_m5_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_p7);
        square_a_64qam_epi16(a_r_m5_p5, a_i_m5_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_p5);
        square_a_64qam_epi16(a_r_m5_p3, a_i_m5_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_p3);
        square_a_64qam_epi16(a_r_m5_p1, a_i_m5_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_p1);
        square_a_64qam_epi16(a_r_m5_m1, a_i_m5_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_m1);
        square_a_64qam_epi16(a_r_m5_m3, a_i_m5_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_m3);
        square_a_64qam_epi16(a_r_m5_m5, a_i_m5_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_m5);
        square_a_64qam_epi16(a_r_m5_m7, a_i_m5_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_m7);
        square_a_64qam_epi16(a_r_m7_p7, a_i_m7_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_p7);
        square_a_64qam_epi16(a_r_m7_p5, a_i_m7_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_p5);
        square_a_64qam_epi16(a_r_m7_p3, a_i_m7_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_p3);
        square_a_64qam_epi16(a_r_m7_p1, a_i_m7_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_p1);
        square_a_64qam_epi16(a_r_m7_m1, a_i_m7_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_m1);
        square_a_64qam_epi16(a_r_m7_m3, a_i_m7_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_m3);
        square_a_64qam_epi16(a_r_m7_m5, a_i_m7_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_m5);
        square_a_64qam_epi16(a_r_m7_m7, a_i_m7_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_m7);

        // Computing different multiples of ||h0||^2
        // x=1, y=1
        ch_mag_2_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,ONE_OVER_FOUR_SQRT_42);
        ch_mag_2_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_2_over_42_with_sigma2,1);
        // x=1, y=3
        ch_mag_10_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,FIVE_OVER_FOUR_SQRT_42);
        ch_mag_10_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_10_over_42_with_sigma2,1);
        // x=1, x=5
        ch_mag_26_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,THIRTEEN_OVER_FOUR_SQRT_42);
        ch_mag_26_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_26_over_42_with_sigma2,1);
        // x=1, y=7
        ch_mag_50_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYFIVE_OVER_FOUR_SQRT_42);
        ch_mag_50_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_50_over_42_with_sigma2,1);
        // x=3, y=3
        ch_mag_18_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,NINE_OVER_FOUR_SQRT_42);
        ch_mag_18_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_18_over_42_with_sigma2,1);
        // x=3, y=5
        ch_mag_34_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,SEVENTEEN_OVER_FOUR_SQRT_42);
        ch_mag_34_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_34_over_42_with_sigma2,1);
        // x=3, y=7
        ch_mag_58_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYNINE_OVER_FOUR_SQRT_42);
        ch_mag_58_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_58_over_42_with_sigma2,2);
        // x=5, y=5
        ch_mag_50_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,TWENTYFIVE_OVER_FOUR_SQRT_42);
        ch_mag_50_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_50_over_42_with_sigma2,1);
        // x=5, y=7
        ch_mag_74_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,THIRTYSEVEN_OVER_FOUR_SQRT_42);
        ch_mag_74_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_74_over_42_with_sigma2,2);
        // x=7, y=7
        ch_mag_98_over_42_with_sigma2 = _mm_mulhi_epi16(ch_mag_des,FORTYNINE_OVER_FOUR_SQRT_42);
        ch_mag_98_over_42_with_sigma2 = _mm_slli_epi16(ch_mag_98_over_42_with_sigma2,2);
        
        // Computing Metrics
        xmm0 = _mm_subs_epi16(psi_a_p7_p7, a_sq_p7_p7); xmm1 = _mm_adds_epi16(xmm0, y0_p_7_7); bit_met_p7_p7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_p5, a_sq_p7_p5); xmm1 = _mm_adds_epi16(xmm0, y0_p_7_5); bit_met_p7_p5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_p3, a_sq_p7_p3); xmm1 = _mm_adds_epi16(xmm0, y0_p_7_3); bit_met_p7_p3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_p1, a_sq_p7_p1); xmm1 = _mm_adds_epi16(xmm0, y0_p_7_1); bit_met_p7_p1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_m1, a_sq_p7_m1); xmm1 = _mm_adds_epi16(xmm0, y0_m_7_1); bit_met_p7_m1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_m3, a_sq_p7_m3); xmm1 = _mm_adds_epi16(xmm0, y0_m_7_3); bit_met_p7_m3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_m5, a_sq_p7_m5); xmm1 = _mm_adds_epi16(xmm0, y0_m_7_5); bit_met_p7_m5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p7_m7, a_sq_p7_m7); xmm1 = _mm_adds_epi16(xmm0, y0_m_7_7); bit_met_p7_m7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_p7, a_sq_p5_p7); xmm1 = _mm_adds_epi16(xmm0, y0_p_5_7); bit_met_p5_p7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_p5, a_sq_p5_p5); xmm1 = _mm_adds_epi16(xmm0, y0_p_5_5); bit_met_p5_p5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_p3, a_sq_p5_p3); xmm1 = _mm_adds_epi16(xmm0, y0_p_5_3); bit_met_p5_p3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_p1, a_sq_p5_p1); xmm1 = _mm_adds_epi16(xmm0, y0_p_5_1); bit_met_p5_p1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_m1, a_sq_p5_m1); xmm1 = _mm_adds_epi16(xmm0, y0_m_5_1); bit_met_p5_m1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_m3, a_sq_p5_m3); xmm1 = _mm_adds_epi16(xmm0, y0_m_5_3); bit_met_p5_m3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_m5, a_sq_p5_m5); xmm1 = _mm_adds_epi16(xmm0, y0_m_5_5); bit_met_p5_m5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p5_m7, a_sq_p5_m7); xmm1 = _mm_adds_epi16(xmm0, y0_m_5_7); bit_met_p5_m7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_p7, a_sq_p3_p7); xmm1 = _mm_adds_epi16(xmm0, y0_p_3_7); bit_met_p3_p7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_p5, a_sq_p3_p5); xmm1 = _mm_adds_epi16(xmm0, y0_p_3_5); bit_met_p3_p5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_p3, a_sq_p3_p3); xmm1 = _mm_adds_epi16(xmm0, y0_p_3_3); bit_met_p3_p3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_p1, a_sq_p3_p1); xmm1 = _mm_adds_epi16(xmm0, y0_p_3_1); bit_met_p3_p1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_m1, a_sq_p3_m1); xmm1 = _mm_adds_epi16(xmm0, y0_m_3_1); bit_met_p3_m1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_m3, a_sq_p3_m3); xmm1 = _mm_adds_epi16(xmm0, y0_m_3_3); bit_met_p3_m3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_m5, a_sq_p3_m5); xmm1 = _mm_adds_epi16(xmm0, y0_m_3_5); bit_met_p3_m5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p3_m7, a_sq_p3_m7); xmm1 = _mm_adds_epi16(xmm0, y0_m_3_7); bit_met_p3_m7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_p7, a_sq_p1_p7); xmm1 = _mm_adds_epi16(xmm0, y0_p_1_7); bit_met_p1_p7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_p5, a_sq_p1_p5); xmm1 = _mm_adds_epi16(xmm0, y0_p_1_5); bit_met_p1_p5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_p3, a_sq_p1_p3); xmm1 = _mm_adds_epi16(xmm0, y0_p_1_3); bit_met_p1_p3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_p1, a_sq_p1_p1); xmm1 = _mm_adds_epi16(xmm0, y0_p_1_1); bit_met_p1_p1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_m1, a_sq_p1_m1); xmm1 = _mm_adds_epi16(xmm0, y0_m_1_1); bit_met_p1_m1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_m3, a_sq_p1_m3); xmm1 = _mm_adds_epi16(xmm0, y0_m_1_3); bit_met_p1_m3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_m5, a_sq_p1_m5); xmm1 = _mm_adds_epi16(xmm0, y0_m_1_5); bit_met_p1_m5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_p1_m7, a_sq_p1_m7); xmm1 = _mm_adds_epi16(xmm0, y0_m_1_7); bit_met_p1_m7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);

        xmm0 = _mm_subs_epi16(psi_a_m1_p7, a_sq_m1_p7); xmm1 = _mm_subs_epi16(xmm0, y0_m_1_7); bit_met_m1_p7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_p5, a_sq_m1_p5); xmm1 = _mm_subs_epi16(xmm0, y0_m_1_5); bit_met_m1_p5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_p3, a_sq_m1_p3); xmm1 = _mm_subs_epi16(xmm0, y0_m_1_3); bit_met_m1_p3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_p1, a_sq_m1_p1); xmm1 = _mm_subs_epi16(xmm0, y0_m_1_1); bit_met_m1_p1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_m1, a_sq_m1_m1); xmm1 = _mm_subs_epi16(xmm0, y0_p_1_1); bit_met_m1_m1 = _mm_subs_epi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_m3, a_sq_m1_m3); xmm1 = _mm_subs_epi16(xmm0, y0_p_1_3); bit_met_m1_m3 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_m5, a_sq_m1_m5); xmm1 = _mm_subs_epi16(xmm0, y0_p_1_5); bit_met_m1_m5 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m1_m7, a_sq_m1_m7); xmm1 = _mm_subs_epi16(xmm0, y0_p_1_7); bit_met_m1_m7 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_p7, a_sq_m3_p7); xmm1 = _mm_subs_epi16(xmm0, y0_m_3_7); bit_met_m3_p7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_p5, a_sq_m3_p5); xmm1 = _mm_subs_epi16(xmm0, y0_m_3_5); bit_met_m3_p5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_p3, a_sq_m3_p3); xmm1 = _mm_subs_epi16(xmm0, y0_m_3_3); bit_met_m3_p3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_p1, a_sq_m3_p1); xmm1 = _mm_subs_epi16(xmm0, y0_m_3_1); bit_met_m3_p1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_m1, a_sq_m3_m1); xmm1 = _mm_subs_epi16(xmm0, y0_p_3_1); bit_met_m3_m1 = _mm_subs_epi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_m3, a_sq_m3_m3); xmm1 = _mm_subs_epi16(xmm0, y0_p_3_3); bit_met_m3_m3 = _mm_subs_epi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_m5, a_sq_m3_m5); xmm1 = _mm_subs_epi16(xmm0, y0_p_3_5); bit_met_m3_m5 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m3_m7, a_sq_m3_m7); xmm1 = _mm_subs_epi16(xmm0, y0_p_3_7); bit_met_m3_m7 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_p7, a_sq_m5_p7); xmm1 = _mm_subs_epi16(xmm0, y0_m_5_7); bit_met_m5_p7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_p5, a_sq_m5_p5); xmm1 = _mm_subs_epi16(xmm0, y0_m_5_5); bit_met_m5_p5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_p3, a_sq_m5_p3); xmm1 = _mm_subs_epi16(xmm0, y0_m_5_3); bit_met_m5_p3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_p1, a_sq_m5_p1); xmm1 = _mm_subs_epi16(xmm0, y0_m_5_1); bit_met_m5_p1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_m1, a_sq_m5_m1); xmm1 = _mm_subs_epi16(xmm0, y0_p_5_1); bit_met_m5_m1 = _mm_subs_epi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_m3, a_sq_m5_m3); xmm1 = _mm_subs_epi16(xmm0, y0_p_5_3); bit_met_m5_m3 = _mm_subs_epi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_m5, a_sq_m5_m5); xmm1 = _mm_subs_epi16(xmm0, y0_p_5_5); bit_met_m5_m5 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m5_m7, a_sq_m5_m7); xmm1 = _mm_subs_epi16(xmm0, y0_p_5_7); bit_met_m5_m7 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_p7, a_sq_m7_p7); xmm1 = _mm_subs_epi16(xmm0, y0_m_7_7); bit_met_m7_p7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_p5, a_sq_m7_p5); xmm1 = _mm_subs_epi16(xmm0, y0_m_7_5); bit_met_m7_p5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_p3, a_sq_m7_p3); xmm1 = _mm_subs_epi16(xmm0, y0_m_7_3); bit_met_m7_p3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_p1, a_sq_m7_p1); xmm1 = _mm_subs_epi16(xmm0, y0_m_7_1); bit_met_m7_p1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_m1, a_sq_m7_m1); xmm1 = _mm_subs_epi16(xmm0, y0_p_7_1); bit_met_m7_m1 = _mm_subs_epi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_m3, a_sq_m7_m3); xmm1 = _mm_subs_epi16(xmm0, y0_p_7_3); bit_met_m7_m3 = _mm_subs_epi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_m5, a_sq_m7_m5); xmm1 = _mm_subs_epi16(xmm0, y0_p_7_5); bit_met_m7_m5 = _mm_subs_epi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_epi16(psi_a_m7_m7, a_sq_m7_m7); xmm1 = _mm_subs_epi16(xmm0, y0_p_7_7); bit_met_m7_m7 = _mm_subs_epi16(xmm1, ch_mag_98_over_42_with_sigma2);

        // Detection for 1st bit (LTE mapping)
        // bit = 1 
        xmm0 = _mm_max_epi16(bit_met_m7_p7, bit_met_m7_p5);
        xmm1 = _mm_max_epi16(bit_met_m7_p3, bit_met_m7_p1);
        xmm2 = _mm_max_epi16(bit_met_m7_m1, bit_met_m7_m3);
        xmm3 = _mm_max_epi16(bit_met_m7_m5, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);        
        xmm0 = _mm_max_epi16(bit_met_m5_p7, bit_met_m5_p5);
        xmm1 = _mm_max_epi16(bit_met_m5_p3, bit_met_m5_p1);
        xmm2 = _mm_max_epi16(bit_met_m5_m1, bit_met_m5_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m5_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m3_p7, bit_met_m3_p5);
        xmm1 = _mm_max_epi16(bit_met_m3_p3, bit_met_m3_p1);
        xmm2 = _mm_max_epi16(bit_met_m3_m1, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m3_m5, bit_met_m3_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_p7, bit_met_m1_p5);
        xmm1 = _mm_max_epi16(bit_met_m1_p3, bit_met_m1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m1_m3);
        xmm3 = _mm_max_epi16(bit_met_m1_m5, bit_met_m1_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        
        // bit = 0
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p7_p5);
        xmm1 = _mm_max_epi16(bit_met_p7_p3, bit_met_p7_p1);
        xmm2 = _mm_max_epi16(bit_met_p7_m1, bit_met_p7_m3);
        xmm3 = _mm_max_epi16(bit_met_p7_m5, bit_met_p7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_p7, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p5_p3, bit_met_p5_p1);
        xmm2 = _mm_max_epi16(bit_met_p5_m1, bit_met_p5_m3);
        xmm3 = _mm_max_epi16(bit_met_p5_m5, bit_met_p5_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_p7, bit_met_p3_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p3_p1);
        xmm2 = _mm_max_epi16(bit_met_p3_m1, bit_met_p3_m3);
        xmm3 = _mm_max_epi16(bit_met_p3_m5, bit_met_p3_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_p7, bit_met_p1_p5);
        xmm1 = _mm_max_epi16(bit_met_p1_p3, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_p1_m1, bit_met_p1_m3);
        xmm3 = _mm_max_epi16(bit_met_p1_m5, bit_met_p1_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        
        y0r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);
	 
        // Detection for 2nd bit (LTE mapping)
        // bit = 1
        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        
        // bit = 0
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m7_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m7_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y1r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);
     
        // Detection for 3rd bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_m7_m7, bit_met_m7_m5);
        xmm1 = _mm_max_epi16(bit_met_m7_m3, bit_met_m7_m1);
        xmm2 = _mm_max_epi16(bit_met_m7_p1, bit_met_m7_p3);
        xmm3 = _mm_max_epi16(bit_met_m7_p5, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m5_m7, bit_met_m5_m5);
        xmm1 = _mm_max_epi16(bit_met_m5_m3, bit_met_m5_m1);
        xmm2 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_m7, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p5_m3, bit_met_p5_m1);
        xmm2 = _mm_max_epi16(bit_met_p5_p1, bit_met_p5_p3);
        xmm3 = _mm_max_epi16(bit_met_p5_p5, bit_met_p5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p7_m5);
        xmm1 = _mm_max_epi16(bit_met_p7_m3, bit_met_p7_m1);
        xmm2 = _mm_max_epi16(bit_met_p7_p1, bit_met_p7_p3);
        xmm3 = _mm_max_epi16(bit_met_p7_p5, bit_met_p7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_m3_m7, bit_met_m3_m5);
        xmm1 = _mm_max_epi16(bit_met_m3_m3, bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_m3_p1, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_p5, bit_met_m3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_m7, bit_met_m1_m5);
        xmm1 = _mm_max_epi16(bit_met_m1_m3, bit_met_m1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m1_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p5, bit_met_m1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_m7, bit_met_p1_m5);
        xmm1 = _mm_max_epi16(bit_met_p1_m3, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1, bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_p5, bit_met_p1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_m7, bit_met_p3_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p3_m1);
        xmm2 = _mm_max_epi16(bit_met_p3_p1, bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_p5, bit_met_p3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y2r = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // Detection for 4th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m7_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y0i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);


        // Detection for 5th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_m7_m7, bit_met_m7_m5);
        xmm1 = _mm_max_epi16(bit_met_m7_m3, bit_met_m7_m1);
        xmm2 = _mm_max_epi16(bit_met_m7_p1, bit_met_m7_p3);
        xmm3 = _mm_max_epi16(bit_met_m7_p5, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m1_m7, bit_met_m1_m5);
        xmm1 = _mm_max_epi16(bit_met_m1_m3, bit_met_m1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m1_p3);
        xmm3 = _mm_max_epi16(bit_met_m1_p5, bit_met_m1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p1_m7, bit_met_p1_m5);
        xmm1 = _mm_max_epi16(bit_met_p1_m3, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_p1_p1, bit_met_p1_p3);
        xmm3 = _mm_max_epi16(bit_met_p1_p5, bit_met_p1_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p7_m5);
        xmm1 = _mm_max_epi16(bit_met_p7_m3, bit_met_p7_m1);
        xmm2 = _mm_max_epi16(bit_met_p7_p1, bit_met_p7_p3);
        xmm3 = _mm_max_epi16(bit_met_p7_p5, bit_met_p7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_m5_m7, bit_met_m5_m5);
        xmm1 = _mm_max_epi16(bit_met_m5_m3, bit_met_m5_m1);
        xmm2 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_m3_m7, bit_met_m3_m5);
        xmm1 = _mm_max_epi16(bit_met_m3_m3, bit_met_m3_m1);
        xmm2 = _mm_max_epi16(bit_met_m3_p1, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m3_p5, bit_met_m3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p3_m7, bit_met_p3_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p3_m1);
        xmm2 = _mm_max_epi16(bit_met_p3_p1, bit_met_p3_p3);
        xmm3 = _mm_max_epi16(bit_met_p3_p5, bit_met_p3_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p5_m7, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p5_m3, bit_met_p5_m1);
        xmm2 = _mm_max_epi16(bit_met_p5_p1, bit_met_p5_p3);
        xmm3 = _mm_max_epi16(bit_met_p5_p5, bit_met_p5_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y1i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // Detection for 6th bit (LTE mapping)
        xmm0 = _mm_max_epi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_epi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_epi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_epi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_epi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_epi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_epi16(bit_met_m5_p1, bit_met_m5_p1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_epi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_epi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_epi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_epi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_epi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_epi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_epi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_epi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_epi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_epi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_epi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(xmm4, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_epi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_epi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_epi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_epi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_epi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_epi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_epi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_epi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_epi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_epi16(bit_met_m5_p5, bit_met_m7_p5);
        xmm4 = _mm_max_epi16(xmm0, xmm1);
        xmm5 = _mm_max_epi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_epi16(logmax_num_re0, xmm5);

        y2i = _mm_subs_epi16(logmax_num_re0, logmax_den_re0);

        // map to output stream, difficult to do in SIMD since we have 6 16bit LLRs
        // RE 1
        j = 24*i;
        stream0_out[j + 0] = ((short *)&y0r)[0];
        stream0_out[j + 1] = ((short *)&y1r)[0];
        stream0_out[j + 2] = ((short *)&y2r)[0];
        stream0_out[j + 3] = ((short *)&y0i)[0];
        stream0_out[j + 4] = ((short *)&y1i)[0];
        stream0_out[j + 5] = ((short *)&y2i)[0];
        // RE 2
        stream0_out[j + 6] = ((short *)&y0r)[1];
        stream0_out[j + 7] = ((short *)&y1r)[1];
        stream0_out[j + 8] = ((short *)&y2r)[1];
        stream0_out[j + 9] = ((short *)&y0i)[1];
        stream0_out[j + 10] = ((short *)&y1i)[1];
        stream0_out[j + 11] = ((short *)&y2i)[1];
        // RE 3
        stream0_out[j + 12] = ((short *)&y0r)[2];
        stream0_out[j + 13] = ((short *)&y1r)[2];
        stream0_out[j + 14] = ((short *)&y2r)[2];
        stream0_out[j + 15] = ((short *)&y0i)[2];
        stream0_out[j + 16] = ((short *)&y1i)[2];
        stream0_out[j + 17] = ((short *)&y2i)[2];
        // RE 4
        stream0_out[j + 18] = ((short *)&y0r)[3];
        stream0_out[j + 19] = ((short *)&y1r)[3];
        stream0_out[j + 20] = ((short *)&y2r)[3];
        stream0_out[j + 21] = ((short *)&y0i)[3];
        stream0_out[j + 22] = ((short *)&y1i)[3];
        stream0_out[j + 23] = ((short *)&y2i)[3];       
        // RE 5
        stream0_out[j + 24] = ((short *)&y0r)[4];
        stream0_out[j + 25] = ((short *)&y1r)[4];
        stream0_out[j + 26] = ((short *)&y2r)[4];
        stream0_out[j + 27] = ((short *)&y0i)[4];
        stream0_out[j + 28] = ((short *)&y1i)[4];
        stream0_out[j + 29] = ((short *)&y2i)[4];       
        // RE 6
        stream0_out[j + 30] = ((short *)&y0r)[5];
        stream0_out[j + 31] = ((short *)&y1r)[5];
        stream0_out[j + 32] = ((short *)&y2r)[5];
        stream0_out[j + 33] = ((short *)&y0i)[5];
        stream0_out[j + 34] = ((short *)&y1i)[5];
        stream0_out[j + 35] = ((short *)&y2i)[5];       
        // RE 7
        stream0_out[j + 36] = ((short *)&y0r)[6];
        stream0_out[j + 37] = ((short *)&y1r)[6];
        stream0_out[j + 38] = ((short *)&y2r)[6];
        stream0_out[j + 39] = ((short *)&y0i)[6];
        stream0_out[j + 40] = ((short *)&y1i)[6];
        stream0_out[j + 41] = ((short *)&y2i)[6];       
        // RE 8
        stream0_out[j + 42] = ((short *)&y0r)[7];
        stream0_out[j + 43] = ((short *)&y1r)[7];
        stream0_out[j + 44] = ((short *)&y2r)[7];
        stream0_out[j + 45] = ((short *)&y0i)[7];
        stream0_out[j + 46] = ((short *)&y1i)[7];
        stream0_out[j + 47] = ((short *)&y2i)[7];       
  }
  _mm_empty();
  _m_empty();

}


int dlsch_64qam_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                          int **rxdataF_comp,
                          int **rxdataF_comp_i,
                          int **dl_ch_mag,
                          int **dl_ch_mag_i,
                          int **rho_i,
                          short *dlsch_llr,
                          unsigned char symbol,
                          unsigned char first_symbol_flag,
                          unsigned short nb_rb,
                          uint16_t pbch_pss_sss_adjust,
                          short **llr16p) {

    int16_t *rxF      = (int16_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rxF_i    = (int16_t*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag   = (int16_t*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *ch_mag_i = (int16_t*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *rho      = (int16_t*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    int16_t *llr16;
    int len;
    uint8_t symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;

    //first symbol has different structure due to more pilots
    if (first_symbol_flag == 1) {
        llr16 = (int16_t*)dlsch_llr;
    }
    else {
        llr16 = (int16_t*)(*llr16p);
    }
  
    if (!llr16) {
        msg("dlsch_64qam_64qam_llr: llr is null, symbol %d\n",symbol);
        return(-1);
    }

    if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
        // if symbol has pilots
        if (frame_parms->mode1_flag==0)
            // in 2 antenna ports we have 8 REs per symbol per RB
            len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
        else
            // for 1 antenna port we have 10 REs per symbol per RB 
            len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
    else {
        // symbol has no pilots
        len = (nb_rb*12) - pbch_pss_sss_adjust;
    }

    qam64_qam64((short *)rxF,
                (short *)rxF_i,
                (short *)ch_mag,
                (short *)ch_mag_i,
                (short *)llr16,
                (short *)rho,
                len);

    llr16 += (6*len);
    *llr16p = (short *)llr16;
    return(0);
}
