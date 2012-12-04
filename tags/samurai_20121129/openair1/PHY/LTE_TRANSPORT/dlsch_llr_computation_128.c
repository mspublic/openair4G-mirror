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
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"

#ifndef __SSE3__
__m128i zero;
//#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_abs_epi16(xmmx) _mm_add_epi16(_mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx))),_mm_srli_epi16(_mm_cmpgt_epi16(zero,(xmmx)),15))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmy)))
#endif

#ifndef USER_MODE
#define NOCYGWIN_STATIC static
#else
#define NOCYGWIN_STATIC 
#endif

NOCYGWIN_STATIC __m128i rho_rpi_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_1_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_1_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_1_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_1_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_3_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_3_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_3_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_3_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_5_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_5_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_5_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_5_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_7_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_7_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_7_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rpi_7_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_1_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_1_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_1_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_1_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_3_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_3_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_3_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_3_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_5_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_5_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_5_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_5_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_7_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_7_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_7_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i rho_rmi_7_7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i psi_r_m7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m7_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_m1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_r_p7_p7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i psi_i_m7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m7_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_m1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_i_p7_p7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i a_r_m7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m7_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_m1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_r_p7_p7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i a_i_m7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m7_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_m1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_i_p7_p7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i psi_a_m7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m7_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_m1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i psi_a_p7_p7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i a_sq_m7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m7_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_m1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i a_sq_p7_p7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i bit_met_m7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m7_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_m1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p1_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p3_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p5_p7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_m7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_m5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_m3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_m1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_p1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_p3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_p5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i bit_met_p7_p7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  y0_p_1_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_1_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_1_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_1_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_3_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_3_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_3_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_3_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_5_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_5_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_5_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_5_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_7_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_7_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_7_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_p_7_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_1_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_1_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_1_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_1_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_3_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_3_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_3_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_3_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_5_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_5_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_5_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_5_7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_7_1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_7_3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_7_5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0_m_7_7_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  xmm0_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm2_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm3_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm4_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm5_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm6_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm7_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  xmm8_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  y0r_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y0i_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y1r_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y1i_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y2r_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  y2i_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i  logmax_num_re0_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_num_im0_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_den_re0_128i __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m128i  logmax_den_im0_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_num_re1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_num_im1_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i  logmax_den_re1_128i __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m128i  logmax_den_im1_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i tmp_result_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i tmp_result_128i2 __attribute__ ((aligned(16)));

//==============================================================================================
// Auxiliary Functions

// absolute value
#define abs_epi16(x,zero,res,sign) sign = _mm_cmpgt_epi16(zero,x); tmp_result_128i = _mm_xor_si128(x,sign); tmp_result_128i2 = _mm_srli_epi16(sign,15); res = _mm_adds_epi16(tmp_result_128i2,tmp_result_128i);

// calculates psi_a = psi_r*a_r + psi_i*a_i 
#define prodsum_psi_a_epi16(psi_r,a_r,psi_i,a_i,psi_a) tmp_result_128i = _mm_mulhi_epi16(psi_r,a_r); tmp_result_128i = _mm_slli_epi16(tmp_result_128i,1); tmp_result_128i2 = _mm_mulhi_epi16(psi_i,a_i); tmp_result_128i2 = _mm_slli_epi16(tmp_result_128i2,1); psi_a = _mm_adds_epi16(tmp_result_128i,tmp_result_128i2);

// calculates a_sq = int_ch_mag*(a_r^2 + a_i^2)*scale_factor 
#define square_a_epi16(a_r,a_i,int_ch_mag,scale_factor,a_sq) tmp_result_128i = _mm_mulhi_epi16(a_r,a_r); tmp_result_128i = _mm_slli_epi16(tmp_result_128i,1); tmp_result_128i = _mm_mulhi_epi16(tmp_result_128i,scale_factor); tmp_result_128i = _mm_slli_epi16(tmp_result_128i,1); tmp_result_128i = _mm_mulhi_epi16(tmp_result_128i,int_ch_mag); tmp_result_128i = _mm_slli_epi16(tmp_result_128i,1); tmp_result_128i2 = _mm_mulhi_epi16(a_i,a_i); tmp_result_128i2 = _mm_slli_epi16(tmp_result_128i2,1); tmp_result_128i2 = _mm_mulhi_epi16(tmp_result_128i2,scale_factor); tmp_result_128i2 = _mm_slli_epi16(tmp_result_128i2,1); tmp_result_128i2 = _mm_mulhi_epi16(tmp_result_128i2,int_ch_mag); tmp_result_128i2 = _mm_slli_epi16(tmp_result_128i2,1); a_sq = _mm_adds_epi16(tmp_result_128i,tmp_result_128i2);

void interference_abs_epi16(__m128i *psi,
                            __m128i *int_ch_mag, 
                            __m128i *int_mag,
                            __m128i *ONE_OVER_SQRT_10, 
                            __m128i *THREE_OVER_SQRT_10) {

    short *psi_temp = (short *)psi;
    short *int_ch_mag_temp = (short *)int_ch_mag;
    short *int_mag_temp = (short *)int_mag;
    short *ONE_OVER_SQRT_10_temp = (short *)ONE_OVER_SQRT_10;
    short *THREE_OVER_SQRT_10_temp = (short *)THREE_OVER_SQRT_10;
    int jj;
    
    for (jj=0;jj<8;jj++) {
        if (psi_temp[jj] < int_ch_mag_temp[jj])
            int_mag_temp[jj] = ONE_OVER_SQRT_10_temp[jj];
        else
            int_mag_temp[jj] = THREE_OVER_SQRT_10_temp[jj];
    }
    int_mag= (__m128i *) int_mag_temp;
}

//==============================================================================================
// SINGLE-STREAM
//==============================================================================================

//----------------------------------------------------------------------------------------------
// QPSK
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// 16-QAM
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// 64-QAM
//----------------------------------------------------------------------------------------------

//==============================================================================================
// DUAL-STREAM
//==============================================================================================

//----------------------------------------------------------------------------------------------
// QPSK
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// 16-QAM
//----------------------------------------------------------------------------------------------

int dlsch_16qam_16qam_llr_128(LTE_DL_FRAME_PARMS *frame_parms,
                              int **rxdataF_comp,
                              int **rxdataF_comp_i,
                              int **dl_ch_mag,   //|h_0|^2*(2/sqrt{10})
                              int **dl_ch_mag_i, //|h_1|^2*(2/sqrt{10})
                              int **rho_i,
                              short *dlsch_llr,
                              unsigned char symbol,
                              unsigned char first_symbol_flag,
                              unsigned short nb_rb,
                              u16 pbch_pss_sss_adjust,
                              short **llr16p) {

    s16 *rxF      = (s16*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    s16 *rxF_i    = (s16*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    s16 *ch_mag   = (s16*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    s16 *ch_mag_i = (s16*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
    s16 *rho      = (s16*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    s16 *llr16;
    int len;
    u8 symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
  
    // first symbol has different structure due to more pilots
    if (first_symbol_flag == 1) {
        llr16 = (s16*)dlsch_llr;
    }
    else {
        llr16 = (s16*)(*llr16p);
    }
  

    if (!llr16) {
        msg("dlsch_16qam_16qam_llr_128: llr is null, symbol %d\n",symbol);
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
  
    qam16_qam16_128((short *)rxF,
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

NOCYGWIN_STATIC __m128i ONE_OVER_SQRT_10_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i ONE_OVER_SQRT_10_Q15_128i __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i THREE_OVER_SQRT_10_128i __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i ONE_OVER_TWO_SQRT_10_128i __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i SQRT_10_OVER_FOUR_128i __attribute__((aligned(16)));
NOCYGWIN_STATIC __m128i NINE_OVER_TWO_SQRT_10_128i __attribute__((aligned(16)));

NOCYGWIN_STATIC __m128i y0r_over_sqrt10_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i y0i_over_sqrt10_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i y0r_three_over_sqrt10_128i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m128i y0i_three_over_sqrt10_128i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m128i ch_mag_des_128i __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_int_128i __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_over_10_128i __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_over_2_128i __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m128i ch_mag_9_over_10_128i __attribute__ ((aligned(16))); 

void qam16_qam16_128(short *stream0_in,
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

    ONE_OVER_SQRT_10_128i = _mm_set1_epi16(20724); // round(1/sqrt(10)*2^16)
    ONE_OVER_SQRT_10_Q15_128i = _mm_set1_epi16(10362); // round(1/sqrt(10)*2^15)
    THREE_OVER_SQRT_10_128i = _mm_set1_epi16(31086); // round(3/sqrt(10)*2^15)
    SQRT_10_OVER_FOUR_128i = _mm_set1_epi16(25905); // round(sqrt(10)/4*2^15)
    ONE_OVER_TWO_SQRT_10_128i = _mm_set1_epi16(10362); // round(1/2/sqrt(10)*2^16)
    NINE_OVER_TWO_SQRT_10_128i = _mm_set1_epi16(23315); // round(9/2/sqrt(10)*2^14)

    for (i=0;i<length>>2;i+=2) {
        // In one iteration, we deal with 8 REs

        // Get rho
        xmm0_128i = rho01_128i[i];
        xmm1_128i = rho01_128i[i+1];
        xmm0_128i = _mm_shufflelo_epi16(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0_128i = _mm_shufflehi_epi16(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0_128i = _mm_shuffle_epi32(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shufflelo_epi16(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shufflehi_epi16(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shuffle_epi32(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0_128i = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1_128i = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        xmm2_128i = _mm_unpacklo_epi64(xmm0_128i,xmm1_128i); // Re(rho)
        xmm3_128i = _mm_unpackhi_epi64(xmm0_128i,xmm1_128i); // Im(rho)
        rho_rpi_128i = _mm_adds_epi16(xmm2_128i,xmm3_128i); // rho = Re(rho) + Im(rho)
        rho_rmi_128i = _mm_subs_epi16(xmm2_128i,xmm3_128i); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1_128i = _mm_mulhi_epi16(rho_rpi_128i,ONE_OVER_SQRT_10_128i);
        rho_rmi_1_1_128i = _mm_mulhi_epi16(rho_rmi_128i,ONE_OVER_SQRT_10_128i);
        rho_rpi_3_3_128i = _mm_mulhi_epi16(rho_rpi_128i,THREE_OVER_SQRT_10_128i);
        rho_rmi_3_3_128i = _mm_mulhi_epi16(rho_rmi_128i,THREE_OVER_SQRT_10_128i);
        rho_rpi_3_3_128i = _mm_slli_epi16(rho_rpi_3_3_128i,1);
        rho_rmi_3_3_128i = _mm_slli_epi16(rho_rmi_3_3_128i,1);

        xmm4_128i = _mm_mulhi_epi16(xmm2_128i,ONE_OVER_SQRT_10_128i); // Re(rho)
        xmm5_128i = _mm_mulhi_epi16(xmm3_128i,THREE_OVER_SQRT_10_128i); // Im(rho)
        xmm5_128i = _mm_slli_epi16(xmm5_128i,1);

        rho_rpi_1_3_128i = _mm_adds_epi16(xmm4_128i,xmm5_128i);
        rho_rmi_1_3_128i = _mm_subs_epi16(xmm4_128i,xmm5_128i);

        xmm6_128i = _mm_mulhi_epi16(xmm2_128i,THREE_OVER_SQRT_10_128i); // Re(rho)
        xmm7_128i = _mm_mulhi_epi16(xmm3_128i,ONE_OVER_SQRT_10_128i); // Im(rho)
        xmm6_128i = _mm_slli_epi16(xmm6_128i,1);

        rho_rpi_3_1_128i = _mm_adds_epi16(xmm6_128i,xmm7_128i);
        rho_rmi_3_1_128i = _mm_subs_epi16(xmm6_128i,xmm7_128i);

        // Rearrange interfering MF output
        xmm0_128i = stream1_128i_in[i];
        xmm1_128i = stream1_128i_in[i+1];
        xmm0_128i = _mm_shufflelo_epi16(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0_128i = _mm_shufflehi_epi16(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0_128i = _mm_shuffle_epi32(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shufflelo_epi16(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shufflehi_epi16(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shuffle_epi32(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0_128i = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1_128i = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y1r_128i = _mm_unpacklo_epi64(xmm0_128i,xmm1_128i); //[y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i_128i = _mm_unpackhi_epi64(xmm0_128i,xmm1_128i); //[y1i(1),y1i(2),y1i(3),y1i(4)]

        xmm0_128i = _mm_xor_si128(xmm0_128i,xmm0_128i); // ZERO
        xmm2_128i = _mm_subs_epi16(rho_rpi_1_1_128i,y1r_128i); // = [Re(rho)+ Im(rho)]/sqrt(10) - y1r
        abs_epi16(xmm2_128i,xmm0_128i,psi_r_p1_p1_128i,xmm1_128i); // = |[Re(rho)+ Im(rho)]/sqrt(10) - y1r|
 
        xmm2_128i= _mm_subs_epi16(rho_rmi_1_1_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_p1_m1_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rmi_1_1_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_p1_p1_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rpi_1_3_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_p1_p3_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rmi_1_3_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_p1_m3_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rmi_3_1_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_p1_p3_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rpi_3_1_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_p3_p1_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rmi_3_1_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_p3_m1_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rmi_1_3_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_p3_p1_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rpi_3_3_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_p3_p3_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rmi_3_3_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_p3_m3_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rmi_3_3_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_p3_p3_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rpi_1_1_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_m1_p1_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rpi_3_1_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_m1_p3_128i,xmm1_128i);        
        xmm2_128i= _mm_subs_epi16(rho_rpi_1_3_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_m3_p1_128i,xmm1_128i);
        xmm2_128i= _mm_subs_epi16(rho_rpi_3_3_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_m3_p3_128i,xmm1_128i);    
        xmm2_128i= _mm_adds_epi16(rho_rpi_1_1_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_p1_m1_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(rho_rpi_3_1_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_p1_m3_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(rho_rpi_1_3_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_p3_m1_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(rho_rpi_3_3_128i,y1i_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_p3_m3_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(rho_rpi_1_1_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_m1_m1_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(rho_rpi_1_3_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_m1_m3_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(rho_rpi_3_1_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_m3_m1_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(rho_rpi_3_3_128i,y1r_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_m3_m3_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(y1r_128i,rho_rmi_1_1_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_m1_p1_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(y1r_128i,rho_rmi_1_3_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_m1_p3_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(y1i_128i,rho_rmi_1_1_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_m1_m1_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(y1i_128i,rho_rmi_3_1_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_m1_m3_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(y1r_128i,rho_rmi_3_1_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_m3_p1_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(y1r_128i,rho_rmi_3_3_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_r_m3_p3_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(y1i_128i,rho_rmi_1_3_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_m3_m1_128i,xmm1_128i);
        xmm2_128i= _mm_adds_epi16(y1i_128i,rho_rmi_3_3_128i);        abs_epi16(xmm2_128i,xmm0_128i,psi_i_m3_m3_128i,xmm1_128i);

        // Rearrange desired MF output
        xmm0_128i = stream0_128i_in[i];
        xmm1_128i = stream0_128i_in[i+1];
        xmm0_128i = _mm_shufflelo_epi16(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0_128i = _mm_shufflehi_epi16(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm0_128i = _mm_shuffle_epi32(xmm0_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shufflelo_epi16(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shufflehi_epi16(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1_128i = _mm_shuffle_epi32(xmm1_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        //xmm0_128i = [Re(0,1) Re(2,3) Im(0,1) Im(2,3)]
        //xmm1_128i = [Re(4,5) Re(6,7) Im(4,5) Im(6,7)]
        y0r_128i = _mm_unpacklo_epi64(xmm0_128i,xmm1_128i); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i_128i = _mm_unpackhi_epi64(xmm0_128i,xmm1_128i); 
    
        // Rearrange desired channel magnitudes
        xmm2_128i = ch_mag_128i[i]; // = [|h|^2(1),|h|^2(1),|h|^2(2),|h|^2(2)]*(2/sqrt(10))
        xmm3_128i = ch_mag_128i[i+1]; // = [|h|^2(3),|h|^2(3),|h|^2(4),|h|^2(4)]*(2/sqrt(10))
        xmm2_128i = _mm_shufflelo_epi16(xmm2_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2_128i = _mm_shufflehi_epi16(xmm2_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2_128i = _mm_shuffle_epi32(xmm2_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3_128i = _mm_shufflelo_epi16(xmm3_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3_128i = _mm_shufflehi_epi16(xmm3_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3_128i = _mm_shuffle_epi32(xmm3_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_des_128i = _mm_unpacklo_epi64(xmm2_128i,xmm3_128i); // = [|h|^2(1),|h|^2(2),|h|^2(3),|h|^2(4)]*(2/sqrt(10))

        // Rearrange interfering channel magnitudes
        xmm2_128i = ch_mag_128i_i[i];   
        xmm3_128i = ch_mag_128i_i[i+1]; 

        xmm2_128i = _mm_shufflelo_epi16(xmm2_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2_128i = _mm_shufflehi_epi16(xmm2_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2_128i = _mm_shuffle_epi32(xmm2_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3_128i = _mm_shufflelo_epi16(xmm3_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3_128i = _mm_shufflehi_epi16(xmm3_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3_128i = _mm_shuffle_epi32(xmm3_128i,0xd8); //_MM_SHUFFLE(0,2,1,3));

        ch_mag_int_128i  = _mm_unpacklo_epi64(xmm2_128i,xmm3_128i); 

        // Scale MF output of desired signal
        y0r_over_sqrt10_128i = _mm_mulhi_epi16(y0r_128i,ONE_OVER_SQRT_10_128i);
        y0i_over_sqrt10_128i = _mm_mulhi_epi16(y0i_128i,ONE_OVER_SQRT_10_128i);
        y0r_three_over_sqrt10_128i = _mm_mulhi_epi16(y0r_128i,THREE_OVER_SQRT_10_128i);
        y0i_three_over_sqrt10_128i = _mm_mulhi_epi16(y0i_128i,THREE_OVER_SQRT_10_128i);
        y0r_three_over_sqrt10_128i = _mm_slli_epi16(y0r_three_over_sqrt10_128i,1);
        y0i_three_over_sqrt10_128i = _mm_slli_epi16(y0i_three_over_sqrt10_128i,1);

        // Compute necessary combination of required terms
        y0_p_1_1_128i = _mm_adds_epi16(y0r_over_sqrt10_128i,y0i_over_sqrt10_128i);
        y0_m_1_1_128i = _mm_subs_epi16(y0r_over_sqrt10_128i,y0i_over_sqrt10_128i);  

        y0_p_1_3_128i = _mm_adds_epi16(y0r_over_sqrt10_128i,y0i_three_over_sqrt10_128i);
        y0_m_1_3_128i = _mm_subs_epi16(y0r_over_sqrt10_128i,y0i_three_over_sqrt10_128i);

        y0_p_3_1_128i = _mm_adds_epi16(y0r_three_over_sqrt10_128i,y0i_over_sqrt10_128i);
        y0_m_3_1_128i = _mm_subs_epi16(y0r_three_over_sqrt10_128i,y0i_over_sqrt10_128i);

        y0_p_3_3_128i = _mm_adds_epi16(y0r_three_over_sqrt10_128i,y0i_three_over_sqrt10_128i);
        y0_m_3_3_128i = _mm_subs_epi16(y0r_three_over_sqrt10_128i,y0i_three_over_sqrt10_128i);

        // Compute optimal interfering symbol magnitude
        interference_abs_epi16(&psi_r_p1_p1_128i ,&ch_mag_int_128i,&a_r_p1_p1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_p1_p1_128i ,&ch_mag_int_128i,&a_i_p1_p1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_p1_p3_128i ,&ch_mag_int_128i,&a_r_p1_p3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_p1_p3_128i ,&ch_mag_int_128i,&a_i_p1_p3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_p1_m1_128i ,&ch_mag_int_128i,&a_r_p1_m1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_p1_m1_128i ,&ch_mag_int_128i,&a_i_p1_m1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_p1_m3_128i ,&ch_mag_int_128i,&a_r_p1_m3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_p1_m3_128i ,&ch_mag_int_128i,&a_i_p1_m3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_p3_p1_128i ,&ch_mag_int_128i,&a_r_p3_p1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_p3_p1_128i ,&ch_mag_int_128i,&a_i_p3_p1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_p3_p3_128i ,&ch_mag_int_128i,&a_r_p3_p3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_p3_p3_128i ,&ch_mag_int_128i,&a_i_p3_p3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_p3_m1_128i ,&ch_mag_int_128i,&a_r_p3_m1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_p3_m1_128i ,&ch_mag_int_128i,&a_i_p3_m1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_p3_m3_128i ,&ch_mag_int_128i,&a_r_p3_m3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_p3_m3_128i ,&ch_mag_int_128i,&a_i_p3_m3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_m1_p1_128i ,&ch_mag_int_128i,&a_r_m1_p1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_m1_p1_128i ,&ch_mag_int_128i,&a_i_m1_p1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_m1_p3_128i ,&ch_mag_int_128i,&a_r_m1_p3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_m1_p3_128i ,&ch_mag_int_128i,&a_i_m1_p3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_m1_m1_128i ,&ch_mag_int_128i,&a_r_m1_m1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_m1_m1_128i ,&ch_mag_int_128i,&a_i_m1_m1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_m1_m3_128i ,&ch_mag_int_128i,&a_r_m1_m3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_m1_m3_128i ,&ch_mag_int_128i,&a_i_m1_m3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_m3_p1_128i ,&ch_mag_int_128i,&a_r_m3_p1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_m3_p1_128i ,&ch_mag_int_128i,&a_i_m3_p1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_m3_p3_128i ,&ch_mag_int_128i,&a_r_m3_p3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_m3_p3_128i ,&ch_mag_int_128i,&a_i_m3_p3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_m3_m1_128i ,&ch_mag_int_128i,&a_r_m3_m1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_m3_m1_128i ,&ch_mag_int_128i,&a_i_m3_m1_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_r_m3_m3_128i ,&ch_mag_int_128i,&a_r_m3_m3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);
        interference_abs_epi16(&psi_i_m3_m3_128i ,&ch_mag_int_128i,&a_i_m3_m3_128i ,&ONE_OVER_SQRT_10_Q15_128i, &THREE_OVER_SQRT_10_128i);

        // Calculation of groups of two terms in the bit metric involving product of psi and interference magnitude
        prodsum_psi_a_epi16(psi_r_p1_p1_128i,a_r_p1_p1_128i,psi_i_p1_p1_128i,a_i_p1_p1_128i,psi_a_p1_p1_128i);
        prodsum_psi_a_epi16(psi_r_p1_p3_128i,a_r_p1_p3_128i,psi_i_p1_p3_128i,a_i_p1_p3_128i,psi_a_p1_p3_128i);
        prodsum_psi_a_epi16(psi_r_p3_p1_128i,a_r_p3_p1_128i,psi_i_p3_p1_128i,a_i_p3_p1_128i,psi_a_p3_p1_128i);
        prodsum_psi_a_epi16(psi_r_p3_p3_128i,a_r_p3_p3_128i,psi_i_p3_p3_128i,a_i_p3_p3_128i,psi_a_p3_p3_128i);
        prodsum_psi_a_epi16(psi_r_p1_m1_128i,a_r_p1_m1_128i,psi_i_p1_m1_128i,a_i_p1_m1_128i,psi_a_p1_m1_128i);
        prodsum_psi_a_epi16(psi_r_p1_m3_128i,a_r_p1_m3_128i,psi_i_p1_m3_128i,a_i_p1_m3_128i,psi_a_p1_m3_128i);
        prodsum_psi_a_epi16(psi_r_p3_m1_128i,a_r_p3_m1_128i,psi_i_p3_m1_128i,a_i_p3_m1_128i,psi_a_p3_m1_128i);
        prodsum_psi_a_epi16(psi_r_p3_m3_128i,a_r_p3_m3_128i,psi_i_p3_m3_128i,a_i_p3_m3_128i,psi_a_p3_m3_128i);
        prodsum_psi_a_epi16(psi_r_m1_p1_128i,a_r_m1_p1_128i,psi_i_m1_p1_128i,a_i_m1_p1_128i,psi_a_m1_p1_128i);
        prodsum_psi_a_epi16(psi_r_m1_p3_128i,a_r_m1_p3_128i,psi_i_m1_p3_128i,a_i_m1_p3_128i,psi_a_m1_p3_128i);
        prodsum_psi_a_epi16(psi_r_m3_p1_128i,a_r_m3_p1_128i,psi_i_m3_p1_128i,a_i_m3_p1_128i,psi_a_m3_p1_128i);
        prodsum_psi_a_epi16(psi_r_m3_p3_128i,a_r_m3_p3_128i,psi_i_m3_p3_128i,a_i_m3_p3_128i,psi_a_m3_p3_128i);
        prodsum_psi_a_epi16(psi_r_m1_m1_128i,a_r_m1_m1_128i,psi_i_m1_m1_128i,a_i_m1_m1_128i,psi_a_m1_m1_128i);
        prodsum_psi_a_epi16(psi_r_m1_m3_128i,a_r_m1_m3_128i,psi_i_m1_m3_128i,a_i_m1_m3_128i,psi_a_m1_m3_128i);
        prodsum_psi_a_epi16(psi_r_m3_m1_128i,a_r_m3_m1_128i,psi_i_m3_m1_128i,a_i_m3_m1_128i,psi_a_m3_m1_128i);
        prodsum_psi_a_epi16(psi_r_m3_m3_128i,a_r_m3_m3_128i,psi_i_m3_m3_128i,a_i_m3_m3_128i,psi_a_m3_m3_128i);

        
        // squared interference magnitude times int. ch. power
        square_a_epi16(a_r_p1_p1_128i,a_i_p1_p1_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_p1_p1_128i);
        square_a_epi16(a_r_p1_p3_128i,a_i_p1_p3_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_p1_p3_128i);
        square_a_epi16(a_r_p3_p1_128i,a_i_p3_p1_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_p3_p1_128i);
        square_a_epi16(a_r_p3_p3_128i,a_i_p3_p3_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_p3_p3_128i);
        square_a_epi16(a_r_p1_m1_128i,a_i_p1_m1_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_p1_m1_128i);
        square_a_epi16(a_r_p1_m3_128i,a_i_p1_m3_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_p1_m3_128i);
        square_a_epi16(a_r_p3_m1_128i,a_i_p3_m1_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_p3_m1_128i);
        square_a_epi16(a_r_p3_m3_128i,a_i_p3_m3_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_p3_m3_128i);
        square_a_epi16(a_r_m1_p1_128i,a_i_m1_p1_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_m1_p1_128i);
        square_a_epi16(a_r_m1_p3_128i,a_i_m1_p3_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_m1_p3_128i);
        square_a_epi16(a_r_m3_p1_128i,a_i_m3_p1_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_m3_p1_128i);
        square_a_epi16(a_r_m3_p3_128i,a_i_m3_p3_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_m3_p3_128i);
        square_a_epi16(a_r_m1_m1_128i,a_i_m1_m1_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_m1_m1_128i);
        square_a_epi16(a_r_m1_m3_128i,a_i_m1_m3_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_m1_m3_128i);
        square_a_epi16(a_r_m3_m1_128i,a_i_m3_m1_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_m3_m1_128i);
        square_a_epi16(a_r_m3_m3_128i,a_i_m3_m3_128i,ch_mag_int_128i,SQRT_10_OVER_FOUR_128i,a_sq_m3_m3_128i);   

        // Computing different multiples of channel norms
        ch_mag_over_10_128i=_mm_mulhi_epi16(ch_mag_des_128i, ONE_OVER_TWO_SQRT_10_128i);
        ch_mag_over_2_128i=_mm_mulhi_epi16(ch_mag_des_128i, SQRT_10_OVER_FOUR_128i);
        ch_mag_over_2_128i=_mm_slli_epi16(ch_mag_over_2_128i, 1);
        ch_mag_9_over_10_128i=_mm_mulhi_epi16(ch_mag_des_128i, NINE_OVER_TWO_SQRT_10_128i);
        ch_mag_9_over_10_128i=_mm_slli_epi16(ch_mag_9_over_10_128i, 2);                  

        // Computing Metrics
        xmm0_128i = _mm_subs_epi16(psi_a_p1_p1_128i,a_sq_p1_p1_128i);
        xmm1_128i = _mm_adds_epi16(xmm0_128i,y0_p_1_1_128i);
        bit_met_p1_p1_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_10_128i);

        xmm0_128i = _mm_subs_epi16(psi_a_p1_p3_128i,a_sq_p1_p3_128i);
        xmm1_128i = _mm_adds_epi16(xmm0_128i,y0_p_1_3_128i);
        bit_met_p1_p3_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_2_128i);

        xmm0_128i = _mm_subs_epi16(psi_a_p1_m1_128i,a_sq_p1_m1_128i);
        xmm1_128i = _mm_adds_epi16(xmm0_128i,y0_m_1_1_128i);
        bit_met_p1_m1_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_10_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_p1_m3_128i,a_sq_p1_m3_128i);
        xmm1_128i = _mm_adds_epi16(xmm0_128i,y0_m_1_3_128i);
        bit_met_p1_m3_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_2_128i);

        xmm0_128i = _mm_subs_epi16(psi_a_p3_p1_128i,a_sq_p3_p1_128i);
        xmm1_128i = _mm_adds_epi16(xmm0_128i,y0_p_3_1_128i);
        bit_met_p3_p1_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_2_128i);

        xmm0_128i = _mm_subs_epi16(psi_a_p3_p3_128i,a_sq_p3_p3_128i);
        xmm1_128i = _mm_adds_epi16(xmm0_128i,y0_p_3_3_128i);
        bit_met_p3_p3_128i= _mm_subs_epi16(xmm1_128i,ch_mag_9_over_10_128i);

        xmm0_128i = _mm_subs_epi16(psi_a_p3_m1_128i,a_sq_p3_m1_128i);
        xmm1_128i = _mm_adds_epi16(xmm0_128i,y0_m_3_1_128i);
        bit_met_p3_m1_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_2_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_p3_m3_128i,a_sq_p3_m3_128i);
        xmm1_128i = _mm_adds_epi16(xmm0_128i,y0_m_3_3_128i);
        bit_met_p3_m3_128i= _mm_subs_epi16(xmm1_128i,ch_mag_9_over_10_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_m1_p1_128i,a_sq_m1_p1_128i);
        xmm1_128i = _mm_subs_epi16(xmm0_128i,y0_m_1_1_128i);
        bit_met_m1_p1_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_10_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_m1_p3_128i,a_sq_m1_p3_128i);
        xmm1_128i = _mm_subs_epi16(xmm0_128i,y0_m_1_3_128i);
        bit_met_m1_p3_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_2_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_m1_m1_128i,a_sq_m1_m1_128i);
        xmm1_128i = _mm_subs_epi16(xmm0_128i,y0_p_1_1_128i);
        bit_met_m1_m1_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_10_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_m1_m3_128i,a_sq_m1_m3_128i);
        xmm1_128i = _mm_subs_epi16(xmm0_128i,y0_p_1_3_128i);
        bit_met_m1_m3_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_2_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_m3_p1_128i,a_sq_m3_p1_128i);
        xmm1_128i = _mm_subs_epi16(xmm0_128i,y0_m_3_1_128i);
        bit_met_m3_p1_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_2_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_m3_p3_128i,a_sq_m3_p3_128i);
        xmm1_128i = _mm_subs_epi16(xmm0_128i,y0_m_3_3_128i);
        bit_met_m3_p3_128i= _mm_subs_epi16(xmm1_128i,ch_mag_9_over_10_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_m3_m1_128i,a_sq_m3_m1_128i);
        xmm1_128i = _mm_subs_epi16(xmm0_128i,y0_p_3_1_128i);
        bit_met_m3_m1_128i= _mm_subs_epi16(xmm1_128i,ch_mag_over_2_128i);
        
        xmm0_128i = _mm_subs_epi16(psi_a_m3_m3_128i,a_sq_m3_m3_128i);
        xmm1_128i = _mm_subs_epi16(xmm0_128i,y0_p_3_3_128i);
        bit_met_m3_m3_128i= _mm_subs_epi16(xmm1_128i,ch_mag_9_over_10_128i);
        
        // LLR of the first bit
        // Bit = 1
        xmm0_128i = _mm_max_epi16(bit_met_m1_p1_128i,bit_met_m1_p3_128i);
        xmm1_128i = _mm_max_epi16(bit_met_m1_m1_128i,bit_met_m1_m3_128i);
        xmm2_128i = _mm_max_epi16(bit_met_m3_p1_128i,bit_met_m3_p3_128i);
        xmm3_128i = _mm_max_epi16(bit_met_m3_m1_128i,bit_met_m3_m3_128i);
        xmm4_128i = _mm_max_epi16(xmm0_128i,xmm1_128i);
        xmm5_128i = _mm_max_epi16(xmm2_128i,xmm3_128i);
        logmax_num_re0_128i= _mm_max_epi16(xmm4_128i,xmm5_128i);

        // Bit = 0
        xmm0_128i = _mm_max_epi16(bit_met_p1_p1_128i,bit_met_p1_p3_128i);
        xmm1_128i = _mm_max_epi16(bit_met_p1_m1_128i,bit_met_p1_m3_128i);
        xmm2_128i = _mm_max_epi16(bit_met_p3_p1_128i,bit_met_p3_p3_128i);
        xmm3_128i = _mm_max_epi16(bit_met_p3_m1_128i,bit_met_p3_m3_128i);
        xmm4_128i = _mm_max_epi16(xmm0_128i,xmm1_128i);
        xmm5_128i = _mm_max_epi16(xmm2_128i,xmm3_128i);
        logmax_den_re0_128i = _mm_max_epi16(xmm4_128i,xmm5_128i);

        // LLR of first bit [L1(1), L1(2), L1(3), L1(4), L1(5), L1(6), L1(7), L1(8)]
        y0r_128i = _mm_subs_epi16(logmax_den_re0_128i,logmax_num_re0_128i);    

        // LLR of the second bit
        // Bit = 1
        xmm0_128i = _mm_max_epi16(bit_met_p1_m1_128i,bit_met_p3_m1_128i);
        xmm1_128i = _mm_max_epi16(bit_met_m1_m1_128i,bit_met_m3_m1_128i);
        xmm2_128i = _mm_max_epi16(bit_met_p1_m3_128i,bit_met_p3_m3_128i);
        xmm3_128i = _mm_max_epi16(bit_met_m1_m3_128i,bit_met_m3_m3_128i);
        xmm4_128i = _mm_max_epi16(xmm0_128i,xmm1_128i);
        xmm5_128i = _mm_max_epi16(xmm2_128i,xmm3_128i);
        logmax_num_re1_128i = _mm_max_epi16(xmm4_128i,xmm5_128i);
   
        // Bit = 0
        xmm0_128i = _mm_max_epi16(bit_met_p1_p1_128i,bit_met_p3_p1_128i);
        xmm1_128i = _mm_max_epi16(bit_met_m1_p1_128i,bit_met_m3_p1_128i);
        xmm2_128i = _mm_max_epi16(bit_met_p1_p3_128i,bit_met_p3_p3_128i);
        xmm3_128i = _mm_max_epi16(bit_met_m1_p3_128i,bit_met_m3_p3_128i);
        xmm4_128i = _mm_max_epi16(xmm0_128i,xmm1_128i);
        xmm5_128i = _mm_max_epi16(xmm2_128i,xmm3_128i);
        logmax_den_re1_128i = _mm_max_epi16(xmm4_128i,xmm5_128i);

        // LLR of second bit [L2(1), L2(2), L2(3), L2(4)]
        y1r_128i = _mm_subs_epi16(logmax_den_re1_128i,logmax_num_re1_128i);

        // LLR of the third bit
        // Bit = 1
        xmm0_128i = _mm_max_epi16(bit_met_m3_p1_128i,bit_met_m3_p3_128i);
        xmm1_128i = _mm_max_epi16(bit_met_m3_m1_128i,bit_met_m3_m3_128i);
        xmm2_128i = _mm_max_epi16(bit_met_p3_p1_128i,bit_met_p3_p3_128i);
        xmm3_128i = _mm_max_epi16(bit_met_p3_m1_128i,bit_met_p3_m3_128i);
        xmm4_128i = _mm_max_epi16(xmm0_128i,xmm1_128i);
        xmm5_128i = _mm_max_epi16(xmm2_128i,xmm3_128i);
        logmax_num_im0_128i = _mm_max_epi16(xmm4_128i,xmm5_128i);

        // Bit = 0
        xmm0_128i = _mm_max_epi16(bit_met_m1_p1_128i,bit_met_m1_p3_128i);
        xmm1_128i = _mm_max_epi16(bit_met_m1_m1_128i,bit_met_m1_m3_128i);
        xmm2_128i = _mm_max_epi16(bit_met_p1_p1_128i,bit_met_p1_p3_128i);
        xmm3_128i = _mm_max_epi16(bit_met_p1_m1_128i,bit_met_p1_m3_128i);
        xmm4_128i = _mm_max_epi16(xmm0_128i,xmm1_128i);
        xmm5_128i = _mm_max_epi16(xmm2_128i,xmm3_128i);
        logmax_den_im0_128i = _mm_max_epi16(xmm4_128i,xmm5_128i);

        // LLR of third bit [L3(1), L3(2), L3(3), L3(4)]
        y0i_128i = _mm_subs_epi16(logmax_den_im0_128i,logmax_num_im0_128i);

        // LLR of the fourth bit
        // Bit = 1
        xmm0_128i = _mm_max_epi16(bit_met_p1_m3_128i,bit_met_p3_m3_128i);
        xmm1_128i = _mm_max_epi16(bit_met_m1_m3_128i,bit_met_m3_m3_128i);
        xmm2_128i = _mm_max_epi16(bit_met_p1_p3_128i,bit_met_p3_p3_128i);
        xmm3_128i = _mm_max_epi16(bit_met_m1_p3_128i,bit_met_m3_p3_128i);
        xmm4_128i = _mm_max_epi16(xmm0_128i,xmm1_128i);
        xmm5_128i = _mm_max_epi16(xmm2_128i,xmm3_128i);
        logmax_num_im1_128i = _mm_max_epi16(xmm4_128i,xmm5_128i);

        // Bit = 0
        xmm0_128i = _mm_max_epi16(bit_met_p1_m1_128i,bit_met_p3_m1_128i);
        xmm1_128i = _mm_max_epi16(bit_met_m1_m1_128i,bit_met_m3_m1_128i);
        xmm2_128i = _mm_max_epi16(bit_met_p1_p1_128i,bit_met_p3_p1_128i);
        xmm3_128i = _mm_max_epi16(bit_met_m1_p1_128i,bit_met_m3_p1_128i);
        xmm4_128i = _mm_max_epi16(xmm0_128i,xmm1_128i);
        xmm5_128i = _mm_max_epi16(xmm2_128i,xmm3_128i);
        logmax_den_im1_128i = _mm_max_epi16(xmm4_128i,xmm5_128i);

        // LLR of fourth bit [L4(1), L4(2), L4(3), L4(4)]
        y1i_128i = _mm_subs_epi16(logmax_den_im1_128i,logmax_num_im1_128i);

        // Pack LLRs in output
        // [L1(1), L2(1), L1(2), L2(2), L1(3), L2(3), L1(4), L2(4)]
        xmm0_128i = _mm_unpacklo_epi16(y0r_128i,y1r_128i);
        // [L1(5), L2(5), L1(6), L2(6), L1(7), L2(7), L1(8), L2(8)]
        xmm1_128i = _mm_unpackhi_epi16(y0r_128i,y1r_128i);
        // [L3(1), L4(1), L3(2), L4(2), L3(3), L4(3), L3(4), L4(4)]
        xmm2_128i = _mm_unpacklo_epi16(y0i_128i,y1i_128i);
        // [L3(5), L4(5), L3(6), L4(6), L3(7), L4(7), L3(8), L4(8)]
        xmm3_128i = _mm_unpackhi_epi16(y0i_128i,y1i_128i);

        stream0_128i_out[2*i+0] = _mm_unpacklo_epi32(xmm0_128i,xmm2_128i); // 8LLRs, 2REs
        stream0_128i_out[2*i+1] = _mm_unpackhi_epi32(xmm0_128i,xmm2_128i);
        stream0_128i_out[2*i+2] = _mm_unpacklo_epi32(xmm1_128i,xmm3_128i);
        stream0_128i_out[2*i+3] = _mm_unpackhi_epi32(xmm1_128i,xmm3_128i);
    }
    _mm_empty();
    _m_empty();
}


//----------------------------------------------------------------------------------------------
// 64-QAM
//----------------------------------------------------------------------------------------------
