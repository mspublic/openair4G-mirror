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

extern __m128i mmtmpD0,mmtmpD1,mmtmpD2,mmtmpD3; // declared in dlsch_demodulation.c

NOCYGWIN_STATIC __m64 rho_rpi __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_1_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_1_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_1_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_1_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_3_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_3_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_3_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_3_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_5_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_5_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_5_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_5_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_7_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_7_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_7_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rpi_7_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_1_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_1_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_1_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_1_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_3_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_3_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_3_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_3_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_5_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_5_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_5_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_5_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_7_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_7_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_7_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 rho_rmi_7_7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 psi_r_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_r_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 psi_i_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_i_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 a_r_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_r_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 a_i_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_i_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 psi_a_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 psi_a_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 a_sq_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 a_sq_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 bit_met_m7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m7_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_m1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p1_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p1_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p1_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p1_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p1_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p1_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p1_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p1_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p3_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p3_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p3_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p3_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p3_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p3_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p3_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p3_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p5_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p5_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p5_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p5_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p5_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p5_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p5_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p5_p7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p7_m7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p7_m5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p7_m3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p7_m1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p7_p1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p7_p3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p7_p5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 bit_met_p7_p7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64  y0_p_1_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_1_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_1_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_1_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_3_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_3_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_3_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_3_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_5_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_5_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_5_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_5_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_7_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_7_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_7_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_p_7_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_1_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_1_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_1_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_1_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_3_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_3_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_3_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_3_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_5_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_5_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_5_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_5_7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_7_1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_7_3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_7_5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0_m_7_7 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64  xmm0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm4 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm5 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm6 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm7 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm8 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64  y0r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y2r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y2i __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64  logmax_num_re0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_im0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_den_re0 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m64  logmax_den_im0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_re1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_im1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_den_re1 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m64  logmax_den_im1 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 tmp_result __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 tmp_result2 __attribute__ ((aligned(16)));

//==============================================================================================
// Auxiliary Functions

// absolute value
#define abs_pi16(x,zero,res,sign) sign = _mm_cmpgt_pi16(zero,x); tmp_result = _mm_xor_si64(x,sign); tmp_result2 = _mm_srli_pi16(sign,15); res = _mm_adds_pi16(tmp_result2,tmp_result);

// calculates psi_a = psi_r*a_r + psi_i*a_i 
#define prodsum_psi_a_pi16(psi_r,a_r,psi_i,a_i,psi_a) tmp_result = _mm_mulhi_pi16(psi_r,a_r); tmp_result = _mm_slli_pi16(tmp_result,1); tmp_result2 = _mm_mulhi_pi16(psi_i,a_i); tmp_result2 = _mm_slli_pi16(tmp_result2,1); psi_a = _mm_adds_pi16(tmp_result,tmp_result2);

// calculates a_sq = int_ch_mag*(a_r^2 + a_i^2)*scale_factor 
#define square_a_pi16(a_r,a_i,int_ch_mag,scale_factor,a_sq) tmp_result = _mm_mulhi_pi16(a_r,a_r); tmp_result = _mm_slli_pi16(tmp_result,1); tmp_result = _mm_mulhi_pi16(tmp_result,scale_factor); tmp_result = _mm_slli_pi16(tmp_result,1); tmp_result = _mm_mulhi_pi16(tmp_result,int_ch_mag); tmp_result = _mm_slli_pi16(tmp_result,1); tmp_result2 = _mm_mulhi_pi16(a_i,a_i); tmp_result2 = _mm_slli_pi16(tmp_result2,1); tmp_result2 = _mm_mulhi_pi16(tmp_result2,scale_factor); tmp_result2 = _mm_slli_pi16(tmp_result2,1); tmp_result2 = _mm_mulhi_pi16(tmp_result2,int_ch_mag); tmp_result2 = _mm_slli_pi16(tmp_result2,1); a_sq = _mm_adds_pi16(tmp_result,tmp_result2);

// calculates a_sq = int_ch_mag*(a_r^2 + a_i^2)*scale_factor for 64-QAM
#define square_a_64qam_pi16(a_r,a_i,int_ch_mag,scale_factor,a_sq)  tmp_result = _mm_mulhi_pi16(a_r,a_r); tmp_result = _mm_slli_pi16(tmp_result,1); tmp_result = _mm_mulhi_pi16(tmp_result,scale_factor); tmp_result = _mm_slli_pi16(tmp_result,3); tmp_result = _mm_mulhi_pi16(tmp_result,int_ch_mag); tmp_result = _mm_slli_pi16(tmp_result,1); tmp_result2 = _mm_mulhi_pi16(a_i,a_i); tmp_result2 = _mm_slli_pi16(tmp_result2,1); tmp_result2 = _mm_mulhi_pi16(tmp_result2,scale_factor); tmp_result2 = _mm_slli_pi16(tmp_result2,3); tmp_result2 = _mm_mulhi_pi16(tmp_result2,int_ch_mag); tmp_result2 = _mm_slli_pi16(tmp_result2,1); a_sq = _mm_adds_pi16(tmp_result,tmp_result2);



void interference_abs_pi16(__m64 *psi,
                           __m64 *int_ch_mag, 
                           __m64 *int_mag,
                           __m64 *ONE_OVER_SQRT_10, 
                           __m64 *THREE_OVER_SQRT_10) {

    short *psi_temp = (short *)psi;
    short *int_ch_mag_temp = (short *)int_ch_mag;
    short *int_mag_temp = (short *)int_mag;
    short *ONE_OVER_SQRT_10_temp = (short *)ONE_OVER_SQRT_10;
    short *THREE_OVER_SQRT_10_temp = (short *)THREE_OVER_SQRT_10;
    int jj;
    
    for (jj=0;jj<4;jj++) {
        if (psi_temp[jj] < int_ch_mag_temp[jj])
            int_mag_temp[jj] = ONE_OVER_SQRT_10_temp[jj];
        else
            int_mag_temp[jj] = THREE_OVER_SQRT_10_temp[jj];
    }
    int_mag= (__m64 *) int_mag_temp;
}



void interference_abs_64qam_pi16(__m64 *psi, 
                                 __m64 *int_ch_mag, 
                                 __m64 *int_two_ch_mag, 
                                 __m64 *int_three_ch_mag, 
                                 __m64 *a, 
                                 __m64 *ONE_OVER_SQRT_2_42, 
                                 __m64 *THREE_OVER_SQRT_2_42, 
                                 __m64 *FIVE_OVER_SQRT_2_42, 
                                 __m64 *SEVEN_OVER_SQRT_2_42) {
	
	short *psi_temp = (short *)psi;
	short *int_ch_mag_temp = (short *)int_ch_mag;
	short *int_two_ch_mag_temp = (short *)int_two_ch_mag;
	short *int_three_ch_mag_temp = (short *)int_three_ch_mag;
	short *a_temp = (short *)a;
	short *ONE_OVER_SQRT_2_42_temp   = (short *)ONE_OVER_SQRT_2_42;
	short *THREE_OVER_SQRT_2_42_temp = (short *)THREE_OVER_SQRT_2_42;
	short *FIVE_OVER_SQRT_2_42_temp  = (short *)FIVE_OVER_SQRT_2_42;
	short *SEVEN_OVER_SQRT_2_42_temp = (short *)SEVEN_OVER_SQRT_2_42;
	int jj;
	
	for (jj=0; jj<4; jj++) {
		if (psi_temp[jj] < int_two_ch_mag_temp[jj]) { // 3 or 1
			if (psi_temp[jj] < int_ch_mag_temp[jj]) { // 1
				a_temp[jj] = ONE_OVER_SQRT_2_42_temp[jj];
			}
			else { // 3
				a_temp[jj] = THREE_OVER_SQRT_2_42_temp[jj];
			}
		}
		else { // 5 or 7
			if ( psi_temp[jj] < int_three_ch_mag_temp[jj] ) { // 5
				a_temp[jj] = FIVE_OVER_SQRT_2_42_temp[jj];
			}
			else { // 7
				a_temp[jj] = SEVEN_OVER_SQRT_2_42_temp[jj];
			}
		}
	}
	a= (__m64 *) a_temp;
}


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
                   u8 first_symbol_flag,
                   u16 nb_rb,
                   u16 pbch_pss_sss_adjust,
                   short **llr32p) {

  u32 *rxF = (u32*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  u32 *llr32;
  int i,len;
  u8 symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;

  if (first_symbol_flag==1) {
      llr32 = (u32*)dlsch_llr;
  }
  else {
      llr32 = (u32*)(*llr32p);
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
                     u8 first_symbol_flag,
                     unsigned short nb_rb,
                     u16 pbch_pss_sss_adjust,
                     s16 **llr32p) {

    __m128i *rxF = (__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    __m128i *ch_mag;
    __m128i llr128[2];
    int i,len;
    unsigned char symbol_mod,len_mod4=0;
    u32 *llr32;
    
    if (first_symbol_flag==1) {
        llr32 = (u32*)dlsch_llr;
    }
    else {
        llr32 = (u32*)*llr32p;
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
        
        mmtmpD0 = _mm_abs_epi16(rxF[i]);
        mmtmpD0 = _mm_subs_epi16(ch_mag[i],mmtmpD0);

        // lambda_1=y_R, lambda_2=|y_R|-|h|^2, lamda_3=y_I, lambda_4=|y_I|-|h|^2
        llr128[0] = _mm_unpacklo_epi32(rxF[i],mmtmpD0); 
        llr128[1] = _mm_unpackhi_epi32(rxF[i],mmtmpD0);
        llr32[0] = ((u32 *)&llr128[0])[0];
        llr32[1] = ((u32 *)&llr128[0])[1];
        llr32[2] = ((u32 *)&llr128[0])[2];
        llr32[3] = ((u32 *)&llr128[0])[3];
        llr32[4] = ((u32 *)&llr128[1])[0];
        llr32[5] = ((u32 *)&llr128[1])[1];
        llr32[6] = ((u32 *)&llr128[1])[2];
        llr32[7] = ((u32 *)&llr128[1])[3];
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
                     u8 first_symbol_flag,
                     unsigned short nb_rb,
                     u16 pbch_pss_sss_adjust,
                     short **llr_save) {

    __m128i *rxF = (__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    __m128i *ch_mag,*ch_magb;
    int j=0,i,len,len2;
    unsigned char symbol_mod,len_mod4;
    short *llr;
    s16 *llr2;
    
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

    for (i=0;i<len;i++) {
        
        mmtmpD1 = _mm_abs_epi16(rxF[i]);
        mmtmpD1  = _mm_subs_epi16(ch_mag[i],mmtmpD1);
        mmtmpD2 = _mm_abs_epi16(mmtmpD1);
        mmtmpD2 = _mm_subs_epi16(ch_magb[i],mmtmpD2);
        
        // loop over all LLRs in quad word (24 coded bits)
        for (j=0;j<8;j+=2) {
            llr2[0] = ((short *)&rxF[i])[j];
            llr2[1] = ((short *)&rxF[i])[j+1];
            llr2[2] = ((short *)&mmtmpD1)[j];
            llr2[3] = ((short *)&mmtmpD1)[j+1];
            llr2[4] = ((short *)&mmtmpD2)[j];
            llr2[5] = ((short *)&mmtmpD2)[j+1];
            
            llr2+=6;
        }
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

NOCYGWIN_STATIC __m64  y0r_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0i_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1r_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1i_over2 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64  A __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  B __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  C __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  D __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  E __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  F __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  G __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  H __attribute__ ((aligned(16))); 


int dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
                        int **rxdataF_comp,
                        int **rxdataF_comp_i,
                        int **rho_i,
                        short *dlsch_llr,
                        unsigned char symbol,
                        unsigned char first_symbol_flag,
                        unsigned short nb_rb,
                        u16 pbch_pss_sss_adjust,
                        short **llr128p) {
    
    __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    __m128i *rxF_i=(__m128i*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
    __m128i *rho=(__m128i*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
    __m128i *llr128;
    int len;
    u8 symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
    
    if (first_symbol_flag == 1) {
        llr128 = (__m128i*)dlsch_llr;
    }
    else {
        llr128 = (__m128i*)(*llr128p);
    }
    
    if (!llr128) {
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
    
    qpsk_qpsk((short *)rxF,
              (short *)rxF_i,
              (short *)llr128,
              (short *)rho,
              len);
    
    llr128 += (len>>2);
    *llr128p = (short *)llr128;
    
    return(0);
}

NOCYGWIN_STATIC __m64 ONE_OVER_SQRT_8 __attribute__((aligned(16))); 

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

    __m64 *rho01_64 = (__m64 *)rho01;
    __m64 *stream0_64_in = (__m64 *)stream0_in;
    __m64 *stream1_64_in = (__m64 *)stream1_in;
    __m64 *stream0_64_out = (__m64 *)stream0_out;

#ifdef DEBUG_LLR
    print_shorts2("rho01_64:\n",rho01_64);
#endif

    int i;

    ((short*)&ONE_OVER_SQRT_8)[0] = 23170;  //round(2^16/sqrt(8))
    ((short*)&ONE_OVER_SQRT_8)[1] = 23170;
    ((short*)&ONE_OVER_SQRT_8)[2] = 23170;
    ((short*)&ONE_OVER_SQRT_8)[3] = 23170;

    for (i=0;i<length>>1;i+=2) {
        // in each iteration, we take 4 complex samples or 4 real and 4 imag samples

        xmm0 = rho01_64[i]; // 2 symbols, i.e. 2 real and 2 imag parts
        xmm1 = rho01_64[i+1]; 
    
        // put (rho_r + rho_i)/2sqrt2 in rho_rpi
        // put (rho_r - rho_i)/2sqrt2 in rho_rmi
     
        xmm0 = _mm_shuffle_pi16(xmm0,0xd8); // _MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_pi16(xmm1,0xd8); // _MM_SHUFFLE(0,2,1,3));
        
        xmm2 = _mm_unpacklo_pi32(xmm0,xmm1);
        xmm3 = _mm_unpackhi_pi32(xmm0,xmm1);
    
        rho_rpi = _mm_adds_pi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_pi16(xmm2,xmm3); // rho*= Re(rho) - Im(rho)
    
        // divide by sqrt(8), no shift needed ONE_OVER_SQRT_8 =Q1.16
        rho_rpi = _mm_mulhi_pi16(rho_rpi,ONE_OVER_SQRT_8);
        rho_rmi = _mm_mulhi_pi16(rho_rmi,ONE_OVER_SQRT_8);

        // Compute LLR for first bit of stream 0
        
        // Compute real and imaginary parts of MF output for stream 0
        xmm0 = stream0_64_in[i];
        xmm1 = stream0_64_in[i+1];

        xmm0 = _mm_shuffle_pi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_pi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));

        y0r  = _mm_unpacklo_pi32(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0r_over2  = _mm_srai_pi16(y0r,1);   // divide by 2                    
        y0i  = _mm_unpackhi_pi32(xmm0,xmm1); // = [y0i(1),y0i(2),y0i(3),y0i(4)]
        y0i_over2  = _mm_srai_pi16(y0i,1);   // divide by 2 
    
        // Compute real and imaginary parts of MF output for stream 1
        xmm0 = stream1_64_in[i];
        xmm1 = stream1_64_in[i+1];
    
        xmm0 = _mm_shuffle_pi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_pi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));

        y1r  = _mm_unpacklo_pi32(xmm0,xmm1); // = [y1r(1),y1r(2),y1r(3),y1r(4)]
        y1r_over2  = _mm_srai_pi16(y1r,1);   // divide by 2                    
        y1i  = _mm_unpackhi_pi32(xmm0,xmm1); // = [y1i(1),y1i(2),y1i(3),y1i(4)]
        y1i_over2  = _mm_srai_pi16(y1i,1);   // divide by 2
    
        // Compute the terms for the LLR of first bit
    
        xmm0 = _mm_xor_si64(xmm0,xmm0); // ZERO
    
        // 1 term for nominator of LLR
        xmm3 = _mm_subs_pi16(y1r_over2,rho_rpi);
        abs_pi16(xmm3,xmm0,A,xmm1); // A = |y1r/2 - rho/sqrt(8)|
        xmm2 = _mm_adds_pi16(A,y0i_over2); // = |y1r/2 - rho/sqrt(8)| + y0i/2
        xmm3 = _mm_subs_pi16(y1i_over2,rho_rmi); 
        abs_pi16(xmm3,xmm0,B,xmm1); // B = |y1i/2 - rho*/sqrt(8)|
        logmax_num_re0 = _mm_adds_pi16(B,xmm2); // = |y1r/2 - rho/sqrt(8)|+|y1i/2 - rho*/sqrt(8)| + y0i/2

        // 2 term for nominator of LLR        
        xmm3 = _mm_subs_pi16(y1r_over2,rho_rmi); 
        abs_pi16(xmm3,xmm0,C,xmm1); // C = |y1r/2 - rho*/4|
        xmm2 = _mm_subs_pi16(C,y0i_over2); // = |y1r/2 - rho*/4| - y0i/2
        xmm3 = _mm_adds_pi16(y1i_over2,rho_rpi); 
        abs_pi16(xmm3,xmm0,D,xmm1); // D = |y1i/2 + rho/4|      
        xmm2 = _mm_adds_pi16(xmm2,D); // |y1r/2 - rho*/4| + |y1i/2 + rho/4| - y0i/2
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0,xmm2); // max, numerator done

        // 1 term for denominator of LLR
        xmm3 = _mm_adds_pi16(y1r_over2,rho_rmi); 
        abs_pi16(xmm3,xmm0,E,xmm1); // E = |y1r/2 + rho*/4|
        xmm2 = _mm_adds_pi16(E,y0i_over2); // = |y1r/2 + rho*/4| + y0i/2
        xmm3 = _mm_subs_pi16(y1i_over2,rho_rpi); 
        abs_pi16(xmm3,xmm0,F,xmm1); // F = |y1i/2 - rho/4|
        logmax_den_re0 = _mm_adds_pi16(F,xmm2); // = |y1r/2 + rho*/4| + |y1i/2 - rho/4| + y0i/2

        // 2 term for denominator of LLR
        xmm3 = _mm_adds_pi16(y1r_over2,rho_rpi); 
        abs_pi16(xmm3,xmm0,G,xmm1); // G = |y1r/2 + rho/4|      
        xmm2 = _mm_subs_pi16(G,y0i_over2); // = |y1r/2 + rho/4| - y0i/2
        xmm3 = _mm_adds_pi16(y1i_over2,rho_rmi); 
        abs_pi16(xmm3,xmm0,H,xmm1); // H = |y1i/2 + rho*/4|
        xmm2 = _mm_adds_pi16(xmm2,H); // = |y1r/2 + rho/4| + |y1i/2 + rho*/4| - y0i/2    
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0,xmm2); // max, denominator done

        // Compute the terms for the LLR of first bit

        // 1 term for nominator of LLR    
        xmm2 = _mm_adds_pi16(A,y0r_over2); 
        logmax_num_im0 = _mm_adds_pi16(B,xmm2); // = |y1r/2 - rho/4| + |y1i/2 - rho*/4| + y0r/2
 
        // 2 term for nominator of LLR
        xmm2 = _mm_subs_pi16(E,y0r_over2); 
        xmm2 = _mm_adds_pi16(xmm2,F); // = |y1r/2 + rho*/4| + |y1i/2 - rho/4| - y0r/2

        logmax_num_im0 = _mm_max_pi16(logmax_num_im0,xmm2); // max, nominator done
    
        // 1 term for denominator of LLR
        xmm2 = _mm_adds_pi16(C,y0r_over2); 
        logmax_den_im0 = _mm_adds_pi16(D,xmm2); // = |y1r/2 - rho*/4| + |y1i/2 + rho/4| - y0r/2

        xmm2 = _mm_subs_pi16(G,y0r_over2); 
        xmm2 = _mm_adds_pi16(xmm2,H); // = |y1r/2 + rho/4| + |y1i/2 + rho*/4| - y0r/2
    
        logmax_den_im0 = _mm_max_pi16(logmax_den_im0,xmm2); // max, denominator done

        // LLR of first bit [L1(1), L1(2), L1(3), L1(4)]
        y0r = _mm_adds_pi16(y0r,logmax_num_re0);
        y0r = _mm_subs_pi16(y0r,logmax_den_re0);
    
        // LLR of second bit [L2(1), L2(2), L2(3), L2(4)]
        y0i = _mm_adds_pi16(y0i,logmax_num_im0);
        y0i = _mm_subs_pi16(y0i,logmax_den_im0);

        stream0_64_out[i] = _mm_unpacklo_pi16(y0r,y0i); // = [L1(1), L2(1), L1(2), L2(2)]
        if (i<((length>>1) - 1)) // false if only 2 REs remain
            stream0_64_out[i+1] = _mm_unpackhi_pi16(y0r,y0i);
   
    }
    _mm_empty();
    _m_empty();
}

//----------------------------------------------------------------------------------------------
// 16-QAM
//----------------------------------------------------------------------------------------------

NOCYGWIN_STATIC __m64 ONE_OVER_SQRT_10 __attribute__((aligned(16)));
NOCYGWIN_STATIC __m64 ONE_OVER_SQRT_10_Q15 __attribute__((aligned(16)));
NOCYGWIN_STATIC __m64 THREE_OVER_SQRT_10 __attribute__((aligned(16)));
NOCYGWIN_STATIC __m64 ONE_OVER_TWO_SQRT_10 __attribute__((aligned(16)));
NOCYGWIN_STATIC __m64 SQRT_10_OVER_FOUR __attribute__((aligned(16)));
NOCYGWIN_STATIC __m64 NINE_OVER_TWO_SQRT_10 __attribute__((aligned(16)));

NOCYGWIN_STATIC __m64  y0r_over_sqrt10 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0i_over_sqrt10 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0r_three_over_sqrt10 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0i_three_over_sqrt10 __attribute__ ((aligned(16)));

NOCYGWIN_STATIC __m64 ch_mag_des __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_int __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_over_10 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_over_2 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_9_over_10 __attribute__ ((aligned(16))); 

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

    __m64 *rho01_64       = (__m64 *)rho01;
    __m64 *stream0_64_in  = (__m64 *)stream0_in;
    __m64 *stream1_64_in  = (__m64 *)stream1_in;
    __m64 *stream0_64_out = (__m64 *)stream0_out;
    __m64 *ch_mag_64      = (__m64 *)ch_mag;
    __m64 *ch_mag_64_i    = (__m64 *)ch_mag_i;

    int i;
    ((short*)&ONE_OVER_SQRT_10)[0] = 20724; // round(1/sqrt(10)*2^16)
    ((short*)&ONE_OVER_SQRT_10)[1] = 20724;
    ((short*)&ONE_OVER_SQRT_10)[2] = 20724;
    ((short*)&ONE_OVER_SQRT_10)[3] = 20724;
    
    ((short*)&ONE_OVER_SQRT_10_Q15)[0] = 10362; // round(1/sqrt(10)*2^15)
    ((short*)&ONE_OVER_SQRT_10_Q15)[1] = 10362;
    ((short*)&ONE_OVER_SQRT_10_Q15)[2] = 10362;
    ((short*)&ONE_OVER_SQRT_10_Q15)[3] = 10362;

    ((short*)&THREE_OVER_SQRT_10)[0] = 31086; // round(3/sqrt(10)*2^15)
    ((short*)&THREE_OVER_SQRT_10)[1] = 31086;
    ((short*)&THREE_OVER_SQRT_10)[2] = 31086;
    ((short*)&THREE_OVER_SQRT_10)[3] = 31086; 

    ((short*)&SQRT_10_OVER_FOUR)[0] = 25905; // round(sqrt(10)/4*2^15)
    ((short*)&SQRT_10_OVER_FOUR)[1] = 25905;
    ((short*)&SQRT_10_OVER_FOUR)[2] = 25905;
    ((short*)&SQRT_10_OVER_FOUR)[3] = 25905; 

    ((short*)&ONE_OVER_TWO_SQRT_10)[0] = 10362; // round(1/2/sqrt(10)*2^16)
    ((short*)&ONE_OVER_TWO_SQRT_10)[1] = 10362;
    ((short*)&ONE_OVER_TWO_SQRT_10)[2] = 10362;
    ((short*)&ONE_OVER_TWO_SQRT_10)[3] = 10362; 

    ((short*)&NINE_OVER_TWO_SQRT_10)[0] = 23315; // round(9/2/sqrt(10)*2^14)
    ((short*)&NINE_OVER_TWO_SQRT_10)[1] = 23315;
    ((short*)&NINE_OVER_TWO_SQRT_10)[2] = 23315;
    ((short*)&NINE_OVER_TWO_SQRT_10)[3] = 23315; 

    for (i=0;i<length>>1;i+=2) {
        // In one iteration, we deal with 4 complex samples or 8 real samples

        // Get rho
        xmm0 = rho01_64[i];
        xmm1 = rho01_64[i+1];
        xmm0 = _mm_shuffle_pi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_pi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm2 = _mm_unpacklo_pi32(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_pi32(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_pi16(xmm2,xmm3); // rho = Re(rho) + Im(rho)
        rho_rmi = _mm_subs_pi16(xmm2,xmm3); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1 = _mm_mulhi_pi16(rho_rpi,ONE_OVER_SQRT_10);
        rho_rmi_1_1 = _mm_mulhi_pi16(rho_rmi,ONE_OVER_SQRT_10);

        rho_rpi_3_3 = _mm_mulhi_pi16(rho_rpi,THREE_OVER_SQRT_10);
        rho_rmi_3_3 = _mm_mulhi_pi16(rho_rmi,THREE_OVER_SQRT_10);
        rho_rpi_3_3 = _mm_slli_pi16(rho_rpi_3_3,1);
        rho_rmi_3_3 = _mm_slli_pi16(rho_rmi_3_3,1);

        xmm4 = _mm_mulhi_pi16(xmm2,ONE_OVER_SQRT_10); // Re(rho)
        xmm5 = _mm_mulhi_pi16(xmm3,THREE_OVER_SQRT_10); // Im(rho)
        xmm5 = _mm_slli_pi16(xmm5,1);

        rho_rpi_1_3 = _mm_adds_pi16(xmm4,xmm5);
        rho_rmi_1_3 = _mm_subs_pi16(xmm4,xmm5);

        xmm6 = _mm_mulhi_pi16(xmm2,THREE_OVER_SQRT_10); // Re(rho)
        xmm7 = _mm_mulhi_pi16(xmm3,ONE_OVER_SQRT_10); // Im(rho)
        xmm6 = _mm_slli_pi16(xmm6,1);

        rho_rpi_3_1 = _mm_adds_pi16(xmm6,xmm7);
        rho_rmi_3_1 = _mm_subs_pi16(xmm6,xmm7);

        // Rearrange interfering MF output
        xmm0 = stream1_64_in[i];
        xmm1 = stream1_64_in[i+1];           
        xmm0 = _mm_shuffle_pi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_pi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        y1r  = _mm_unpacklo_pi32(xmm0,xmm1); // = [y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i  = _mm_unpackhi_pi32(xmm0,xmm1); // = [y1i(1),y1i(2),y1i(3),y1i(4)]

        xmm0 = _mm_xor_si64(xmm0,xmm0); // ZERO
        xmm2 = _mm_subs_pi16(rho_rpi_1_1,y1r); // = [Re(rho)+ Im(rho)]/sqrt(10) - y1r
        abs_pi16(xmm2,xmm0,psi_r_p1_p1,xmm1); // = |[Re(rho)+ Im(rho)]/sqrt(10) - y1r|

        xmm2= _mm_subs_pi16(rho_rmi_1_1,y1r);
        abs_pi16(xmm2,xmm0,psi_r_p1_m1,xmm1);
        xmm2= _mm_subs_pi16(rho_rmi_1_1,y1i);
        abs_pi16(xmm2,xmm0,psi_i_p1_p1,xmm1);
        xmm2= _mm_subs_pi16(rho_rpi_1_3,y1r);
        abs_pi16(xmm2,xmm0,psi_r_p1_p3,xmm1);
        xmm2= _mm_subs_pi16(rho_rmi_1_3,y1r);
        abs_pi16(xmm2,xmm0,psi_r_p1_m3,xmm1);
        xmm2= _mm_subs_pi16(rho_rmi_3_1,y1i);
        abs_pi16(xmm2,xmm0,psi_i_p1_p3,xmm1);
        xmm2= _mm_subs_pi16(rho_rpi_3_1,y1r);
        abs_pi16(xmm2,xmm0,psi_r_p3_p1,xmm1);
        xmm2= _mm_subs_pi16(rho_rmi_3_1,y1r);
        abs_pi16(xmm2,xmm0,psi_r_p3_m1,xmm1);
        xmm2= _mm_subs_pi16(rho_rmi_1_3,y1i);
        abs_pi16(xmm2,xmm0,psi_i_p3_p1,xmm1);
        xmm2= _mm_subs_pi16(rho_rpi_3_3,y1r);
        abs_pi16(xmm2,xmm0,psi_r_p3_p3,xmm1);
        xmm2= _mm_subs_pi16(rho_rmi_3_3,y1r);
        abs_pi16(xmm2,xmm0,psi_r_p3_m3,xmm1);
        xmm2= _mm_subs_pi16(rho_rmi_3_3,y1i);
        abs_pi16(xmm2,xmm0,psi_i_p3_p3,xmm1);
        xmm2= _mm_subs_pi16(rho_rpi_1_1,y1i);
        abs_pi16(xmm2,xmm0,psi_i_m1_p1,xmm1);
        xmm2= _mm_subs_pi16(rho_rpi_3_1,y1i);
        abs_pi16(xmm2,xmm0,psi_i_m1_p3,xmm1);        
        xmm2= _mm_subs_pi16(rho_rpi_1_3,y1i);
        abs_pi16(xmm2,xmm0,psi_i_m3_p1,xmm1);
        xmm2= _mm_subs_pi16(rho_rpi_3_3,y1i);
        abs_pi16(xmm2,xmm0,psi_i_m3_p3,xmm1);    
        xmm2= _mm_adds_pi16(rho_rpi_1_1,y1i);
        abs_pi16(xmm2,xmm0,psi_i_p1_m1,xmm1);
        xmm2= _mm_adds_pi16(rho_rpi_3_1,y1i);
        abs_pi16(xmm2,xmm0,psi_i_p1_m3,xmm1);
        xmm2= _mm_adds_pi16(rho_rpi_1_3,y1i);
        abs_pi16(xmm2,xmm0,psi_i_p3_m1,xmm1);
        xmm2= _mm_adds_pi16(rho_rpi_3_3,y1i);
        abs_pi16(xmm2,xmm0,psi_i_p3_m3,xmm1);
        xmm2= _mm_adds_pi16(rho_rpi_1_1,y1r);
        abs_pi16(xmm2,xmm0,psi_r_m1_m1,xmm1);
        xmm2= _mm_adds_pi16(rho_rpi_1_3,y1r);
        abs_pi16(xmm2,xmm0,psi_r_m1_m3,xmm1);
        xmm2= _mm_adds_pi16(rho_rpi_3_1,y1r);
        abs_pi16(xmm2,xmm0,psi_r_m3_m1,xmm1);
        xmm2= _mm_adds_pi16(rho_rpi_3_3,y1r);
        abs_pi16(xmm2,xmm0,psi_r_m3_m3,xmm1);
        xmm2= _mm_adds_pi16(y1r,rho_rmi_1_1);
        abs_pi16(xmm2,xmm0,psi_r_m1_p1,xmm1);
        xmm2= _mm_adds_pi16(y1r,rho_rmi_1_3);
        abs_pi16(xmm2,xmm0,psi_r_m1_p3,xmm1);
        xmm2= _mm_adds_pi16(y1i,rho_rmi_1_1);
        abs_pi16(xmm2,xmm0,psi_i_m1_m1,xmm1);
        xmm2= _mm_adds_pi16(y1i,rho_rmi_3_1);
        abs_pi16(xmm2,xmm0,psi_i_m1_m3,xmm1);
        xmm2= _mm_adds_pi16(y1r,rho_rmi_3_1);
        abs_pi16(xmm2,xmm0,psi_r_m3_p1,xmm1);
        xmm2= _mm_adds_pi16(y1r,rho_rmi_3_3);
        abs_pi16(xmm2,xmm0,psi_r_m3_p3,xmm1);
        xmm2= _mm_adds_pi16(y1i,rho_rmi_1_3);
        abs_pi16(xmm2,xmm0,psi_i_m3_m1,xmm1);
        xmm2= _mm_adds_pi16(y1i,rho_rmi_3_3);
        abs_pi16(xmm2,xmm0,psi_i_m3_m3,xmm1);

        // Rearrange desired MF output
        xmm0 = stream0_64_in[i];
        xmm1 = stream0_64_in[i+1];
        xmm0 = _mm_shuffle_pi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_pi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        y0r  = _mm_unpacklo_pi32(xmm0,xmm1); // = [y0r(1),y0r(2),y0r(3),y0r(4)]
        y0i  = _mm_unpackhi_pi32(xmm0,xmm1);
    
        // Rearrange desired channel magnitudes
        xmm2 = ch_mag_64[i]; // = [|h|^2(1),|h|^2(1),|h|^2(2),|h|^2(2)]*(2/sqrt(10))
        xmm3 = ch_mag_64[i+1]; // = [|h|^2(3),|h|^2(3),|h|^2(4),|h|^2(4)]*(2/sqrt(10))
        xmm2 = _mm_shuffle_pi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_pi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_des = _mm_unpacklo_pi32(xmm2,xmm3); // = [|h|^2(1),|h|^2(2),|h|^2(3),|h|^2(4)]*(2/sqrt(10))

        // Rearrange interfering channel magnitudes
        xmm2 = ch_mag_64_i[i];   
        xmm3 = ch_mag_64_i[i+1]; 
        xmm2 = _mm_shuffle_pi16(xmm2,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_pi16(xmm3,0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_int  = _mm_unpacklo_pi32(xmm2,xmm3); 

        // Scale MF output of desired signal
        y0r_over_sqrt10 = _mm_mulhi_pi16(y0r,ONE_OVER_SQRT_10);
        y0i_over_sqrt10 = _mm_mulhi_pi16(y0i,ONE_OVER_SQRT_10);
        y0r_three_over_sqrt10 = _mm_mulhi_pi16(y0r,THREE_OVER_SQRT_10);
        y0i_three_over_sqrt10 = _mm_mulhi_pi16(y0i,THREE_OVER_SQRT_10);
        y0r_three_over_sqrt10 = _mm_slli_pi16(y0r_three_over_sqrt10,1);
        y0i_three_over_sqrt10 = _mm_slli_pi16(y0i_three_over_sqrt10,1);

        // Compute necessary combination of required terms
        y0_p_1_1 = _mm_adds_pi16(y0r_over_sqrt10,y0i_over_sqrt10);
        y0_m_1_1 = _mm_subs_pi16(y0r_over_sqrt10,y0i_over_sqrt10);  
    
        y0_p_1_3 = _mm_adds_pi16(y0r_over_sqrt10,y0i_three_over_sqrt10);
        y0_m_1_3 = _mm_subs_pi16(y0r_over_sqrt10,y0i_three_over_sqrt10);
        
        y0_p_3_1 = _mm_adds_pi16(y0r_three_over_sqrt10,y0i_over_sqrt10);
        y0_m_3_1 = _mm_subs_pi16(y0r_three_over_sqrt10,y0i_over_sqrt10);
        
        y0_p_3_3 = _mm_adds_pi16(y0r_three_over_sqrt10,y0i_three_over_sqrt10);
        y0_m_3_3 = _mm_subs_pi16(y0r_three_over_sqrt10,y0i_three_over_sqrt10);

        // Compute optimal interfering symbol magnitude
        interference_abs_pi16(&psi_r_p1_p1 ,&ch_mag_int,&a_r_p1_p1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_p1_p1 ,&ch_mag_int,&a_i_p1_p1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_p1_p3 ,&ch_mag_int,&a_r_p1_p3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_p1_p3 ,&ch_mag_int,&a_i_p1_p3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_p1_m1 ,&ch_mag_int,&a_r_p1_m1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_p1_m1 ,&ch_mag_int,&a_i_p1_m1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_p1_m3 ,&ch_mag_int,&a_r_p1_m3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_p1_m3 ,&ch_mag_int,&a_i_p1_m3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_p3_p1 ,&ch_mag_int,&a_r_p3_p1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_p3_p1 ,&ch_mag_int,&a_i_p3_p1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_p3_p3 ,&ch_mag_int,&a_r_p3_p3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_p3_p3 ,&ch_mag_int,&a_i_p3_p3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_p3_m1 ,&ch_mag_int,&a_r_p3_m1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_p3_m1 ,&ch_mag_int,&a_i_p3_m1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_p3_m3 ,&ch_mag_int,&a_r_p3_m3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_p3_m3 ,&ch_mag_int,&a_i_p3_m3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_m1_p1 ,&ch_mag_int,&a_r_m1_p1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_m1_p1 ,&ch_mag_int,&a_i_m1_p1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_m1_p3 ,&ch_mag_int,&a_r_m1_p3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_m1_p3 ,&ch_mag_int,&a_i_m1_p3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_m1_m1 ,&ch_mag_int,&a_r_m1_m1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_m1_m1 ,&ch_mag_int,&a_i_m1_m1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_m1_m3 ,&ch_mag_int,&a_r_m1_m3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_m1_m3 ,&ch_mag_int,&a_i_m1_m3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_m3_p1 ,&ch_mag_int,&a_r_m3_p1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_m3_p1 ,&ch_mag_int,&a_i_m3_p1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_m3_p3 ,&ch_mag_int,&a_r_m3_p3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_m3_p3 ,&ch_mag_int,&a_i_m3_p3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_m3_m1 ,&ch_mag_int,&a_r_m3_m1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_m3_m1 ,&ch_mag_int,&a_i_m3_m1 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_r_m3_m3 ,&ch_mag_int,&a_r_m3_m3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);
        interference_abs_pi16(&psi_i_m3_m3 ,&ch_mag_int,&a_i_m3_m3 ,&ONE_OVER_SQRT_10_Q15, &THREE_OVER_SQRT_10);

        // Calculation of groups of two terms in the bit metric involving product of psi and interference magnitude
        prodsum_psi_a_pi16(psi_r_p1_p1,a_r_p1_p1,psi_i_p1_p1,a_i_p1_p1,psi_a_p1_p1);
        prodsum_psi_a_pi16(psi_r_p1_p3,a_r_p1_p3,psi_i_p1_p3,a_i_p1_p3,psi_a_p1_p3);
        prodsum_psi_a_pi16(psi_r_p3_p1,a_r_p3_p1,psi_i_p3_p1,a_i_p3_p1,psi_a_p3_p1);
        prodsum_psi_a_pi16(psi_r_p3_p3,a_r_p3_p3,psi_i_p3_p3,a_i_p3_p3,psi_a_p3_p3);
        prodsum_psi_a_pi16(psi_r_p1_m1,a_r_p1_m1,psi_i_p1_m1,a_i_p1_m1,psi_a_p1_m1);
        prodsum_psi_a_pi16(psi_r_p1_m3,a_r_p1_m3,psi_i_p1_m3,a_i_p1_m3,psi_a_p1_m3);
        prodsum_psi_a_pi16(psi_r_p3_m1,a_r_p3_m1,psi_i_p3_m1,a_i_p3_m1,psi_a_p3_m1);
        prodsum_psi_a_pi16(psi_r_p3_m3,a_r_p3_m3,psi_i_p3_m3,a_i_p3_m3,psi_a_p3_m3);
        prodsum_psi_a_pi16(psi_r_m1_p1,a_r_m1_p1,psi_i_m1_p1,a_i_m1_p1,psi_a_m1_p1);
        prodsum_psi_a_pi16(psi_r_m1_p3,a_r_m1_p3,psi_i_m1_p3,a_i_m1_p3,psi_a_m1_p3);
        prodsum_psi_a_pi16(psi_r_m3_p1,a_r_m3_p1,psi_i_m3_p1,a_i_m3_p1,psi_a_m3_p1);
        prodsum_psi_a_pi16(psi_r_m3_p3,a_r_m3_p3,psi_i_m3_p3,a_i_m3_p3,psi_a_m3_p3);
        prodsum_psi_a_pi16(psi_r_m1_m1,a_r_m1_m1,psi_i_m1_m1,a_i_m1_m1,psi_a_m1_m1);
        prodsum_psi_a_pi16(psi_r_m1_m3,a_r_m1_m3,psi_i_m1_m3,a_i_m1_m3,psi_a_m1_m3);
        prodsum_psi_a_pi16(psi_r_m3_m1,a_r_m3_m1,psi_i_m3_m1,a_i_m3_m1,psi_a_m3_m1);
        prodsum_psi_a_pi16(psi_r_m3_m3,a_r_m3_m3,psi_i_m3_m3,a_i_m3_m3,psi_a_m3_m3);

        
        // squared interference magnitude times int. ch. power
        square_a_pi16(a_r_p1_p1,a_i_p1_p1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p1_p1);
        square_a_pi16(a_r_p1_p3,a_i_p1_p3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p1_p3);
        square_a_pi16(a_r_p3_p1,a_i_p3_p1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p3_p1);
        square_a_pi16(a_r_p3_p3,a_i_p3_p3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p3_p3);
        square_a_pi16(a_r_p1_m1,a_i_p1_m1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p1_m1);
        square_a_pi16(a_r_p1_m3,a_i_p1_m3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p1_m3);
        square_a_pi16(a_r_p3_m1,a_i_p3_m1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p3_m1);
        square_a_pi16(a_r_p3_m3,a_i_p3_m3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_p3_m3);
        square_a_pi16(a_r_m1_p1,a_i_m1_p1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m1_p1);
        square_a_pi16(a_r_m1_p3,a_i_m1_p3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m1_p3);
        square_a_pi16(a_r_m3_p1,a_i_m3_p1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m3_p1);
        square_a_pi16(a_r_m3_p3,a_i_m3_p3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m3_p3);
        square_a_pi16(a_r_m1_m1,a_i_m1_m1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m1_m1);
        square_a_pi16(a_r_m1_m3,a_i_m1_m3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m1_m3);
        square_a_pi16(a_r_m3_m1,a_i_m3_m1,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m3_m1);
        square_a_pi16(a_r_m3_m3,a_i_m3_m3,ch_mag_int,SQRT_10_OVER_FOUR,a_sq_m3_m3);   

        // Computing different multiples of channel norms
        ch_mag_over_10=_mm_mulhi_pi16(ch_mag_des, ONE_OVER_TWO_SQRT_10);
        ch_mag_over_2=_mm_mulhi_pi16(ch_mag_des, SQRT_10_OVER_FOUR);
        ch_mag_over_2=_mm_slli_pi16(ch_mag_over_2, 1);
        ch_mag_9_over_10=_mm_mulhi_pi16(ch_mag_des, NINE_OVER_TWO_SQRT_10);
        ch_mag_9_over_10=_mm_slli_pi16(ch_mag_9_over_10, 2);                  

        // Computing Metrics
        xmm0 = _mm_subs_pi16(psi_a_p1_p1,a_sq_p1_p1);
        xmm1 = _mm_adds_pi16(xmm0,y0_p_1_1);
        bit_met_p1_p1= _mm_subs_pi16(xmm1,ch_mag_over_10);

        xmm0 = _mm_subs_pi16(psi_a_p1_p3,a_sq_p1_p3);
        xmm1 = _mm_adds_pi16(xmm0,y0_p_1_3);
        bit_met_p1_p3= _mm_subs_pi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_pi16(psi_a_p1_m1,a_sq_p1_m1);
        xmm1 = _mm_adds_pi16(xmm0,y0_m_1_1);
        bit_met_p1_m1= _mm_subs_pi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_pi16(psi_a_p1_m3,a_sq_p1_m3);
        xmm1 = _mm_adds_pi16(xmm0,y0_m_1_3);
        bit_met_p1_m3= _mm_subs_pi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_pi16(psi_a_p3_p1,a_sq_p3_p1);
        xmm1 = _mm_adds_pi16(xmm0,y0_p_3_1);
        bit_met_p3_p1= _mm_subs_pi16(xmm1,ch_mag_over_2);

        xmm0 = _mm_subs_pi16(psi_a_p3_p3,a_sq_p3_p3);
        xmm1 = _mm_adds_pi16(xmm0,y0_p_3_3);
        bit_met_p3_p3= _mm_subs_pi16(xmm1,ch_mag_9_over_10);

        xmm0 = _mm_subs_pi16(psi_a_p3_m1,a_sq_p3_m1);
        xmm1 = _mm_adds_pi16(xmm0,y0_m_3_1);
        bit_met_p3_m1= _mm_subs_pi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_pi16(psi_a_p3_m3,a_sq_p3_m3);
        xmm1 = _mm_adds_pi16(xmm0,y0_m_3_3);
        bit_met_p3_m3= _mm_subs_pi16(xmm1,ch_mag_9_over_10);
        
        xmm0 = _mm_subs_pi16(psi_a_m1_p1,a_sq_m1_p1);
        xmm1 = _mm_subs_pi16(xmm0,y0_m_1_1);
        bit_met_m1_p1= _mm_subs_pi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_pi16(psi_a_m1_p3,a_sq_m1_p3);
        xmm1 = _mm_subs_pi16(xmm0,y0_m_1_3);
        bit_met_m1_p3= _mm_subs_pi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_pi16(psi_a_m1_m1,a_sq_m1_m1);
        xmm1 = _mm_subs_pi16(xmm0,y0_p_1_1);
        bit_met_m1_m1= _mm_subs_pi16(xmm1,ch_mag_over_10);
        
        xmm0 = _mm_subs_pi16(psi_a_m1_m3,a_sq_m1_m3);
        xmm1 = _mm_subs_pi16(xmm0,y0_p_1_3);
        bit_met_m1_m3= _mm_subs_pi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_pi16(psi_a_m3_p1,a_sq_m3_p1);
        xmm1 = _mm_subs_pi16(xmm0,y0_m_3_1);
        bit_met_m3_p1= _mm_subs_pi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_pi16(psi_a_m3_p3,a_sq_m3_p3);
        xmm1 = _mm_subs_pi16(xmm0,y0_m_3_3);
        bit_met_m3_p3= _mm_subs_pi16(xmm1,ch_mag_9_over_10);
        
        xmm0 = _mm_subs_pi16(psi_a_m3_m1,a_sq_m3_m1);
        xmm1 = _mm_subs_pi16(xmm0,y0_p_3_1);
        bit_met_m3_m1= _mm_subs_pi16(xmm1,ch_mag_over_2);
        
        xmm0 = _mm_subs_pi16(psi_a_m3_m3,a_sq_m3_m3);
        xmm1 = _mm_subs_pi16(xmm0,y0_p_3_3);
        bit_met_m3_m3= _mm_subs_pi16(xmm1,ch_mag_9_over_10);
        
        // LLR of the first bit
        // Bit = 1
        xmm0 = _mm_max_pi16(bit_met_m1_p1,bit_met_m1_p3);
        xmm1 = _mm_max_pi16(bit_met_m1_m1,bit_met_m1_m3);
        xmm2 = _mm_max_pi16(bit_met_m3_p1,bit_met_m3_p3);
        xmm3 = _mm_max_pi16(bit_met_m3_m1,bit_met_m3_m3);
        xmm4 = _mm_max_pi16(xmm0,xmm1);
        xmm5 = _mm_max_pi16(xmm2,xmm3);
        logmax_num_re0 = _mm_max_pi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_pi16(bit_met_p1_p1,bit_met_p1_p3);
        xmm1 = _mm_max_pi16(bit_met_p1_m1,bit_met_p1_m3);
        xmm2 = _mm_max_pi16(bit_met_p3_p1,bit_met_p3_p3);
        xmm3 = _mm_max_pi16(bit_met_p3_m1,bit_met_p3_m3);
        xmm4 = _mm_max_pi16(xmm0,xmm1);
        xmm5 = _mm_max_pi16(xmm2,xmm3);
        logmax_den_re0 = _mm_max_pi16(xmm4,xmm5);

        // LLR of first bit [L1(1), L1(2), L1(3), L1(4)]
        y0r = _mm_subs_pi16(logmax_den_re0,logmax_num_re0);    

        // LLR of the second bit
        // Bit = 1
        xmm0 = _mm_max_pi16(bit_met_p1_m1,bit_met_p3_m1);
        xmm1 = _mm_max_pi16(bit_met_m1_m1,bit_met_m3_m1);
        xmm2 = _mm_max_pi16(bit_met_p1_m3,bit_met_p3_m3);
        xmm3 = _mm_max_pi16(bit_met_m1_m3,bit_met_m3_m3);
        xmm4 = _mm_max_pi16(xmm0,xmm1);
        xmm5 = _mm_max_pi16(xmm2,xmm3);
        logmax_num_re1 = _mm_max_pi16(xmm4,xmm5);
   
        // Bit = 0
        xmm0 = _mm_max_pi16(bit_met_p1_p1,bit_met_p3_p1);
        xmm1 = _mm_max_pi16(bit_met_m1_p1,bit_met_m3_p1);
        xmm2 = _mm_max_pi16(bit_met_p1_p3,bit_met_p3_p3);
        xmm3 = _mm_max_pi16(bit_met_m1_p3,bit_met_m3_p3);
        xmm4 = _mm_max_pi16(xmm0,xmm1);
        xmm5 = _mm_max_pi16(xmm2,xmm3);
        logmax_den_re1 = _mm_max_pi16(xmm4,xmm5);

        // LLR of second bit [L2(1), L2(2), L2(3), L2(4)]
        y1r = _mm_subs_pi16(logmax_den_re1,logmax_num_re1);

        // LLR of the third bit
        // Bit = 1
        xmm0 = _mm_max_pi16(bit_met_m3_p1,bit_met_m3_p3);
        xmm1 = _mm_max_pi16(bit_met_m3_m1,bit_met_m3_m3);
        xmm2 = _mm_max_pi16(bit_met_p3_p1,bit_met_p3_p3);
        xmm3 = _mm_max_pi16(bit_met_p3_m1,bit_met_p3_m3);
        xmm4 = _mm_max_pi16(xmm0,xmm1);
        xmm5 = _mm_max_pi16(xmm2,xmm3);
        logmax_num_im0 = _mm_max_pi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_pi16(bit_met_m1_p1,bit_met_m1_p3);
        xmm1 = _mm_max_pi16(bit_met_m1_m1,bit_met_m1_m3);
        xmm2 = _mm_max_pi16(bit_met_p1_p1,bit_met_p1_p3);
        xmm3 = _mm_max_pi16(bit_met_p1_m1,bit_met_p1_m3);
        xmm4 = _mm_max_pi16(xmm0,xmm1);
        xmm5 = _mm_max_pi16(xmm2,xmm3);
        logmax_den_im0 = _mm_max_pi16(xmm4,xmm5);

        // LLR of third bit [L3(1), L3(2), L3(3), L3(4)]
        y0i = _mm_subs_pi16(logmax_den_im0,logmax_num_im0);

        // LLR of the fourth bit
        // Bit = 1
        xmm0 = _mm_max_pi16(bit_met_p1_m3,bit_met_p3_m3);
        xmm1 = _mm_max_pi16(bit_met_m1_m3,bit_met_m3_m3);
        xmm2 = _mm_max_pi16(bit_met_p1_p3,bit_met_p3_p3);
        xmm3 = _mm_max_pi16(bit_met_m1_p3,bit_met_m3_p3);
        xmm4 = _mm_max_pi16(xmm0,xmm1);
        xmm5 = _mm_max_pi16(xmm2,xmm3);
        logmax_num_im1 = _mm_max_pi16(xmm4,xmm5);

        // Bit = 0
        xmm0 = _mm_max_pi16(bit_met_p1_m1,bit_met_p3_m1);
        xmm1 = _mm_max_pi16(bit_met_m1_m1,bit_met_m3_m1);
        xmm2 = _mm_max_pi16(bit_met_p1_p1,bit_met_p3_p1);
        xmm3 = _mm_max_pi16(bit_met_m1_p1,bit_met_m3_p1);
        xmm4 = _mm_max_pi16(xmm0,xmm1);
        xmm5 = _mm_max_pi16(xmm2,xmm3);
        logmax_den_im1 = _mm_max_pi16(xmm4,xmm5);

        // LLR of fourth bit [L4(1), L4(2), L4(3), L4(4)]
        y1i = _mm_subs_pi16(logmax_den_im1,logmax_num_im1);

        // Pack LLRs in output
        xmm0 = _mm_unpacklo_pi16(y0r,y0i); // = [L1(1), L3(1), L1(2), L3(2)]
        xmm1 = _mm_unpackhi_pi16(y0r,y0i); // = [L1(3), L3(3), L1(4), L3(4)]
        xmm2 = _mm_unpacklo_pi16(y1r,y1i); // = [L2(1), L4(1), L2(2), L4(2)]
        xmm3 = _mm_unpackhi_pi16(y1r,y1i); // = [L2(3), L4(3), L2(4), L4(4)]

        stream0_64_out[2*i+0] = _mm_unpacklo_pi16(xmm0,xmm2);
        stream0_64_out[2*i+1] = _mm_unpackhi_pi16(xmm0,xmm2);
        stream0_64_out[2*i+2] = _mm_unpacklo_pi16(xmm1,xmm3);
        stream0_64_out[2*i+3] = _mm_unpackhi_pi16(xmm1,xmm3);
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

//----------------------------------------------------------------------------------------------
// 64-QAM
//----------------------------------------------------------------------------------------------

NOCYGWIN_STATIC __m64 ONE_OVER_SQRT_2 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m64 ONE_OVER_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 THREE_OVER_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 FIVE_OVER_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 SEVEN_OVER_SQRT_42 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m64 ONE_OVER_SQRT_2_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 THREE_OVER_SQRT_2_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 FIVE_OVER_SQRT_2_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 SEVEN_OVER_SQRT_2_42 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m64 FORTYNINE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 THIRTYSEVEN_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 TWENTYNINE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 TWENTYFIVE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 SEVENTEEN_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 NINE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 THIRTEEN_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 FIVE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ONE_OVER_FOUR_SQRT_42 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 SQRT_42_OVER_FOUR __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m64  y0r_one_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64  y0r_three_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64  y0r_five_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64  y0r_seven_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64  y0i_one_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64  y0i_three_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64  y0i_five_over_sqrt_21 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64  y0i_seven_over_sqrt_21 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m64 ch_mag_int_direct __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_98_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_74_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_58_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_50_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_34_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_18_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_26_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_10_over_42_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 ch_mag_2_over_42_with_sigma2 __attribute__((aligned(16))); 

NOCYGWIN_STATIC __m64 ch_mag_int_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 two_ch_mag_int_with_sigma2 __attribute__((aligned(16))); 
NOCYGWIN_STATIC __m64 three_ch_mag_int_with_sigma2 __attribute__((aligned(16))); 

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

    __m64 *rho01_64      = (__m64 *)rho01;
    __m64 *stream0_64_in = (__m64 *)stream0_in;
    __m64 *stream1_64_in = (__m64 *)stream1_in;
    __m64 *ch_mag_64     = (__m64 *)ch_mag;
    __m64 *ch_mag_64_i   = (__m64 *)ch_mag_i;

    int i;

    ((short*)&ONE_OVER_SQRT_42)[0] = 10112; // round(1/sqrt(42)*2^16)
    ((short*)&ONE_OVER_SQRT_42)[1] = 10112;
    ((short*)&ONE_OVER_SQRT_42)[2] = 10112;
    ((short*)&ONE_OVER_SQRT_42)[3] = 10112;
  
    ((short*)&THREE_OVER_SQRT_42)[0] = 30337; // round(3/sqrt(42)*2^16)
    ((short*)&THREE_OVER_SQRT_42)[1] = 30337;
    ((short*)&THREE_OVER_SQRT_42)[2] = 30337;
    ((short*)&THREE_OVER_SQRT_42)[3] = 30337;
  
    ((short*)&FIVE_OVER_SQRT_42)[0] = 25281; // round(5/sqrt(42)*2^15)
    ((short*)&FIVE_OVER_SQRT_42)[1] = 25281;
    ((short*)&FIVE_OVER_SQRT_42)[2] = 25281;
    ((short*)&FIVE_OVER_SQRT_42)[3] = 25281;
  
    ((short*)&SEVEN_OVER_SQRT_42)[0] = 17697; // round(5/sqrt(42)*2^15)
    ((short*)&SEVEN_OVER_SQRT_42)[1] = 17697;
    ((short*)&SEVEN_OVER_SQRT_42)[2] = 17697;
    ((short*)&SEVEN_OVER_SQRT_42)[3] = 17697;
    
    ((short*)&ONE_OVER_SQRT_2)[0] = 23170; // round(1/sqrt(2)*2^15)
    ((short*)&ONE_OVER_SQRT_2)[1] = 23170;
    ((short*)&ONE_OVER_SQRT_2)[2] = 23170;
    ((short*)&ONE_OVER_SQRT_2)[3] = 23170;
  
    ((short*)&ONE_OVER_SQRT_2_42)[0] = 3575; // round(1/sqrt(2*42)*2^15)
    ((short*)&ONE_OVER_SQRT_2_42)[1] = 3575;
    ((short*)&ONE_OVER_SQRT_2_42)[2] = 3575;
    ((short*)&ONE_OVER_SQRT_2_42)[3] = 3575;
  
    ((short*)&THREE_OVER_SQRT_2_42)[0] = 10726; // round(3/sqrt(2*42)*2^15)
    ((short*)&THREE_OVER_SQRT_2_42)[1] = 10726;
    ((short*)&THREE_OVER_SQRT_2_42)[2] = 10726;
    ((short*)&THREE_OVER_SQRT_2_42)[3] = 10726;
  
    ((short*)&FIVE_OVER_SQRT_2_42)[0] = 17876; // round(5/sqrt(2*42)*2^15)
    ((short*)&FIVE_OVER_SQRT_2_42)[1] = 17876;
    ((short*)&FIVE_OVER_SQRT_2_42)[2] = 17876;
    ((short*)&FIVE_OVER_SQRT_2_42)[3] = 17876;

    ((short*)&SEVEN_OVER_SQRT_2_42)[0] = 25027; // round(7/sqrt(2*42)*2^15)
    ((short*)&SEVEN_OVER_SQRT_2_42)[1] = 25027;
    ((short*)&SEVEN_OVER_SQRT_2_42)[2] = 25027;
    ((short*)&SEVEN_OVER_SQRT_2_42)[3] = 25027;

    ((short*)&FORTYNINE_OVER_FOUR_SQRT_42)[0] = 30969; // round(49/(4*sqrt(42))*2^14)
    ((short*)&FORTYNINE_OVER_FOUR_SQRT_42)[1] = 30969; // Q2.14
    ((short*)&FORTYNINE_OVER_FOUR_SQRT_42)[2] = 30969;
    ((short*)&FORTYNINE_OVER_FOUR_SQRT_42)[3] = 30969;

    ((short*)&THIRTYSEVEN_OVER_FOUR_SQRT_42)[0] = 23385; // round(37/(4*sqrt(42))*2^14)
    ((short*)&THIRTYSEVEN_OVER_FOUR_SQRT_42)[1] = 23385; // Q2.14
    ((short*)&THIRTYSEVEN_OVER_FOUR_SQRT_42)[2] = 23385;
    ((short*)&THIRTYSEVEN_OVER_FOUR_SQRT_42)[3] = 23385;

    ((short*)&TWENTYFIVE_OVER_FOUR_SQRT_42)[0] = 31601; // round(25/(4*sqrt(42))*2^15)
    ((short*)&TWENTYFIVE_OVER_FOUR_SQRT_42)[1] = 31601;
    ((short*)&TWENTYFIVE_OVER_FOUR_SQRT_42)[2] = 31601;
    ((short*)&TWENTYFIVE_OVER_FOUR_SQRT_42)[3] = 31601;

    ((short*)&TWENTYNINE_OVER_FOUR_SQRT_42)[0] = 18329; // round(29/(4*sqrt(42))*2^15)
    ((short*)&TWENTYNINE_OVER_FOUR_SQRT_42)[1] = 18329; // Q2.14
    ((short*)&TWENTYNINE_OVER_FOUR_SQRT_42)[2] = 18329;
    ((short*)&TWENTYNINE_OVER_FOUR_SQRT_42)[3] = 18329;

    ((short*)&SEVENTEEN_OVER_FOUR_SQRT_42)[0] = 21489; // round(17/(4*sqrt(42))*2^15)
    ((short*)&SEVENTEEN_OVER_FOUR_SQRT_42)[1] = 21489;
    ((short*)&SEVENTEEN_OVER_FOUR_SQRT_42)[2] = 21489;
    ((short*)&SEVENTEEN_OVER_FOUR_SQRT_42)[3] = 21489;

    ((short*)&NINE_OVER_FOUR_SQRT_42)[0] = 11376; // round(9/(4*sqrt(42))*2^15)
    ((short*)&NINE_OVER_FOUR_SQRT_42)[1] = 11376;
    ((short*)&NINE_OVER_FOUR_SQRT_42)[2] = 11376;
    ((short*)&NINE_OVER_FOUR_SQRT_42)[3] = 11376;

    ((short*)&THIRTEEN_OVER_FOUR_SQRT_42)[0] = 16433; // round(13/(4*sqrt(42))*2^15)
    ((short*)&THIRTEEN_OVER_FOUR_SQRT_42)[1] = 16433;
    ((short*)&THIRTEEN_OVER_FOUR_SQRT_42)[2] = 16433;
    ((short*)&THIRTEEN_OVER_FOUR_SQRT_42)[3] = 16433;

    ((short*)&FIVE_OVER_FOUR_SQRT_42)[0] = 6320; // round(5/(4*sqrt(42))*2^15)
    ((short*)&FIVE_OVER_FOUR_SQRT_42)[1] = 6320;
    ((short*)&FIVE_OVER_FOUR_SQRT_42)[2] = 6320;
    ((short*)&FIVE_OVER_FOUR_SQRT_42)[3] = 6320;

    ((short*)&ONE_OVER_FOUR_SQRT_42)[0] = 1264; // round(1/(4*sqrt(42))*2^15)
    ((short*)&ONE_OVER_FOUR_SQRT_42)[1] = 1264;
    ((short*)&ONE_OVER_FOUR_SQRT_42)[2] = 1264;
    ((short*)&ONE_OVER_FOUR_SQRT_42)[3] = 1264;

    ((short*)&SQRT_42_OVER_FOUR)[0] = 13272; // round(sqrt(42)/4*2^13)
    ((short*)&SQRT_42_OVER_FOUR)[1] = 13272; // Q3.12
    ((short*)&SQRT_42_OVER_FOUR)[2] = 13272;
    ((short*)&SQRT_42_OVER_FOUR)[3] = 13272;

    for (i=0; i<length>>1; i+=2) {
          
        // Get rho
        xmm0 = rho01_64[i];
        xmm1 = rho01_64[i+1];    
        xmm0 = _mm_shuffle_pi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3))
        xmm1 = _mm_shuffle_pi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3))
        xmm2 = _mm_unpacklo_pi32(xmm0,xmm1); // Re(rho)
        xmm3 = _mm_unpackhi_pi32(xmm0,xmm1); // Im(rho)
        rho_rpi = _mm_adds_pi16(xmm2, xmm3); // rho = Re(rho) + Im(rho) 
        rho_rmi = _mm_subs_pi16(xmm2, xmm3); // rho* = Re(rho) - Im(rho)

        // Compute the different rhos
        rho_rpi_1_1 = _mm_mulhi_pi16(rho_rpi, ONE_OVER_SQRT_42);   
        rho_rmi_1_1 = _mm_mulhi_pi16(rho_rmi, ONE_OVER_SQRT_42);
        rho_rpi_3_3 = _mm_mulhi_pi16(rho_rpi, THREE_OVER_SQRT_42); 
        rho_rmi_3_3 = _mm_mulhi_pi16(rho_rmi, THREE_OVER_SQRT_42);
        rho_rpi_5_5 = _mm_mulhi_pi16(rho_rpi, FIVE_OVER_SQRT_42);  
        rho_rmi_5_5 = _mm_mulhi_pi16(rho_rmi, FIVE_OVER_SQRT_42); 
        rho_rpi_7_7 = _mm_mulhi_pi16(rho_rpi, SEVEN_OVER_SQRT_42); 
        rho_rmi_7_7 = _mm_mulhi_pi16(rho_rmi, SEVEN_OVER_SQRT_42);

        rho_rpi_5_5 = _mm_slli_pi16(rho_rpi_5_5, 1); 
        rho_rmi_5_5 = _mm_slli_pi16(rho_rmi_5_5, 1);
        rho_rpi_7_7 = _mm_slli_pi16(rho_rpi_7_7, 2); 
        rho_rmi_7_7 = _mm_slli_pi16(rho_rmi_7_7, 2);

        xmm4 = _mm_mulhi_pi16(xmm2, ONE_OVER_SQRT_42);   
        xmm5 = _mm_mulhi_pi16(xmm3, ONE_OVER_SQRT_42);   
        xmm6 = _mm_mulhi_pi16(xmm3, THREE_OVER_SQRT_42); 
        xmm7 = _mm_mulhi_pi16(xmm3, FIVE_OVER_SQRT_42);  
        xmm8 = _mm_mulhi_pi16(xmm3, SEVEN_OVER_SQRT_42); 
        xmm7 = _mm_slli_pi16(xmm7, 1);
        xmm8 = _mm_slli_pi16(xmm8, 2);

        rho_rpi_1_3 = _mm_adds_pi16(xmm4, xmm6); 
        rho_rmi_1_3 = _mm_subs_pi16(xmm4, xmm6); 
        rho_rpi_1_5 = _mm_adds_pi16(xmm4, xmm7); 
        rho_rmi_1_5 = _mm_subs_pi16(xmm4, xmm7);
        rho_rpi_1_7 = _mm_adds_pi16(xmm4, xmm8); 
        rho_rmi_1_7 = _mm_subs_pi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_pi16(xmm2, THREE_OVER_SQRT_42); 
        rho_rpi_3_1 = _mm_adds_pi16(xmm4, xmm5); 
        rho_rmi_3_1 = _mm_subs_pi16(xmm4, xmm5); 
        rho_rpi_3_5 = _mm_adds_pi16(xmm4, xmm7); 
        rho_rmi_3_5 = _mm_subs_pi16(xmm4, xmm7);
        rho_rpi_3_7 = _mm_adds_pi16(xmm4, xmm8); 
        rho_rmi_3_7 = _mm_subs_pi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_pi16(xmm2, FIVE_OVER_SQRT_42); 
        xmm4 = _mm_slli_pi16(xmm4, 1);
        rho_rpi_5_1 = _mm_adds_pi16(xmm4, xmm5); 
        rho_rmi_5_1 = _mm_subs_pi16(xmm4, xmm5); 
        rho_rpi_5_3 = _mm_adds_pi16(xmm4, xmm6); 
        rho_rmi_5_3 = _mm_subs_pi16(xmm4, xmm6);
        rho_rpi_5_7 = _mm_adds_pi16(xmm4, xmm8); 
        rho_rmi_5_7 = _mm_subs_pi16(xmm4, xmm8);

        xmm4 = _mm_mulhi_pi16(xmm2, SEVEN_OVER_SQRT_42); 
        xmm4 = _mm_slli_pi16(xmm4, 2);
        rho_rpi_7_1 = _mm_adds_pi16(xmm4, xmm5); 
        rho_rmi_7_1 = _mm_subs_pi16(xmm4, xmm5); 
        rho_rpi_7_3 = _mm_adds_pi16(xmm4, xmm6); 
        rho_rmi_7_3 = _mm_subs_pi16(xmm4, xmm6);
        rho_rpi_7_5 = _mm_adds_pi16(xmm4, xmm7); 
        rho_rmi_7_5 = _mm_subs_pi16(xmm4, xmm7);

        // Rearrange interfering MF output
        xmm0 = stream1_64_in[i];
        xmm1 = stream1_64_in[i+1];           
        xmm0 = _mm_shuffle_pi16(xmm0,0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_pi16(xmm1,0xd8); //_MM_SHUFFLE(0,2,1,3));
        y1r  = _mm_unpacklo_pi32(xmm0,xmm1); // = [y1r(1),y1r(2),y1r(3),y1r(4)]
        y1i  = _mm_unpackhi_pi32(xmm0,xmm1);

        // Psi_r calculation from rho_rpi or rho_rmi
        xmm0 = _mm_xor_si64(xmm0, xmm0); // ZERO for abs_pi16
        xmm2 = _mm_subs_pi16(rho_rpi_7_7, y1r); abs_pi16(xmm2, xmm0, psi_r_p7_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_7_5, y1r); abs_pi16(xmm2, xmm0, psi_r_p7_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_7_3, y1r); abs_pi16(xmm2, xmm0, psi_r_p7_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_7_1, y1r); abs_pi16(xmm2, xmm0, psi_r_p7_p1, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_7_1, y1r); abs_pi16(xmm2, xmm0, psi_r_p7_m1, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_7_3, y1r); abs_pi16(xmm2, xmm0, psi_r_p7_m3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_7_5, y1r); abs_pi16(xmm2, xmm0, psi_r_p7_m5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_7_7, y1r); abs_pi16(xmm2, xmm0, psi_r_p7_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_5_7, y1r); abs_pi16(xmm2, xmm0, psi_r_p5_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_5_5, y1r); abs_pi16(xmm2, xmm0, psi_r_p5_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_5_3, y1r); abs_pi16(xmm2, xmm0, psi_r_p5_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_5_1, y1r); abs_pi16(xmm2, xmm0, psi_r_p5_p1, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_5_1, y1r); abs_pi16(xmm2, xmm0, psi_r_p5_m1, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_5_3, y1r); abs_pi16(xmm2, xmm0, psi_r_p5_m3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_5_5, y1r); abs_pi16(xmm2, xmm0, psi_r_p5_m5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_5_7, y1r); abs_pi16(xmm2, xmm0, psi_r_p5_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_3_7, y1r); abs_pi16(xmm2, xmm0, psi_r_p3_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_3_5, y1r); abs_pi16(xmm2, xmm0, psi_r_p3_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_3_3, y1r); abs_pi16(xmm2, xmm0, psi_r_p3_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_3_1, y1r); abs_pi16(xmm2, xmm0, psi_r_p3_p1, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_3_1, y1r); abs_pi16(xmm2, xmm0, psi_r_p3_m1, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_3_3, y1r); abs_pi16(xmm2, xmm0, psi_r_p3_m3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_3_5, y1r); abs_pi16(xmm2, xmm0, psi_r_p3_m5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_3_7, y1r); abs_pi16(xmm2, xmm0, psi_r_p3_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_1_7, y1r); abs_pi16(xmm2, xmm0, psi_r_p1_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_1_5, y1r); abs_pi16(xmm2, xmm0, psi_r_p1_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_1_3, y1r); abs_pi16(xmm2, xmm0, psi_r_p1_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_1_1, y1r); abs_pi16(xmm2, xmm0, psi_r_p1_p1, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_1_1, y1r); abs_pi16(xmm2, xmm0, psi_r_p1_m1, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_1_3, y1r); abs_pi16(xmm2, xmm0, psi_r_p1_m3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_1_5, y1r); abs_pi16(xmm2, xmm0, psi_r_p1_m5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_1_7, y1r); abs_pi16(xmm2, xmm0, psi_r_p1_m7, xmm1);
        
        xmm2 = _mm_adds_pi16(rho_rmi_1_7, y1r); abs_pi16(xmm2, xmm0, psi_r_m1_p7, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_1_5, y1r); abs_pi16(xmm2, xmm0, psi_r_m1_p5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_1_3, y1r); abs_pi16(xmm2, xmm0, psi_r_m1_p3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_1_1, y1r); abs_pi16(xmm2, xmm0, psi_r_m1_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_1_1, y1r); abs_pi16(xmm2, xmm0, psi_r_m1_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_1_3, y1r); abs_pi16(xmm2, xmm0, psi_r_m1_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_1_5, y1r); abs_pi16(xmm2, xmm0, psi_r_m1_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_1_7, y1r); abs_pi16(xmm2, xmm0, psi_r_m1_m7, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_3_7, y1r); abs_pi16(xmm2, xmm0, psi_r_m3_p7, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_3_5, y1r); abs_pi16(xmm2, xmm0, psi_r_m3_p5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_3_3, y1r); abs_pi16(xmm2, xmm0, psi_r_m3_p3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_3_1, y1r); abs_pi16(xmm2, xmm0, psi_r_m3_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_3_1, y1r); abs_pi16(xmm2, xmm0, psi_r_m3_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_3_3, y1r); abs_pi16(xmm2, xmm0, psi_r_m3_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_3_5, y1r); abs_pi16(xmm2, xmm0, psi_r_m3_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_3_7, y1r); abs_pi16(xmm2, xmm0, psi_r_m3_m7, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_5_7, y1r); abs_pi16(xmm2, xmm0, psi_r_m5_p7, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_5_5, y1r); abs_pi16(xmm2, xmm0, psi_r_m5_p5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_5_3, y1r); abs_pi16(xmm2, xmm0, psi_r_m5_p3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_5_1, y1r); abs_pi16(xmm2, xmm0, psi_r_m5_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_5_1, y1r); abs_pi16(xmm2, xmm0, psi_r_m5_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_5_3, y1r); abs_pi16(xmm2, xmm0, psi_r_m5_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_5_5, y1r); abs_pi16(xmm2, xmm0, psi_r_m5_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_5_7, y1r); abs_pi16(xmm2, xmm0, psi_r_m5_m7, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_7_7, y1r); abs_pi16(xmm2, xmm0, psi_r_m7_p7, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_7_5, y1r); abs_pi16(xmm2, xmm0, psi_r_m7_p5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_7_3, y1r); abs_pi16(xmm2, xmm0, psi_r_m7_p3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_7_1, y1r); abs_pi16(xmm2, xmm0, psi_r_m7_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_7_1, y1r); abs_pi16(xmm2, xmm0, psi_r_m7_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_7_3, y1r); abs_pi16(xmm2, xmm0, psi_r_m7_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_7_5, y1r); abs_pi16(xmm2, xmm0, psi_r_m7_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_7_7, y1r); abs_pi16(xmm2, xmm0, psi_r_m7_m7, xmm1);
        
        // Psi_i calculation from rho_rpi or rho_rmi
        xmm2 = _mm_subs_pi16(rho_rmi_7_7, y1i); abs_pi16(xmm2, xmm0, psi_i_p7_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_5_7, y1i); abs_pi16(xmm2, xmm0, psi_i_p7_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_3_7, y1i); abs_pi16(xmm2, xmm0, psi_i_p7_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_1_7, y1i); abs_pi16(xmm2, xmm0, psi_i_p7_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_1_7, y1i); abs_pi16(xmm2, xmm0, psi_i_p7_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_3_7, y1i); abs_pi16(xmm2, xmm0, psi_i_p7_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_5_7, y1i); abs_pi16(xmm2, xmm0, psi_i_p7_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_7_7, y1i); abs_pi16(xmm2, xmm0, psi_i_p7_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_7_5, y1i); abs_pi16(xmm2, xmm0, psi_i_p5_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_5_5, y1i); abs_pi16(xmm2, xmm0, psi_i_p5_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_3_5, y1i); abs_pi16(xmm2, xmm0, psi_i_p5_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_1_5, y1i); abs_pi16(xmm2, xmm0, psi_i_p5_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_1_5, y1i); abs_pi16(xmm2, xmm0, psi_i_p5_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_3_5, y1i); abs_pi16(xmm2, xmm0, psi_i_p5_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_5_5, y1i); abs_pi16(xmm2, xmm0, psi_i_p5_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_7_5, y1i); abs_pi16(xmm2, xmm0, psi_i_p5_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_7_3, y1i); abs_pi16(xmm2, xmm0, psi_i_p3_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_5_3, y1i); abs_pi16(xmm2, xmm0, psi_i_p3_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_3_3, y1i); abs_pi16(xmm2, xmm0, psi_i_p3_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_1_3, y1i); abs_pi16(xmm2, xmm0, psi_i_p3_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_1_3, y1i); abs_pi16(xmm2, xmm0, psi_i_p3_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_3_3, y1i); abs_pi16(xmm2, xmm0, psi_i_p3_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_5_3, y1i); abs_pi16(xmm2, xmm0, psi_i_p3_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_7_3, y1i); abs_pi16(xmm2, xmm0, psi_i_p3_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_7_1, y1i); abs_pi16(xmm2, xmm0, psi_i_p1_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_5_1, y1i); abs_pi16(xmm2, xmm0, psi_i_p1_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_3_1, y1i); abs_pi16(xmm2, xmm0, psi_i_p1_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rmi_1_1, y1i); abs_pi16(xmm2, xmm0, psi_i_p1_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_1_1, y1i); abs_pi16(xmm2, xmm0, psi_i_p1_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_3_1, y1i); abs_pi16(xmm2, xmm0, psi_i_p1_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_5_1, y1i); abs_pi16(xmm2, xmm0, psi_i_p1_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rpi_7_1, y1i); abs_pi16(xmm2, xmm0, psi_i_p1_m7, xmm1);
        
        xmm2 = _mm_subs_pi16(rho_rpi_7_1, y1i); abs_pi16(xmm2, xmm0, psi_i_m1_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_5_1, y1i); abs_pi16(xmm2, xmm0, psi_i_m1_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_3_1, y1i); abs_pi16(xmm2, xmm0, psi_i_m1_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_1_1, y1i); abs_pi16(xmm2, xmm0, psi_i_m1_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_1_1, y1i); abs_pi16(xmm2, xmm0, psi_i_m1_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_3_1, y1i); abs_pi16(xmm2, xmm0, psi_i_m1_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_5_1, y1i); abs_pi16(xmm2, xmm0, psi_i_m1_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_7_1, y1i); abs_pi16(xmm2, xmm0, psi_i_m1_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_7_3, y1i); abs_pi16(xmm2, xmm0, psi_i_m3_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_5_3, y1i); abs_pi16(xmm2, xmm0, psi_i_m3_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_3_3, y1i); abs_pi16(xmm2, xmm0, psi_i_m3_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_1_3, y1i); abs_pi16(xmm2, xmm0, psi_i_m3_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_1_3, y1i); abs_pi16(xmm2, xmm0, psi_i_m3_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_3_3, y1i); abs_pi16(xmm2, xmm0, psi_i_m3_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_5_3, y1i); abs_pi16(xmm2, xmm0, psi_i_m3_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_7_3, y1i); abs_pi16(xmm2, xmm0, psi_i_m3_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_7_5, y1i); abs_pi16(xmm2, xmm0, psi_i_m5_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_5_5, y1i); abs_pi16(xmm2, xmm0, psi_i_m5_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_3_5, y1i); abs_pi16(xmm2, xmm0, psi_i_m5_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_1_5, y1i); abs_pi16(xmm2, xmm0, psi_i_m5_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_1_5, y1i); abs_pi16(xmm2, xmm0, psi_i_m5_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_3_5, y1i); abs_pi16(xmm2, xmm0, psi_i_m5_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_5_5, y1i); abs_pi16(xmm2, xmm0, psi_i_m5_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_7_5, y1i); abs_pi16(xmm2, xmm0, psi_i_m5_m7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_7_7, y1i); abs_pi16(xmm2, xmm0, psi_i_m7_p7, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_5_7, y1i); abs_pi16(xmm2, xmm0, psi_i_m7_p5, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_3_7, y1i); abs_pi16(xmm2, xmm0, psi_i_m7_p3, xmm1);
        xmm2 = _mm_subs_pi16(rho_rpi_1_7, y1i); abs_pi16(xmm2, xmm0, psi_i_m7_p1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_1_7, y1i); abs_pi16(xmm2, xmm0, psi_i_m7_m1, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_3_7, y1i); abs_pi16(xmm2, xmm0, psi_i_m7_m3, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_5_7, y1i); abs_pi16(xmm2, xmm0, psi_i_m7_m5, xmm1);
        xmm2 = _mm_adds_pi16(rho_rmi_7_7, y1i); abs_pi16(xmm2, xmm0, psi_i_m7_m7, xmm1);

        // Rearrange desired MF output
        xmm0 = stream0_64_in[i];
        xmm1 = stream0_64_in[i+1];
        xmm0 = _mm_shuffle_pi16(xmm0, 0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm1 = _mm_shuffle_pi16(xmm1, 0xd8); //_MM_SHUFFLE(0,2,1,3));
        y0r  = _mm_unpacklo_pi32(xmm0, xmm1);
        y0i  = _mm_unpackhi_pi32(xmm0, xmm1);
    
        // Rearrange desired channel magnitudes
        xmm2 = ch_mag_64[i];
        xmm3 = ch_mag_64[i+1];
        xmm2 = _mm_shuffle_pi16(xmm2, 0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_pi16(xmm3, 0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_des  = _mm_unpacklo_pi32(xmm2, xmm3);

        // Rearrange interfering channel magnitudes
        xmm2 = ch_mag_64_i[i];
        xmm3 = ch_mag_64_i[i+1];
        xmm2 = _mm_shuffle_pi16(xmm2, 0xd8); //_MM_SHUFFLE(0,2,1,3));
        xmm3 = _mm_shuffle_pi16(xmm3, 0xd8); //_MM_SHUFFLE(0,2,1,3));
        ch_mag_int  = _mm_unpacklo_pi32(xmm2, xmm3);

        y0r_one_over_sqrt_21   = _mm_mulhi_pi16(y0r, ONE_OVER_SQRT_42);   
        y0r_three_over_sqrt_21 = _mm_mulhi_pi16(y0r, THREE_OVER_SQRT_42); 
        y0r_five_over_sqrt_21  = _mm_mulhi_pi16(y0r, FIVE_OVER_SQRT_42);  
        y0r_five_over_sqrt_21  = _mm_slli_pi16(y0r_five_over_sqrt_21, 1);
        y0r_seven_over_sqrt_21 = _mm_mulhi_pi16(y0r, SEVEN_OVER_SQRT_42); 
        y0r_seven_over_sqrt_21 = _mm_slli_pi16(y0r_seven_over_sqrt_21, 2); // Q2.14

        y0i_one_over_sqrt_21   = _mm_mulhi_pi16(y0i, ONE_OVER_SQRT_42);   
        y0i_three_over_sqrt_21 = _mm_mulhi_pi16(y0i, THREE_OVER_SQRT_42); 
        y0i_five_over_sqrt_21  = _mm_mulhi_pi16(y0i, FIVE_OVER_SQRT_42);  
        y0i_five_over_sqrt_21  = _mm_slli_pi16(y0i_five_over_sqrt_21, 1);
        y0i_seven_over_sqrt_21 = _mm_mulhi_pi16(y0i, SEVEN_OVER_SQRT_42); 
        y0i_seven_over_sqrt_21 = _mm_slli_pi16(y0i_seven_over_sqrt_21, 2); // Q2.14

        y0_p_7_1 = _mm_adds_pi16(y0r_seven_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_7_3 = _mm_adds_pi16(y0r_seven_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_7_5 = _mm_adds_pi16(y0r_seven_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_7_7 = _mm_adds_pi16(y0r_seven_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_5_1 = _mm_adds_pi16(y0r_five_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_5_3 = _mm_adds_pi16(y0r_five_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_5_5 = _mm_adds_pi16(y0r_five_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_5_7 = _mm_adds_pi16(y0r_five_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_3_1 = _mm_adds_pi16(y0r_three_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_3_3 = _mm_adds_pi16(y0r_three_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_3_5 = _mm_adds_pi16(y0r_three_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_3_7 = _mm_adds_pi16(y0r_three_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_p_1_1 = _mm_adds_pi16(y0r_one_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_p_1_3 = _mm_adds_pi16(y0r_one_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_p_1_5 = _mm_adds_pi16(y0r_one_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_p_1_7 = _mm_adds_pi16(y0r_one_over_sqrt_21, y0i_seven_over_sqrt_21);

        y0_m_1_1 = _mm_subs_pi16(y0r_one_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_1_3 = _mm_subs_pi16(y0r_one_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_1_5 = _mm_subs_pi16(y0r_one_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_1_7 = _mm_subs_pi16(y0r_one_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_3_1 = _mm_subs_pi16(y0r_three_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_3_3 = _mm_subs_pi16(y0r_three_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_3_5 = _mm_subs_pi16(y0r_three_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_3_7 = _mm_subs_pi16(y0r_three_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_5_1 = _mm_subs_pi16(y0r_five_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_5_3 = _mm_subs_pi16(y0r_five_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_5_5 = _mm_subs_pi16(y0r_five_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_5_7 = _mm_subs_pi16(y0r_five_over_sqrt_21, y0i_seven_over_sqrt_21);
        y0_m_7_1 = _mm_subs_pi16(y0r_seven_over_sqrt_21, y0i_one_over_sqrt_21);
        y0_m_7_3 = _mm_subs_pi16(y0r_seven_over_sqrt_21, y0i_three_over_sqrt_21);
        y0_m_7_5 = _mm_subs_pi16(y0r_seven_over_sqrt_21, y0i_five_over_sqrt_21);
        y0_m_7_7 = _mm_subs_pi16(y0r_seven_over_sqrt_21, y0i_seven_over_sqrt_21);

        // Detection of interference term
        ch_mag_int_with_sigma2       = _mm_srai_pi16(ch_mag_int, 1); // *2
        two_ch_mag_int_with_sigma2   = ch_mag_int; // *4
        three_ch_mag_int_with_sigma2 = _mm_adds_pi16(ch_mag_int_with_sigma2, two_ch_mag_int_with_sigma2); // *6

        interference_abs_64qam_pi16(&psi_r_p7_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p7_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p7_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p7_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p7_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p7_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p7_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p7_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p7_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p7_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p7_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p7_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p7_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p7_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p7_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p7_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p5_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p5_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p5_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p5_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p5_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p5_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p5_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p5_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p5_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p5_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p5_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p5_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p5_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p5_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p5_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p5_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p3_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p3_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p3_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p3_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p3_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p3_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p3_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p3_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p3_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p3_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p3_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p3_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p3_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p3_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p3_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p3_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p1_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p1_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p1_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p1_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p1_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p1_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p1_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p1_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p1_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p1_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p1_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p1_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p1_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p1_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_p1_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_p1_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m1_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m1_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m1_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m1_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m1_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m1_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m1_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m1_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m1_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m1_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m1_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m1_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m1_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m1_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m1_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m1_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m3_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m3_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m3_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m3_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m3_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m3_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m3_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m3_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m3_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m3_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m3_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m3_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m3_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m3_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m3_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m3_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m5_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m5_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m5_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m5_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m5_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m5_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m5_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m5_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m5_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m5_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m5_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m5_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m5_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m5_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m5_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m5_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m7_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m7_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m7_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m7_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m7_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m7_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m7_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m7_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m7_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m7_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m7_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m7_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m7_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m7_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_r_m7_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_r_m7_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);

        interference_abs_64qam_pi16(&psi_i_p7_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p7_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p7_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p7_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p7_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p7_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p7_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p7_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p7_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p7_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p7_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p7_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p7_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p7_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p7_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p7_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p5_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p5_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p5_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p5_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p5_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p5_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p5_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p5_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p5_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p5_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p5_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p5_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p5_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p5_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p5_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p5_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p3_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p3_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p3_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p3_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p3_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p3_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p3_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p3_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p3_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p3_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p3_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p3_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p3_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p3_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p3_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p3_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p1_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p1_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p1_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p1_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p1_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p1_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p1_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p1_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p1_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p1_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p1_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p1_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p1_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p1_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_p1_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_p1_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m1_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m1_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m1_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m1_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m1_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m1_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m1_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m1_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m1_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m1_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m1_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m1_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m1_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m1_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m1_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m1_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m3_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m3_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m3_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m3_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m3_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m3_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m3_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m3_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m3_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m3_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m3_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m3_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m3_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m3_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m3_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m3_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m5_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m5_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m5_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m5_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m5_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m5_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m5_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m5_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m5_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m5_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m5_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m5_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m5_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m5_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m5_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m5_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m7_p7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m7_p7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m7_p5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m7_p5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m7_p3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m7_p3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m7_p1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m7_p1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m7_m1, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m7_m1, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m7_m3, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m7_m3, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m7_m5, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m7_m5, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);
        interference_abs_64qam_pi16(&psi_i_m7_m7, &ch_mag_int_with_sigma2, &two_ch_mag_int_with_sigma2, &three_ch_mag_int_with_sigma2, &a_i_m7_m7, &ONE_OVER_SQRT_2_42, &THREE_OVER_SQRT_2_42, &FIVE_OVER_SQRT_2_42, &SEVEN_OVER_SQRT_2_42);

        // Calculation of a group of two terms in the bit metric involving product of psi and interference     
        prodsum_psi_a_pi16(psi_r_p7_p7, a_r_p7_p7, psi_i_p7_p7, a_i_p7_p7, psi_a_p7_p7);
        prodsum_psi_a_pi16(psi_r_p7_p5, a_r_p7_p5, psi_i_p7_p5, a_i_p7_p5, psi_a_p7_p5); 
        prodsum_psi_a_pi16(psi_r_p7_p3, a_r_p7_p3, psi_i_p7_p3, a_i_p7_p3, psi_a_p7_p3); 
        prodsum_psi_a_pi16(psi_r_p7_p1, a_r_p7_p1, psi_i_p7_p1, a_i_p7_p1, psi_a_p7_p1); 
        prodsum_psi_a_pi16(psi_r_p7_m1, a_r_p7_m1, psi_i_p7_m1, a_i_p7_m1, psi_a_p7_m1); 
        prodsum_psi_a_pi16(psi_r_p7_m3, a_r_p7_m3, psi_i_p7_m3, a_i_p7_m3, psi_a_p7_m3); 
        prodsum_psi_a_pi16(psi_r_p7_m5, a_r_p7_m5, psi_i_p7_m5, a_i_p7_m5, psi_a_p7_m5); 
        prodsum_psi_a_pi16(psi_r_p7_m7, a_r_p7_m7, psi_i_p7_m7, a_i_p7_m7, psi_a_p7_m7); 
        prodsum_psi_a_pi16(psi_r_p5_p7, a_r_p5_p7, psi_i_p5_p7, a_i_p5_p7, psi_a_p5_p7); 
        prodsum_psi_a_pi16(psi_r_p5_p5, a_r_p5_p5, psi_i_p5_p5, a_i_p5_p5, psi_a_p5_p5); 
        prodsum_psi_a_pi16(psi_r_p5_p3, a_r_p5_p3, psi_i_p5_p3, a_i_p5_p3, psi_a_p5_p3); 
        prodsum_psi_a_pi16(psi_r_p5_p1, a_r_p5_p1, psi_i_p5_p1, a_i_p5_p1, psi_a_p5_p1); 
        prodsum_psi_a_pi16(psi_r_p5_m1, a_r_p5_m1, psi_i_p5_m1, a_i_p5_m1, psi_a_p5_m1); 
        prodsum_psi_a_pi16(psi_r_p5_m3, a_r_p5_m3, psi_i_p5_m3, a_i_p5_m3, psi_a_p5_m3); 
        prodsum_psi_a_pi16(psi_r_p5_m5, a_r_p5_m5, psi_i_p5_m5, a_i_p5_m5, psi_a_p5_m5); 
        prodsum_psi_a_pi16(psi_r_p5_m7, a_r_p5_m7, psi_i_p5_m7, a_i_p5_m7, psi_a_p5_m7); 
        prodsum_psi_a_pi16(psi_r_p3_p7, a_r_p3_p7, psi_i_p3_p7, a_i_p3_p7, psi_a_p3_p7);
        prodsum_psi_a_pi16(psi_r_p3_p5, a_r_p3_p5, psi_i_p3_p5, a_i_p3_p5, psi_a_p3_p5);
        prodsum_psi_a_pi16(psi_r_p3_p3, a_r_p3_p3, psi_i_p3_p3, a_i_p3_p3, psi_a_p3_p3);
        prodsum_psi_a_pi16(psi_r_p3_p1, a_r_p3_p1, psi_i_p3_p1, a_i_p3_p1, psi_a_p3_p1);
        prodsum_psi_a_pi16(psi_r_p3_m1, a_r_p3_m1, psi_i_p3_m1, a_i_p3_m1, psi_a_p3_m1);
        prodsum_psi_a_pi16(psi_r_p3_m3, a_r_p3_m3, psi_i_p3_m3, a_i_p3_m3, psi_a_p3_m3);
        prodsum_psi_a_pi16(psi_r_p3_m5, a_r_p3_m5, psi_i_p3_m5, a_i_p3_m5, psi_a_p3_m5);
        prodsum_psi_a_pi16(psi_r_p3_m7, a_r_p3_m7, psi_i_p3_m7, a_i_p3_m7, psi_a_p3_m7);
        prodsum_psi_a_pi16(psi_r_p1_p7, a_r_p1_p7, psi_i_p1_p7, a_i_p1_p7, psi_a_p1_p7);
        prodsum_psi_a_pi16(psi_r_p1_p5, a_r_p1_p5, psi_i_p1_p5, a_i_p1_p5, psi_a_p1_p5);
        prodsum_psi_a_pi16(psi_r_p1_p3, a_r_p1_p3, psi_i_p1_p3, a_i_p1_p3, psi_a_p1_p3);
        prodsum_psi_a_pi16(psi_r_p1_p1, a_r_p1_p1, psi_i_p1_p1, a_i_p1_p1, psi_a_p1_p1);
        prodsum_psi_a_pi16(psi_r_p1_m1, a_r_p1_m1, psi_i_p1_m1, a_i_p1_m1, psi_a_p1_m1);
        prodsum_psi_a_pi16(psi_r_p1_m3, a_r_p1_m3, psi_i_p1_m3, a_i_p1_m3, psi_a_p1_m3);
        prodsum_psi_a_pi16(psi_r_p1_m5, a_r_p1_m5, psi_i_p1_m5, a_i_p1_m5, psi_a_p1_m5);
        prodsum_psi_a_pi16(psi_r_p1_m7, a_r_p1_m7, psi_i_p1_m7, a_i_p1_m7, psi_a_p1_m7);
        prodsum_psi_a_pi16(psi_r_m1_p7, a_r_m1_p7, psi_i_m1_p7, a_i_m1_p7, psi_a_m1_p7);
        prodsum_psi_a_pi16(psi_r_m1_p5, a_r_m1_p5, psi_i_m1_p5, a_i_m1_p5, psi_a_m1_p5);
        prodsum_psi_a_pi16(psi_r_m1_p3, a_r_m1_p3, psi_i_m1_p3, a_i_m1_p3, psi_a_m1_p3);
        prodsum_psi_a_pi16(psi_r_m1_p1, a_r_m1_p1, psi_i_m1_p1, a_i_m1_p1, psi_a_m1_p1);
        prodsum_psi_a_pi16(psi_r_m1_m1, a_r_m1_m1, psi_i_m1_m1, a_i_m1_m1, psi_a_m1_m1);
        prodsum_psi_a_pi16(psi_r_m1_m3, a_r_m1_m3, psi_i_m1_m3, a_i_m1_m3, psi_a_m1_m3);
        prodsum_psi_a_pi16(psi_r_m1_m5, a_r_m1_m5, psi_i_m1_m5, a_i_m1_m5, psi_a_m1_m5);
        prodsum_psi_a_pi16(psi_r_m1_m7, a_r_m1_m7, psi_i_m1_m7, a_i_m1_m7, psi_a_m1_m7);
        prodsum_psi_a_pi16(psi_r_m3_p7, a_r_m3_p7, psi_i_m3_p7, a_i_m3_p7, psi_a_m3_p7);
        prodsum_psi_a_pi16(psi_r_m3_p5, a_r_m3_p5, psi_i_m3_p5, a_i_m3_p5, psi_a_m3_p5);
        prodsum_psi_a_pi16(psi_r_m3_p3, a_r_m3_p3, psi_i_m3_p3, a_i_m3_p3, psi_a_m3_p3);
        prodsum_psi_a_pi16(psi_r_m3_p1, a_r_m3_p1, psi_i_m3_p1, a_i_m3_p1, psi_a_m3_p1);
        prodsum_psi_a_pi16(psi_r_m3_m1, a_r_m3_m1, psi_i_m3_m1, a_i_m3_m1, psi_a_m3_m1);
        prodsum_psi_a_pi16(psi_r_m3_m3, a_r_m3_m3, psi_i_m3_m3, a_i_m3_m3, psi_a_m3_m3);
        prodsum_psi_a_pi16(psi_r_m3_m5, a_r_m3_m5, psi_i_m3_m5, a_i_m3_m5, psi_a_m3_m5);
        prodsum_psi_a_pi16(psi_r_m3_m7, a_r_m3_m7, psi_i_m3_m7, a_i_m3_m7, psi_a_m3_m7);
        prodsum_psi_a_pi16(psi_r_m5_p7, a_r_m5_p7, psi_i_m5_p7, a_i_m5_p7, psi_a_m5_p7);
        prodsum_psi_a_pi16(psi_r_m5_p5, a_r_m5_p5, psi_i_m5_p5, a_i_m5_p5, psi_a_m5_p5);
        prodsum_psi_a_pi16(psi_r_m5_p3, a_r_m5_p3, psi_i_m5_p3, a_i_m5_p3, psi_a_m5_p3);
        prodsum_psi_a_pi16(psi_r_m5_p1, a_r_m5_p1, psi_i_m5_p1, a_i_m5_p1, psi_a_m5_p1);
        prodsum_psi_a_pi16(psi_r_m5_m1, a_r_m5_m1, psi_i_m5_m1, a_i_m5_m1, psi_a_m5_m1); 
        prodsum_psi_a_pi16(psi_r_m5_m3, a_r_m5_m3, psi_i_m5_m3, a_i_m5_m3, psi_a_m5_m3);
        prodsum_psi_a_pi16(psi_r_m5_m5, a_r_m5_m5, psi_i_m5_m5, a_i_m5_m5, psi_a_m5_m5);
        prodsum_psi_a_pi16(psi_r_m5_m7, a_r_m5_m7, psi_i_m5_m7, a_i_m5_m7, psi_a_m5_m7);
        prodsum_psi_a_pi16(psi_r_m7_p7, a_r_m7_p7, psi_i_m7_p7, a_i_m7_p7, psi_a_m7_p7);
        prodsum_psi_a_pi16(psi_r_m7_p5, a_r_m7_p5, psi_i_m7_p5, a_i_m7_p5, psi_a_m7_p5);
        prodsum_psi_a_pi16(psi_r_m7_p3, a_r_m7_p3, psi_i_m7_p3, a_i_m7_p3, psi_a_m7_p3); 
        prodsum_psi_a_pi16(psi_r_m7_p1, a_r_m7_p1, psi_i_m7_p1, a_i_m7_p1, psi_a_m7_p1);
        prodsum_psi_a_pi16(psi_r_m7_m1, a_r_m7_m1, psi_i_m7_m1, a_i_m7_m1, psi_a_m7_m1); 
        prodsum_psi_a_pi16(psi_r_m7_m3, a_r_m7_m3, psi_i_m7_m3, a_i_m7_m3, psi_a_m7_m3);
        prodsum_psi_a_pi16(psi_r_m7_m5, a_r_m7_m5, psi_i_m7_m5, a_i_m7_m5, psi_a_m7_m5);
        prodsum_psi_a_pi16(psi_r_m7_m7, a_r_m7_m7, psi_i_m7_m7, a_i_m7_m7, psi_a_m7_m7);

        // Multiply by sqrt(2)
        psi_a_p7_p7 = _mm_mulhi_pi16(psi_a_p7_p7, ONE_OVER_SQRT_2);
        psi_a_p7_p7 = _mm_slli_pi16(psi_a_p7_p7, 2);
        psi_a_p7_p5 = _mm_mulhi_pi16(psi_a_p7_p5, ONE_OVER_SQRT_2);
        psi_a_p7_p5 = _mm_slli_pi16(psi_a_p7_p5, 2);
        psi_a_p7_p3 = _mm_mulhi_pi16(psi_a_p7_p3, ONE_OVER_SQRT_2);
        psi_a_p7_p3 = _mm_slli_pi16(psi_a_p7_p3, 2);
        psi_a_p7_p1 = _mm_mulhi_pi16(psi_a_p7_p1, ONE_OVER_SQRT_2);
        psi_a_p7_p1 = _mm_slli_pi16(psi_a_p7_p1, 2);
        psi_a_p7_m1 = _mm_mulhi_pi16(psi_a_p7_m1, ONE_OVER_SQRT_2);
        psi_a_p7_m1 = _mm_slli_pi16(psi_a_p7_m1, 2);
        psi_a_p7_m3 = _mm_mulhi_pi16(psi_a_p7_m3, ONE_OVER_SQRT_2);
        psi_a_p7_m3 = _mm_slli_pi16(psi_a_p7_m3, 2);
        psi_a_p7_m5 = _mm_mulhi_pi16(psi_a_p7_m5, ONE_OVER_SQRT_2);
        psi_a_p7_m5 = _mm_slli_pi16(psi_a_p7_m5, 2);
        psi_a_p7_m7 = _mm_mulhi_pi16(psi_a_p7_m7, ONE_OVER_SQRT_2);
        psi_a_p7_m7 = _mm_slli_pi16(psi_a_p7_m7, 2);
        psi_a_p5_p7 = _mm_mulhi_pi16(psi_a_p5_p7, ONE_OVER_SQRT_2);
        psi_a_p5_p7 = _mm_slli_pi16(psi_a_p5_p7, 2);
        psi_a_p5_p5 = _mm_mulhi_pi16(psi_a_p5_p5, ONE_OVER_SQRT_2);
        psi_a_p5_p5 = _mm_slli_pi16(psi_a_p5_p5, 2);
        psi_a_p5_p3 = _mm_mulhi_pi16(psi_a_p5_p3, ONE_OVER_SQRT_2);
        psi_a_p5_p3 = _mm_slli_pi16(psi_a_p5_p3, 2);
        psi_a_p5_p1 = _mm_mulhi_pi16(psi_a_p5_p1, ONE_OVER_SQRT_2);
        psi_a_p5_p1 = _mm_slli_pi16(psi_a_p5_p1, 2);
        psi_a_p5_m1 = _mm_mulhi_pi16(psi_a_p5_m1, ONE_OVER_SQRT_2);
        psi_a_p5_m1 = _mm_slli_pi16(psi_a_p5_m1, 2);
        psi_a_p5_m3 = _mm_mulhi_pi16(psi_a_p5_m3, ONE_OVER_SQRT_2);
        psi_a_p5_m3 = _mm_slli_pi16(psi_a_p5_m3, 2);
        psi_a_p5_m5 = _mm_mulhi_pi16(psi_a_p5_m5, ONE_OVER_SQRT_2);
        psi_a_p5_m5 = _mm_slli_pi16(psi_a_p5_m5, 2);
        psi_a_p5_m7 = _mm_mulhi_pi16(psi_a_p5_m7, ONE_OVER_SQRT_2);
        psi_a_p5_m7 = _mm_slli_pi16(psi_a_p5_m7, 2);
        psi_a_p3_p7 = _mm_mulhi_pi16(psi_a_p3_p7, ONE_OVER_SQRT_2);
        psi_a_p3_p7 = _mm_slli_pi16(psi_a_p3_p7, 2);
        psi_a_p3_p5 = _mm_mulhi_pi16(psi_a_p3_p5, ONE_OVER_SQRT_2);
        psi_a_p3_p5 = _mm_slli_pi16(psi_a_p3_p5, 2);
        psi_a_p3_p3 = _mm_mulhi_pi16(psi_a_p3_p3, ONE_OVER_SQRT_2);
        psi_a_p3_p3 = _mm_slli_pi16(psi_a_p3_p3, 2);
        psi_a_p3_p1 = _mm_mulhi_pi16(psi_a_p3_p1, ONE_OVER_SQRT_2);
        psi_a_p3_p1 = _mm_slli_pi16(psi_a_p3_p1, 2);
        psi_a_p3_m1 = _mm_mulhi_pi16(psi_a_p3_m1, ONE_OVER_SQRT_2);
        psi_a_p3_m1 = _mm_slli_pi16(psi_a_p3_m1, 2);
        psi_a_p3_m3 = _mm_mulhi_pi16(psi_a_p3_m3, ONE_OVER_SQRT_2);
        psi_a_p3_m3 = _mm_slli_pi16(psi_a_p3_m3, 2);
        psi_a_p3_m5 = _mm_mulhi_pi16(psi_a_p3_m5, ONE_OVER_SQRT_2);
        psi_a_p3_m5 = _mm_slli_pi16(psi_a_p3_m5, 2);
        psi_a_p3_m7 = _mm_mulhi_pi16(psi_a_p3_m7, ONE_OVER_SQRT_2);
        psi_a_p3_m7 = _mm_slli_pi16(psi_a_p3_m7, 2);
        psi_a_p1_p7 = _mm_mulhi_pi16(psi_a_p1_p7, ONE_OVER_SQRT_2);
        psi_a_p1_p7 = _mm_slli_pi16(psi_a_p1_p7, 2);
        psi_a_p1_p5 = _mm_mulhi_pi16(psi_a_p1_p5, ONE_OVER_SQRT_2);
        psi_a_p1_p5 = _mm_slli_pi16(psi_a_p1_p5, 2);
        psi_a_p1_p3 = _mm_mulhi_pi16(psi_a_p1_p3, ONE_OVER_SQRT_2);
        psi_a_p1_p3 = _mm_slli_pi16(psi_a_p1_p3, 2);
        psi_a_p1_p1 = _mm_mulhi_pi16(psi_a_p1_p1, ONE_OVER_SQRT_2);
        psi_a_p1_p1 = _mm_slli_pi16(psi_a_p1_p1, 2);
        psi_a_p1_m1 = _mm_mulhi_pi16(psi_a_p1_m1, ONE_OVER_SQRT_2);
        psi_a_p1_m1 = _mm_slli_pi16(psi_a_p1_m1, 2);
        psi_a_p1_m3 = _mm_mulhi_pi16(psi_a_p1_m3, ONE_OVER_SQRT_2);
        psi_a_p1_m3 = _mm_slli_pi16(psi_a_p1_m3, 2);
        psi_a_p1_m5 = _mm_mulhi_pi16(psi_a_p1_m5, ONE_OVER_SQRT_2);
        psi_a_p1_m5 = _mm_slli_pi16(psi_a_p1_m5, 2);
        psi_a_p1_m7 = _mm_mulhi_pi16(psi_a_p1_m7, ONE_OVER_SQRT_2);
        psi_a_p1_m7 = _mm_slli_pi16(psi_a_p1_m7, 2);
        psi_a_m1_p7 = _mm_mulhi_pi16(psi_a_m1_p7, ONE_OVER_SQRT_2);
        psi_a_m1_p7 = _mm_slli_pi16(psi_a_m1_p7, 2);
        psi_a_m1_p5 = _mm_mulhi_pi16(psi_a_m1_p5, ONE_OVER_SQRT_2);
        psi_a_m1_p5 = _mm_slli_pi16(psi_a_m1_p5, 2);
        psi_a_m1_p3 = _mm_mulhi_pi16(psi_a_m1_p3, ONE_OVER_SQRT_2);
        psi_a_m1_p3 = _mm_slli_pi16(psi_a_m1_p3, 2);
        psi_a_m1_p1 = _mm_mulhi_pi16(psi_a_m1_p1, ONE_OVER_SQRT_2);
        psi_a_m1_p1 = _mm_slli_pi16(psi_a_m1_p1, 2);
        psi_a_m1_m1 = _mm_mulhi_pi16(psi_a_m1_m1, ONE_OVER_SQRT_2);
        psi_a_m1_m1 = _mm_slli_pi16(psi_a_m1_m1, 2);
        psi_a_m1_m3 = _mm_mulhi_pi16(psi_a_m1_m3, ONE_OVER_SQRT_2);
        psi_a_m1_m3 = _mm_slli_pi16(psi_a_m1_m3, 2);
        psi_a_m1_m5 = _mm_mulhi_pi16(psi_a_m1_m5, ONE_OVER_SQRT_2);
        psi_a_m1_m5 = _mm_slli_pi16(psi_a_m1_m5, 2);
        psi_a_m1_m7 = _mm_mulhi_pi16(psi_a_m1_m7, ONE_OVER_SQRT_2);
        psi_a_m1_m7 = _mm_slli_pi16(psi_a_m1_m7, 2);
        psi_a_m3_p7 = _mm_mulhi_pi16(psi_a_m3_p7, ONE_OVER_SQRT_2);
        psi_a_m3_p7 = _mm_slli_pi16(psi_a_m3_p7, 2);
        psi_a_m3_p5 = _mm_mulhi_pi16(psi_a_m3_p5, ONE_OVER_SQRT_2);
        psi_a_m3_p5 = _mm_slli_pi16(psi_a_m3_p5, 2);
        psi_a_m3_p3 = _mm_mulhi_pi16(psi_a_m3_p3, ONE_OVER_SQRT_2);
        psi_a_m3_p3 = _mm_slli_pi16(psi_a_m3_p3, 2);
        psi_a_m3_p1 = _mm_mulhi_pi16(psi_a_m3_p1, ONE_OVER_SQRT_2);
        psi_a_m3_p1 = _mm_slli_pi16(psi_a_m3_p1, 2);
        psi_a_m3_m1 = _mm_mulhi_pi16(psi_a_m3_m1, ONE_OVER_SQRT_2);
        psi_a_m3_m1 = _mm_slli_pi16(psi_a_m3_m1, 2);
        psi_a_m3_m3 = _mm_mulhi_pi16(psi_a_m3_m3, ONE_OVER_SQRT_2);
        psi_a_m3_m3 = _mm_slli_pi16(psi_a_m3_m3, 2);
        psi_a_m3_m5 = _mm_mulhi_pi16(psi_a_m3_m5, ONE_OVER_SQRT_2);
        psi_a_m3_m5 = _mm_slli_pi16(psi_a_m3_m5, 2);
        psi_a_m3_m7 = _mm_mulhi_pi16(psi_a_m3_m7, ONE_OVER_SQRT_2);
        psi_a_m3_m7 = _mm_slli_pi16(psi_a_m3_m7, 2);
        psi_a_m5_p7 = _mm_mulhi_pi16(psi_a_m5_p7, ONE_OVER_SQRT_2);
        psi_a_m5_p7 = _mm_slli_pi16(psi_a_m5_p7, 2);
        psi_a_m5_p5 = _mm_mulhi_pi16(psi_a_m5_p5, ONE_OVER_SQRT_2);
        psi_a_m5_p5 = _mm_slli_pi16(psi_a_m5_p5, 2);
        psi_a_m5_p3 = _mm_mulhi_pi16(psi_a_m5_p3, ONE_OVER_SQRT_2);
        psi_a_m5_p3 = _mm_slli_pi16(psi_a_m5_p3, 2);
        psi_a_m5_p1 = _mm_mulhi_pi16(psi_a_m5_p1, ONE_OVER_SQRT_2);
        psi_a_m5_p1 = _mm_slli_pi16(psi_a_m5_p1, 2);
        psi_a_m5_m1 = _mm_mulhi_pi16(psi_a_m5_m1, ONE_OVER_SQRT_2);
        psi_a_m5_m1 = _mm_slli_pi16(psi_a_m5_m1, 2);
        psi_a_m5_m3 = _mm_mulhi_pi16(psi_a_m5_m3, ONE_OVER_SQRT_2);
        psi_a_m5_m3 = _mm_slli_pi16(psi_a_m5_m3, 2);
        psi_a_m5_m5 = _mm_mulhi_pi16(psi_a_m5_m5, ONE_OVER_SQRT_2);
        psi_a_m5_m5 = _mm_slli_pi16(psi_a_m5_m5, 2);
        psi_a_m5_m7 = _mm_mulhi_pi16(psi_a_m5_m7, ONE_OVER_SQRT_2);
        psi_a_m5_m7 = _mm_slli_pi16(psi_a_m5_m7, 2);
        psi_a_m7_p7 = _mm_mulhi_pi16(psi_a_m7_p7, ONE_OVER_SQRT_2);
        psi_a_m7_p7 = _mm_slli_pi16(psi_a_m7_p7, 2);
        psi_a_m7_p5 = _mm_mulhi_pi16(psi_a_m7_p5, ONE_OVER_SQRT_2);
        psi_a_m7_p5 = _mm_slli_pi16(psi_a_m7_p5, 2);
        psi_a_m7_p3 = _mm_mulhi_pi16(psi_a_m7_p3, ONE_OVER_SQRT_2);
        psi_a_m7_p3 = _mm_slli_pi16(psi_a_m7_p3, 2);
        psi_a_m7_p1 = _mm_mulhi_pi16(psi_a_m7_p1, ONE_OVER_SQRT_2);
        psi_a_m7_p1 = _mm_slli_pi16(psi_a_m7_p1, 2);
        psi_a_m7_m1 = _mm_mulhi_pi16(psi_a_m7_m1, ONE_OVER_SQRT_2);
        psi_a_m7_m1 = _mm_slli_pi16(psi_a_m7_m1, 2);
        psi_a_m7_m3 = _mm_mulhi_pi16(psi_a_m7_m3, ONE_OVER_SQRT_2);
        psi_a_m7_m3 = _mm_slli_pi16(psi_a_m7_m3, 2);
        psi_a_m7_m5 = _mm_mulhi_pi16(psi_a_m7_m5, ONE_OVER_SQRT_2);
        psi_a_m7_m5 = _mm_slli_pi16(psi_a_m7_m5, 2);
        psi_a_m7_m7 = _mm_mulhi_pi16(psi_a_m7_m7, ONE_OVER_SQRT_2);
        psi_a_m7_m7 = _mm_slli_pi16(psi_a_m7_m7, 2);

        // Calculation of a group of two terms in the bit metric involving squares of interference
        ch_mag_int_direct = _mm_mulhi_pi16(ch_mag_int, SQRT_42_OVER_FOUR);
        ch_mag_int_direct = _mm_slli_pi16(ch_mag_int_direct, 3); // Q4.12
     
        square_a_64qam_pi16(a_r_p7_p7, a_i_p7_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_p7);
        square_a_64qam_pi16(a_r_p7_p5, a_i_p7_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_p5);
        square_a_64qam_pi16(a_r_p7_p3, a_i_p7_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_p3);
        square_a_64qam_pi16(a_r_p7_p1, a_i_p7_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_p1);
        square_a_64qam_pi16(a_r_p7_m1, a_i_p7_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_m1);
        square_a_64qam_pi16(a_r_p7_m3, a_i_p7_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_m3);
        square_a_64qam_pi16(a_r_p7_m5, a_i_p7_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_m5);
        square_a_64qam_pi16(a_r_p7_m7, a_i_p7_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p7_m7);
        square_a_64qam_pi16(a_r_p5_p7, a_i_p5_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_p7);
        square_a_64qam_pi16(a_r_p5_p5, a_i_p5_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_p5);
        square_a_64qam_pi16(a_r_p5_p3, a_i_p5_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_p3);
        square_a_64qam_pi16(a_r_p5_p1, a_i_p5_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_p1);
        square_a_64qam_pi16(a_r_p5_m1, a_i_p5_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_m1);
        square_a_64qam_pi16(a_r_p5_m3, a_i_p5_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_m3);
        square_a_64qam_pi16(a_r_p5_m5, a_i_p5_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_m5);
        square_a_64qam_pi16(a_r_p5_m7, a_i_p5_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p5_m7);
        square_a_64qam_pi16(a_r_p3_p7, a_i_p3_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_p7);
        square_a_64qam_pi16(a_r_p3_p5, a_i_p3_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_p5);
        square_a_64qam_pi16(a_r_p3_p3, a_i_p3_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_p3);
        square_a_64qam_pi16(a_r_p3_p1, a_i_p3_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_p1);
        square_a_64qam_pi16(a_r_p3_m1, a_i_p3_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_m1);
        square_a_64qam_pi16(a_r_p3_m3, a_i_p3_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_m3);
        square_a_64qam_pi16(a_r_p3_m5, a_i_p3_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_m5);
        square_a_64qam_pi16(a_r_p3_m7, a_i_p3_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p3_m7);
        square_a_64qam_pi16(a_r_p1_p7, a_i_p1_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p7);
        square_a_64qam_pi16(a_r_p1_p5, a_i_p1_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p5);
        square_a_64qam_pi16(a_r_p1_p3, a_i_p1_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p3);
        square_a_64qam_pi16(a_r_p1_p1, a_i_p1_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_p1);
        square_a_64qam_pi16(a_r_p1_m1, a_i_p1_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m1);
        square_a_64qam_pi16(a_r_p1_m3, a_i_p1_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m3);
        square_a_64qam_pi16(a_r_p1_m5, a_i_p1_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m5);
        square_a_64qam_pi16(a_r_p1_m7, a_i_p1_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_p1_m7);
        square_a_64qam_pi16(a_r_m1_p7, a_i_m1_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p7);
        square_a_64qam_pi16(a_r_m1_p5, a_i_m1_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p5);
        square_a_64qam_pi16(a_r_m1_p3, a_i_m1_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p3);
        square_a_64qam_pi16(a_r_m1_p1, a_i_m1_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_p1);
        square_a_64qam_pi16(a_r_m1_m1, a_i_m1_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m1);
        square_a_64qam_pi16(a_r_m1_m3, a_i_m1_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m3);
        square_a_64qam_pi16(a_r_m1_m5, a_i_m1_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m5);
        square_a_64qam_pi16(a_r_m1_m7, a_i_m1_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m1_m7);
        square_a_64qam_pi16(a_r_m3_p7, a_i_m3_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_p7);
        square_a_64qam_pi16(a_r_m3_p5, a_i_m3_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_p5);
        square_a_64qam_pi16(a_r_m3_p3, a_i_m3_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_p3);
        square_a_64qam_pi16(a_r_m3_p1, a_i_m3_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_p1);
        square_a_64qam_pi16(a_r_m3_m1, a_i_m3_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_m1);
        square_a_64qam_pi16(a_r_m3_m3, a_i_m3_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_m3);
        square_a_64qam_pi16(a_r_m3_m5, a_i_m3_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_m5);
        square_a_64qam_pi16(a_r_m3_m7, a_i_m3_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m3_m7);
        square_a_64qam_pi16(a_r_m5_p7, a_i_m5_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_p7);
        square_a_64qam_pi16(a_r_m5_p5, a_i_m5_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_p5);
        square_a_64qam_pi16(a_r_m5_p3, a_i_m5_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_p3);
        square_a_64qam_pi16(a_r_m5_p1, a_i_m5_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_p1);
        square_a_64qam_pi16(a_r_m5_m1, a_i_m5_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_m1);
        square_a_64qam_pi16(a_r_m5_m3, a_i_m5_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_m3);
        square_a_64qam_pi16(a_r_m5_m5, a_i_m5_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_m5);
        square_a_64qam_pi16(a_r_m5_m7, a_i_m5_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m5_m7);
        square_a_64qam_pi16(a_r_m7_p7, a_i_m7_p7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_p7);
        square_a_64qam_pi16(a_r_m7_p5, a_i_m7_p5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_p5);
        square_a_64qam_pi16(a_r_m7_p3, a_i_m7_p3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_p3);
        square_a_64qam_pi16(a_r_m7_p1, a_i_m7_p1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_p1);
        square_a_64qam_pi16(a_r_m7_m1, a_i_m7_m1, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_m1);
        square_a_64qam_pi16(a_r_m7_m3, a_i_m7_m3, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_m3);
        square_a_64qam_pi16(a_r_m7_m5, a_i_m7_m5, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_m5);
        square_a_64qam_pi16(a_r_m7_m7, a_i_m7_m7, ch_mag_int, SQRT_42_OVER_FOUR, a_sq_m7_m7);

        // Computing different multiples of ||h0||^2
        // x=1, y=1
        ch_mag_2_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,ONE_OVER_FOUR_SQRT_42);
        ch_mag_2_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_2_over_42_with_sigma2,1);
        // x=1, y=3
        ch_mag_10_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,FIVE_OVER_FOUR_SQRT_42);
        ch_mag_10_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_10_over_42_with_sigma2,1);
        // x=1, x=5
        ch_mag_26_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,THIRTEEN_OVER_FOUR_SQRT_42);
        ch_mag_26_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_26_over_42_with_sigma2,1);
        // x=1, y=7
        ch_mag_50_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,TWENTYFIVE_OVER_FOUR_SQRT_42);
        ch_mag_50_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_50_over_42_with_sigma2,1);
        // x=3, y=3
        ch_mag_18_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,NINE_OVER_FOUR_SQRT_42);
        ch_mag_18_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_18_over_42_with_sigma2,1);
        // x=3, y=5
        ch_mag_34_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,SEVENTEEN_OVER_FOUR_SQRT_42);
        ch_mag_34_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_34_over_42_with_sigma2,1);
        // x=3, y=7
        ch_mag_58_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,TWENTYNINE_OVER_FOUR_SQRT_42);
        ch_mag_58_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_58_over_42_with_sigma2,2);
        // x=5, y=5
        ch_mag_50_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,TWENTYFIVE_OVER_FOUR_SQRT_42);
        ch_mag_50_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_50_over_42_with_sigma2,1);
        // x=5, y=7
        ch_mag_74_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,THIRTYSEVEN_OVER_FOUR_SQRT_42);
        ch_mag_74_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_74_over_42_with_sigma2,2);
        // x=7, y=7
        ch_mag_98_over_42_with_sigma2 = _mm_mulhi_pi16(ch_mag_des,FORTYNINE_OVER_FOUR_SQRT_42);
        ch_mag_98_over_42_with_sigma2 = _mm_slli_pi16(ch_mag_98_over_42_with_sigma2,2);
        
        // Computing Metrics
        xmm0 = _mm_subs_pi16(psi_a_p7_p7, a_sq_p7_p7); xmm1 = _mm_adds_pi16(xmm0, y0_p_7_7); bit_met_p7_p7 = _mm_subs_pi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p7_p5, a_sq_p7_p5); xmm1 = _mm_adds_pi16(xmm0, y0_p_7_5); bit_met_p7_p5 = _mm_subs_pi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p7_p3, a_sq_p7_p3); xmm1 = _mm_adds_pi16(xmm0, y0_p_7_3); bit_met_p7_p3 = _mm_subs_pi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p7_p1, a_sq_p7_p1); xmm1 = _mm_adds_pi16(xmm0, y0_p_7_1); bit_met_p7_p1 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p7_m1, a_sq_p7_m1); xmm1 = _mm_adds_pi16(xmm0, y0_m_7_1); bit_met_p7_m1 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p7_m3, a_sq_p7_m3); xmm1 = _mm_adds_pi16(xmm0, y0_m_7_3); bit_met_p7_m3 = _mm_subs_pi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p7_m5, a_sq_p7_m5); xmm1 = _mm_adds_pi16(xmm0, y0_m_7_5); bit_met_p7_m5 = _mm_subs_pi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p7_m7, a_sq_p7_m7); xmm1 = _mm_adds_pi16(xmm0, y0_m_7_7); bit_met_p7_m7 = _mm_subs_pi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p5_p7, a_sq_p5_p7); xmm1 = _mm_adds_pi16(xmm0, y0_p_5_7); bit_met_p5_p7 = _mm_subs_pi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p5_p5, a_sq_p5_p5); xmm1 = _mm_adds_pi16(xmm0, y0_p_5_5); bit_met_p5_p5 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p5_p3, a_sq_p5_p3); xmm1 = _mm_adds_pi16(xmm0, y0_p_5_3); bit_met_p5_p3 = _mm_subs_pi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p5_p1, a_sq_p5_p1); xmm1 = _mm_adds_pi16(xmm0, y0_p_5_1); bit_met_p5_p1 = _mm_subs_pi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p5_m1, a_sq_p5_m1); xmm1 = _mm_adds_pi16(xmm0, y0_m_5_1); bit_met_p5_m1 = _mm_subs_pi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p5_m3, a_sq_p5_m3); xmm1 = _mm_adds_pi16(xmm0, y0_m_5_3); bit_met_p5_m3 = _mm_subs_pi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p5_m5, a_sq_p5_m5); xmm1 = _mm_adds_pi16(xmm0, y0_m_5_5); bit_met_p5_m5 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p5_m7, a_sq_p5_m7); xmm1 = _mm_adds_pi16(xmm0, y0_m_5_7); bit_met_p5_m7 = _mm_subs_pi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p3_p7, a_sq_p3_p7); xmm1 = _mm_adds_pi16(xmm0, y0_p_3_7); bit_met_p3_p7 = _mm_subs_pi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p3_p5, a_sq_p3_p5); xmm1 = _mm_adds_pi16(xmm0, y0_p_3_5); bit_met_p3_p5 = _mm_subs_pi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p3_p3, a_sq_p3_p3); xmm1 = _mm_adds_pi16(xmm0, y0_p_3_3); bit_met_p3_p3 = _mm_subs_pi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p3_p1, a_sq_p3_p1); xmm1 = _mm_adds_pi16(xmm0, y0_p_3_1); bit_met_p3_p1 = _mm_subs_pi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p3_m1, a_sq_p3_m1); xmm1 = _mm_adds_pi16(xmm0, y0_m_3_1); bit_met_p3_m1 = _mm_subs_pi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p3_m3, a_sq_p3_m3); xmm1 = _mm_adds_pi16(xmm0, y0_m_3_3); bit_met_p3_m3 = _mm_subs_pi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p3_m5, a_sq_p3_m5); xmm1 = _mm_adds_pi16(xmm0, y0_m_3_5); bit_met_p3_m5 = _mm_subs_pi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p3_m7, a_sq_p3_m7); xmm1 = _mm_adds_pi16(xmm0, y0_m_3_7); bit_met_p3_m7 = _mm_subs_pi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p1_p7, a_sq_p1_p7); xmm1 = _mm_adds_pi16(xmm0, y0_p_1_7); bit_met_p1_p7 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p1_p5, a_sq_p1_p5); xmm1 = _mm_adds_pi16(xmm0, y0_p_1_5); bit_met_p1_p5 = _mm_subs_pi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p1_p3, a_sq_p1_p3); xmm1 = _mm_adds_pi16(xmm0, y0_p_1_3); bit_met_p1_p3 = _mm_subs_pi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p1_p1, a_sq_p1_p1); xmm1 = _mm_adds_pi16(xmm0, y0_p_1_1); bit_met_p1_p1 = _mm_subs_pi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p1_m1, a_sq_p1_m1); xmm1 = _mm_adds_pi16(xmm0, y0_m_1_1); bit_met_p1_m1 = _mm_subs_pi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p1_m3, a_sq_p1_m3); xmm1 = _mm_adds_pi16(xmm0, y0_m_1_3); bit_met_p1_m3 = _mm_subs_pi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p1_m5, a_sq_p1_m5); xmm1 = _mm_adds_pi16(xmm0, y0_m_1_5); bit_met_p1_m5 = _mm_subs_pi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_p1_m7, a_sq_p1_m7); xmm1 = _mm_adds_pi16(xmm0, y0_m_1_7); bit_met_p1_m7 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);

        xmm0 = _mm_subs_pi16(psi_a_m1_p7, a_sq_m1_p7); xmm1 = _mm_subs_pi16(xmm0, y0_m_1_7); bit_met_m1_p7 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m1_p5, a_sq_m1_p5); xmm1 = _mm_subs_pi16(xmm0, y0_m_1_5); bit_met_m1_p5 = _mm_subs_pi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m1_p3, a_sq_m1_p3); xmm1 = _mm_subs_pi16(xmm0, y0_m_1_3); bit_met_m1_p3 = _mm_subs_pi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m1_p1, a_sq_m1_p1); xmm1 = _mm_subs_pi16(xmm0, y0_m_1_1); bit_met_m1_p1 = _mm_subs_pi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m1_m1, a_sq_m1_m1); xmm1 = _mm_subs_pi16(xmm0, y0_p_1_1); bit_met_m1_m1 = _mm_subs_pi16(xmm1, ch_mag_2_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m1_m3, a_sq_m1_m3); xmm1 = _mm_subs_pi16(xmm0, y0_p_1_3); bit_met_m1_m3 = _mm_subs_pi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m1_m5, a_sq_m1_m5); xmm1 = _mm_subs_pi16(xmm0, y0_p_1_5); bit_met_m1_m5 = _mm_subs_pi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m1_m7, a_sq_m1_m7); xmm1 = _mm_subs_pi16(xmm0, y0_p_1_7); bit_met_m1_m7 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m3_p7, a_sq_m3_p7); xmm1 = _mm_subs_pi16(xmm0, y0_m_3_7); bit_met_m3_p7 = _mm_subs_pi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m3_p5, a_sq_m3_p5); xmm1 = _mm_subs_pi16(xmm0, y0_m_3_5); bit_met_m3_p5 = _mm_subs_pi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m3_p3, a_sq_m3_p3); xmm1 = _mm_subs_pi16(xmm0, y0_m_3_3); bit_met_m3_p3 = _mm_subs_pi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m3_p1, a_sq_m3_p1); xmm1 = _mm_subs_pi16(xmm0, y0_m_3_1); bit_met_m3_p1 = _mm_subs_pi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m3_m1, a_sq_m3_m1); xmm1 = _mm_subs_pi16(xmm0, y0_p_3_1); bit_met_m3_m1 = _mm_subs_pi16(xmm1, ch_mag_10_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m3_m3, a_sq_m3_m3); xmm1 = _mm_subs_pi16(xmm0, y0_p_3_3); bit_met_m3_m3 = _mm_subs_pi16(xmm1, ch_mag_18_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m3_m5, a_sq_m3_m5); xmm1 = _mm_subs_pi16(xmm0, y0_p_3_5); bit_met_m3_m5 = _mm_subs_pi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m3_m7, a_sq_m3_m7); xmm1 = _mm_subs_pi16(xmm0, y0_p_3_7); bit_met_m3_m7 = _mm_subs_pi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m5_p7, a_sq_m5_p7); xmm1 = _mm_subs_pi16(xmm0, y0_m_5_7); bit_met_m5_p7 = _mm_subs_pi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m5_p5, a_sq_m5_p5); xmm1 = _mm_subs_pi16(xmm0, y0_m_5_5); bit_met_m5_p5 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m5_p3, a_sq_m5_p3); xmm1 = _mm_subs_pi16(xmm0, y0_m_5_3); bit_met_m5_p3 = _mm_subs_pi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m5_p1, a_sq_m5_p1); xmm1 = _mm_subs_pi16(xmm0, y0_m_5_1); bit_met_m5_p1 = _mm_subs_pi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m5_m1, a_sq_m5_m1); xmm1 = _mm_subs_pi16(xmm0, y0_p_5_1); bit_met_m5_m1 = _mm_subs_pi16(xmm1, ch_mag_26_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m5_m3, a_sq_m5_m3); xmm1 = _mm_subs_pi16(xmm0, y0_p_5_3); bit_met_m5_m3 = _mm_subs_pi16(xmm1, ch_mag_34_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m5_m5, a_sq_m5_m5); xmm1 = _mm_subs_pi16(xmm0, y0_p_5_5); bit_met_m5_m5 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m5_m7, a_sq_m5_m7); xmm1 = _mm_subs_pi16(xmm0, y0_p_5_7); bit_met_m5_m7 = _mm_subs_pi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m7_p7, a_sq_m7_p7); xmm1 = _mm_subs_pi16(xmm0, y0_m_7_7); bit_met_m7_p7 = _mm_subs_pi16(xmm1, ch_mag_98_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m7_p5, a_sq_m7_p5); xmm1 = _mm_subs_pi16(xmm0, y0_m_7_5); bit_met_m7_p5 = _mm_subs_pi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m7_p3, a_sq_m7_p3); xmm1 = _mm_subs_pi16(xmm0, y0_m_7_3); bit_met_m7_p3 = _mm_subs_pi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m7_p1, a_sq_m7_p1); xmm1 = _mm_subs_pi16(xmm0, y0_m_7_1); bit_met_m7_p1 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m7_m1, a_sq_m7_m1); xmm1 = _mm_subs_pi16(xmm0, y0_p_7_1); bit_met_m7_m1 = _mm_subs_pi16(xmm1, ch_mag_50_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m7_m3, a_sq_m7_m3); xmm1 = _mm_subs_pi16(xmm0, y0_p_7_3); bit_met_m7_m3 = _mm_subs_pi16(xmm1, ch_mag_58_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m7_m5, a_sq_m7_m5); xmm1 = _mm_subs_pi16(xmm0, y0_p_7_5); bit_met_m7_m5 = _mm_subs_pi16(xmm1, ch_mag_74_over_42_with_sigma2);
        xmm0 = _mm_subs_pi16(psi_a_m7_m7, a_sq_m7_m7); xmm1 = _mm_subs_pi16(xmm0, y0_p_7_7); bit_met_m7_m7 = _mm_subs_pi16(xmm1, ch_mag_98_over_42_with_sigma2);

        // Detection for 1st bit (LTE mapping)
        // bit = 1 
        xmm0 = _mm_max_pi16(bit_met_m7_p7, bit_met_m7_p5);
        xmm1 = _mm_max_pi16(bit_met_m7_p3, bit_met_m7_p1);
        xmm2 = _mm_max_pi16(bit_met_m7_m1, bit_met_m7_m3);
        xmm3 = _mm_max_pi16(bit_met_m7_m5, bit_met_m7_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(xmm4, xmm5);        
        xmm0 = _mm_max_pi16(bit_met_m5_p7, bit_met_m5_p5);
        xmm1 = _mm_max_pi16(bit_met_m5_p3, bit_met_m5_p1);
        xmm2 = _mm_max_pi16(bit_met_m5_m1, bit_met_m5_m3);
        xmm3 = _mm_max_pi16(bit_met_m5_m5, bit_met_m5_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_m3_p7, bit_met_m3_p5);
        xmm1 = _mm_max_pi16(bit_met_m3_p3, bit_met_m3_p1);
        xmm2 = _mm_max_pi16(bit_met_m3_m1, bit_met_m3_m3);
        xmm3 = _mm_max_pi16(bit_met_m3_m5, bit_met_m3_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_m1_p7, bit_met_m1_p5);
        xmm1 = _mm_max_pi16(bit_met_m1_p3, bit_met_m1_p1);
        xmm2 = _mm_max_pi16(bit_met_m1_m1, bit_met_m1_m3);
        xmm3 = _mm_max_pi16(bit_met_m1_m5, bit_met_m1_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        
        // bit = 0
        xmm0 = _mm_max_pi16(bit_met_p7_p7, bit_met_p7_p5);
        xmm1 = _mm_max_pi16(bit_met_p7_p3, bit_met_p7_p1);
        xmm2 = _mm_max_pi16(bit_met_p7_m1, bit_met_p7_m3);
        xmm3 = _mm_max_pi16(bit_met_p7_m5, bit_met_p7_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p5_p7, bit_met_p5_p5);
        xmm1 = _mm_max_pi16(bit_met_p5_p3, bit_met_p5_p1);
        xmm2 = _mm_max_pi16(bit_met_p5_m1, bit_met_p5_m3);
        xmm3 = _mm_max_pi16(bit_met_p5_m5, bit_met_p5_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p3_p7, bit_met_p3_p5);
        xmm1 = _mm_max_pi16(bit_met_p3_p3, bit_met_p3_p1);
        xmm2 = _mm_max_pi16(bit_met_p3_m1, bit_met_p3_m3);
        xmm3 = _mm_max_pi16(bit_met_p3_m5, bit_met_p3_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p1_p7, bit_met_p1_p5);
        xmm1 = _mm_max_pi16(bit_met_p1_p3, bit_met_p1_p1);
        xmm2 = _mm_max_pi16(bit_met_p1_m1, bit_met_p1_m3);
        xmm3 = _mm_max_pi16(bit_met_p1_m5, bit_met_p1_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        
        y0r = _mm_subs_pi16(logmax_num_re0, logmax_den_re0);
	 
        // Detection for 2nd bit (LTE mapping)
        // bit = 1
        xmm0 = _mm_max_pi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_pi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_pi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_pi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_pi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_pi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_pi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_pi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_pi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_pi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_pi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_pi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_pi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        
        // bit = 0
        xmm0 = _mm_max_pi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_pi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_pi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_pi16(bit_met_m5_p1, bit_met_m7_p1);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_pi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_pi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_pi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_pi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_pi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_pi16(bit_met_m5_p5, bit_met_m7_p5);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_pi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_pi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_pi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);

        y1r = _mm_subs_pi16(logmax_num_re0, logmax_den_re0);
     
        // Detection for 3rd bit (LTE mapping)
        xmm0 = _mm_max_pi16(bit_met_m7_m7, bit_met_m7_m5);
        xmm1 = _mm_max_pi16(bit_met_m7_m3, bit_met_m7_m1);
        xmm2 = _mm_max_pi16(bit_met_m7_p1, bit_met_m7_p3);
        xmm3 = _mm_max_pi16(bit_met_m7_p5, bit_met_m7_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_m5_m7, bit_met_m5_m5);
        xmm1 = _mm_max_pi16(bit_met_m5_m3, bit_met_m5_m1);
        xmm2 = _mm_max_pi16(bit_met_m5_p1, bit_met_m5_p3);
        xmm3 = _mm_max_pi16(bit_met_m5_p5, bit_met_m5_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p5_m7, bit_met_p5_m5);
        xmm1 = _mm_max_pi16(bit_met_p5_m3, bit_met_p5_m1);
        xmm2 = _mm_max_pi16(bit_met_p5_p1, bit_met_p5_p3);
        xmm3 = _mm_max_pi16(bit_met_p5_p5, bit_met_p5_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m7, bit_met_p7_m5);
        xmm1 = _mm_max_pi16(bit_met_p7_m3, bit_met_p7_m1);
        xmm2 = _mm_max_pi16(bit_met_p7_p1, bit_met_p7_p3);
        xmm3 = _mm_max_pi16(bit_met_p7_p5, bit_met_p7_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_pi16(bit_met_m3_m7, bit_met_m3_m5);
        xmm1 = _mm_max_pi16(bit_met_m3_m3, bit_met_m3_m1);
        xmm2 = _mm_max_pi16(bit_met_m3_p1, bit_met_m3_p3);
        xmm3 = _mm_max_pi16(bit_met_m3_p5, bit_met_m3_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_m1_m7, bit_met_m1_m5);
        xmm1 = _mm_max_pi16(bit_met_m1_m3, bit_met_m1_m1);
        xmm2 = _mm_max_pi16(bit_met_m1_p1, bit_met_m1_p3);
        xmm3 = _mm_max_pi16(bit_met_m1_p5, bit_met_m1_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p1_m7, bit_met_p1_m5);
        xmm1 = _mm_max_pi16(bit_met_p1_m3, bit_met_p1_m1);
        xmm2 = _mm_max_pi16(bit_met_p1_p1, bit_met_p1_p3);
        xmm3 = _mm_max_pi16(bit_met_p1_p5, bit_met_p1_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p3_m7, bit_met_p3_m5);
        xmm1 = _mm_max_pi16(bit_met_p3_m3, bit_met_p3_m1);
        xmm2 = _mm_max_pi16(bit_met_p3_p1, bit_met_p3_p3);
        xmm3 = _mm_max_pi16(bit_met_p3_p5, bit_met_p3_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);

        y2r = _mm_subs_pi16(logmax_num_re0, logmax_den_re0);

        // Detection for 4th bit (LTE mapping)
        xmm0 = _mm_max_pi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_pi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_pi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_pi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_pi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_pi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_pi16(bit_met_m5_p5, bit_met_m5_p5);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_pi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_pi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_pi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_pi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_pi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_pi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_pi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_pi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_pi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_pi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_pi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_pi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_pi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_pi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_pi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_pi16(bit_met_m5_p1, bit_met_m7_p1);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_pi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_pi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_pi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);

        y0i = _mm_subs_pi16(logmax_num_re0, logmax_den_re0);


        // Detection for 5th bit (LTE mapping)
        xmm0 = _mm_max_pi16(bit_met_m7_m7, bit_met_m7_m5);
        xmm1 = _mm_max_pi16(bit_met_m7_m3, bit_met_m7_m1);
        xmm2 = _mm_max_pi16(bit_met_m7_p1, bit_met_m7_p3);
        xmm3 = _mm_max_pi16(bit_met_m7_p5, bit_met_m7_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_m1_m7, bit_met_m1_m5);
        xmm1 = _mm_max_pi16(bit_met_m1_m3, bit_met_m1_m1);
        xmm2 = _mm_max_pi16(bit_met_m1_p1, bit_met_m1_p3);
        xmm3 = _mm_max_pi16(bit_met_m1_p5, bit_met_m1_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p1_m7, bit_met_p1_m5);
        xmm1 = _mm_max_pi16(bit_met_p1_m3, bit_met_p1_m1);
        xmm2 = _mm_max_pi16(bit_met_p1_p1, bit_met_p1_p3);
        xmm3 = _mm_max_pi16(bit_met_p1_p5, bit_met_p1_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m7, bit_met_p7_m5);
        xmm1 = _mm_max_pi16(bit_met_p7_m3, bit_met_p7_m1);
        xmm2 = _mm_max_pi16(bit_met_p7_p1, bit_met_p7_p3);
        xmm3 = _mm_max_pi16(bit_met_p7_p5, bit_met_p7_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_pi16(bit_met_m5_m7, bit_met_m5_m5);
        xmm1 = _mm_max_pi16(bit_met_m5_m3, bit_met_m5_m1);
        xmm2 = _mm_max_pi16(bit_met_m5_p1, bit_met_m5_p3);
        xmm3 = _mm_max_pi16(bit_met_m5_p5, bit_met_m5_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_m3_m7, bit_met_m3_m5);
        xmm1 = _mm_max_pi16(bit_met_m3_m3, bit_met_m3_m1);
        xmm2 = _mm_max_pi16(bit_met_m3_p1, bit_met_m3_p3);
        xmm3 = _mm_max_pi16(bit_met_m3_p5, bit_met_m3_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p3_m7, bit_met_p3_m5);
        xmm1 = _mm_max_pi16(bit_met_p3_m3, bit_met_p3_m1);
        xmm2 = _mm_max_pi16(bit_met_p3_p1, bit_met_p3_p3);
        xmm3 = _mm_max_pi16(bit_met_p3_p5, bit_met_p3_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p5_m7, bit_met_p5_m5);
        xmm1 = _mm_max_pi16(bit_met_p5_m3, bit_met_p5_m1);
        xmm2 = _mm_max_pi16(bit_met_p5_p1, bit_met_p5_p3);
        xmm3 = _mm_max_pi16(bit_met_p5_p5, bit_met_p5_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);

        y1i = _mm_subs_pi16(logmax_num_re0, logmax_den_re0);

        // Detection for 6th bit (LTE mapping)
        xmm0 = _mm_max_pi16(bit_met_p7_p7, bit_met_p5_p7);
        xmm1 = _mm_max_pi16(bit_met_p3_p7, bit_met_p1_p7);
        xmm2 = _mm_max_pi16(bit_met_m1_p7, bit_met_m3_p7);
        xmm3 = _mm_max_pi16(bit_met_m5_p7, bit_met_m7_p7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p1, bit_met_p5_p1);
        xmm1 = _mm_max_pi16(bit_met_p3_p1, bit_met_p1_p1);
        xmm2 = _mm_max_pi16(bit_met_m1_p1, bit_met_m3_p1);
        xmm3 = _mm_max_pi16(bit_met_m5_p1, bit_met_m5_p1);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m1, bit_met_p5_m1);
        xmm1 = _mm_max_pi16(bit_met_p3_m1, bit_met_p1_m1);
        xmm2 = _mm_max_pi16(bit_met_m1_m1, bit_met_m3_m1);
        xmm3 = _mm_max_pi16(bit_met_m5_m1, bit_met_m7_m1);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m7, bit_met_p5_m7);
        xmm1 = _mm_max_pi16(bit_met_p3_m7, bit_met_p1_m7);
        xmm2 = _mm_max_pi16(bit_met_m1_m7, bit_met_m3_m7);
        xmm3 = _mm_max_pi16(bit_met_m5_m7, bit_met_m7_m7);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm4);
        logmax_den_re0 = _mm_max_pi16(logmax_den_re0, xmm5);

        xmm0 = _mm_max_pi16(bit_met_p7_m5, bit_met_p5_m5);
        xmm1 = _mm_max_pi16(bit_met_p3_m5, bit_met_p1_m5);
        xmm2 = _mm_max_pi16(bit_met_m1_m5, bit_met_m3_m5);
        xmm3 = _mm_max_pi16(bit_met_m5_m5, bit_met_m7_m5);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(xmm4, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_m3, bit_met_p5_m3);
        xmm1 = _mm_max_pi16(bit_met_p3_m3, bit_met_p1_m3);
        xmm2 = _mm_max_pi16(bit_met_m1_m3, bit_met_m3_m3);
        xmm3 = _mm_max_pi16(bit_met_m5_m3, bit_met_m7_m3);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p3, bit_met_p5_p3);
        xmm1 = _mm_max_pi16(bit_met_p3_p3, bit_met_p1_p3);
        xmm2 = _mm_max_pi16(bit_met_m1_p3, bit_met_m3_p3);
        xmm3 = _mm_max_pi16(bit_met_m5_p3, bit_met_m7_p3);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);
        xmm0 = _mm_max_pi16(bit_met_p7_p5, bit_met_p5_p5);
        xmm1 = _mm_max_pi16(bit_met_p3_p5, bit_met_p1_p5);
        xmm2 = _mm_max_pi16(bit_met_m1_p5, bit_met_m3_p5);
        xmm3 = _mm_max_pi16(bit_met_m5_p5, bit_met_m7_p5);
        xmm4 = _mm_max_pi16(xmm0, xmm1);
        xmm5 = _mm_max_pi16(xmm2, xmm3);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm4);
        logmax_num_re0 = _mm_max_pi16(logmax_num_re0, xmm5);

        y2i = _mm_subs_pi16(logmax_num_re0, logmax_den_re0);

        // map to output stream, difficult to do in SIMD since we have 6 16bit LLRs
        // RE 1
        stream0_out[12*i + 0] = ((short *)&y0r)[0];
        stream0_out[12*i + 1] = ((short *)&y1r)[0];
        stream0_out[12*i + 2] = ((short *)&y2r)[0];
        stream0_out[12*i + 3] = ((short *)&y0i)[0];
        stream0_out[12*i + 4] = ((short *)&y1i)[0];
        stream0_out[12*i + 5] = ((short *)&y2i)[0];
        // RE 2
        stream0_out[12*i + 6] = ((short *)&y0r)[1];
        stream0_out[12*i + 7] = ((short *)&y1r)[1];
        stream0_out[12*i + 8] = ((short *)&y2r)[1];
        stream0_out[12*i + 9] = ((short *)&y0i)[1];
        stream0_out[12*i + 10] = ((short *)&y1i)[1];
        stream0_out[12*i + 11] = ((short *)&y2i)[1];
        // RE 3
        stream0_out[12*i + 12] = ((short *)&y0r)[2];
        stream0_out[12*i + 13] = ((short *)&y1r)[2];
        stream0_out[12*i + 14] = ((short *)&y2r)[2];
        stream0_out[12*i + 15] = ((short *)&y0i)[2];
        stream0_out[12*i + 16] = ((short *)&y1i)[2];
        stream0_out[12*i + 17] = ((short *)&y2i)[2];
        // RE 4
        stream0_out[12*i + 18] = ((short *)&y0r)[3];
        stream0_out[12*i + 19] = ((short *)&y1r)[3];
        stream0_out[12*i + 20] = ((short *)&y2r)[3];
        stream0_out[12*i + 21] = ((short *)&y0i)[3];
        stream0_out[12*i + 22] = ((short *)&y1i)[3];
        stream0_out[12*i + 23] = ((short *)&y2i)[3];
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

    //first symbol has different structure due to more pilots
    if (first_symbol_flag == 1) {
        llr16 = (s16*)dlsch_llr;
    }
    else {
        llr16 = (s16*)(*llr16p);
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
