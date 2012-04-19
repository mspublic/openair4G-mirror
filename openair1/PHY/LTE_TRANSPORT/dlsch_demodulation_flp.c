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

/*! \file PHY/LTE_TRANSPORT/dlsch_demodulation_flp.c
* \brief Top-level routines for demodulating the PDSCH physical channel from 36-211, V8.6 2009-03
* \author R. Knopp, F. Kaltenberger,A. Bhamri, S. Aubert
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr,ankit.bhamri@eurecom.fr,sebastien.aubert@eurecom.fr
* \note
* \warning
*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"

void llrFlp2Fxp(float *llr_flp,
		short *llr,
		int length)
{
  int i;

  /*
  msg("llr before normalization forall=[");
  for (i=0; i<length; i++)
    msg("%f(%d),", llr_flp[i], i);
  msg("]\n");
  */

  float max_llr_abs = 0.0;

  // Find max absolute value over all the LLRs values
  for (i=0; i<length; i++)
    {
      if (fabs(llr_flp[i])>max_llr_abs)
	max_llr_abs = fabs(llr_flp[i]);
    }
  
  // Normalize by the max value, thus max/min value is +1/-1
  //for (i=0; i<length; i++)
  //  llr_flp[i] = llr_flp[i]/max_llr;
  
  // Convert to Fxp, data that feed the turbo decoder are stored on 1 byte (-2^7:2^7-1 => min=-2^7 OR max=2^7-1)
  for (i=0; i<length; i++)
    llr[i] = (short)(floor(llr_flp[i]/max_llr_abs*(pow(2, 7)-1)));

  /*
  msg("llr after normalization forall=[");
  for (i=0; i<length; i++)
    msg("%d(%d),", llr[i], i);
  msg("]\n");
  */
}

void qam16_qam16_mu_mimo_full_flp(double *stream0_in, // From rxdataF_comp
				  double *stream1_in,
				  double *ch_mag,
				  double *ch_mag_i,
				  short *stream0_out, // Back to Fxp format
				  double *rho01,
				  int length)
{  
  /*
  msg("\nstream0[i] (h1'*y)=[%f,%f,%f,%f]\n", stream0_in[0], stream0_in[1], stream0_in[2], stream0_in[3]);
  msg("stream1[i] (h2'*y)=[%f,%f,%f,%f]\n", stream1_in[0], stream1_in[1], stream1_in[2], stream1_in[3]);
  msg("ch_mag[i] (2*||h1||^2/sqrt(10))=  [%f,%f,%f,%f]\n", ch_mag[0], ch_mag[1], ch_mag[2], ch_mag[3]);
  msg("ch_mag_i[i] (2*||h2||^2/sqrt(10))=[%f,%f,%f,%f]\n", ch_mag_i[0], ch_mag_i[1], ch_mag_i[2], ch_mag_i[3]);
  msg("rho[i] (h1'*h2)=[%f,%f,%f,%f]\n\n", rho01[0], rho01[1], rho01[2], rho01[3]);
  */

  /*
  if (0) // NOK, new version
  {
  int i,j;
  double rho_re[4];
  double rho_im[4];
  double y0_re[4];
  double y0_im[4];
  double y1_re[4];
  double y1_im[4];
  double two_h1_square_over_sqrt_10[4];
  double two_h2_square_over_sqrt_10[4];

  double psi_r_p1_p1[4]; double psi_i_p1_p1[4];
  double psi_r_p1_p3[4]; double psi_i_p1_p3[4];
  double psi_r_p1_m1[4]; double psi_i_p1_m1[4];
  double psi_r_p1_m3[4]; double psi_i_p1_m3[4];
  double psi_r_p3_p1[4]; double psi_i_p3_p1[4];
  double psi_r_p3_p3[4]; double psi_i_p3_p3[4];
  double psi_r_p3_m1[4]; double psi_i_p3_m1[4];
  double psi_r_p3_m3[4]; double psi_i_p3_m3[4];
  double psi_r_m1_p1[4]; double psi_i_m1_p1[4];
  double psi_r_m1_p3[4]; double psi_i_m1_p3[4];
  double psi_r_m1_m1[4]; double psi_i_m1_m1[4];
  double psi_r_m1_m3[4]; double psi_i_m1_m3[4];
  double psi_r_m3_p1[4]; double psi_i_m3_p1[4];
  double psi_r_m3_p3[4]; double psi_i_m3_p3[4];
  double psi_r_m3_m1[4]; double psi_i_m3_m1[4];
  double psi_r_m3_m3[4]; double psi_i_m3_m3[4];

  double a_r_p1_p1[4]; double a_i_p1_p1[4];
  double a_r_p1_p3[4]; double a_i_p1_p3[4];
  double a_r_p1_m1[4]; double a_i_p1_m1[4];
  double a_r_p1_m3[4]; double a_i_p1_m3[4];
  double a_r_p3_p1[4]; double a_i_p3_p1[4];
  double a_r_p3_p3[4]; double a_i_p3_p3[4];
  double a_r_p3_m1[4]; double a_i_p3_m1[4];
  double a_r_p3_m3[4]; double a_i_p3_m3[4];
  double a_r_m1_p1[4]; double a_i_m1_p1[4];
  double a_r_m1_p3[4]; double a_i_m1_p3[4];
  double a_r_m1_m1[4]; double a_i_m1_m1[4];
  double a_r_m1_m3[4]; double a_i_m1_m3[4];
  double a_r_m3_p1[4]; double a_i_m3_p1[4];
  double a_r_m3_p3[4]; double a_i_m3_p3[4];
  double a_r_m3_m1[4]; double a_i_m3_m1[4];
  double a_r_m3_m3[4]; double a_i_m3_m3[4];

  double ch_mag_des[4];
  double ch_mag_int[4];
  
  double psi_a_p1_p1[4]; double psi_a_p1_p3[4];
  double psi_a_p1_m1[4]; double psi_a_p1_m3[4];
  double psi_a_p3_p1[4]; double psi_a_p3_p3[4];
  double psi_a_p3_m1[4]; double psi_a_p3_m3[4];
  double psi_a_m1_p1[4]; double psi_a_m1_p3[4];
  double psi_a_m1_m1[4]; double psi_a_m1_m3[4];
  double psi_a_m3_p1[4]; double psi_a_m3_p3[4];
  double psi_a_m3_m1[4]; double psi_a_m3_m3[4];

  double a_sq_p1_p1[4]; double a_sq_p1_p3[4];
  double a_sq_p1_m1[4]; double a_sq_p1_m3[4];
  double a_sq_p3_p1[4]; double a_sq_p3_p3[4];
  double a_sq_p3_m1[4]; double a_sq_p3_m3[4];
  double a_sq_m1_p1[4]; double a_sq_m1_p3[4];
  double a_sq_m1_m1[4]; double a_sq_m1_m3[4];
  double a_sq_m3_p1[4]; double a_sq_m3_p3[4];
  double a_sq_m3_m1[4]; double a_sq_m3_m3[4];

  double bit_met_p1_p1[4]; double bit_met_p1_p3[4];
  double bit_met_p1_m1[4]; double bit_met_p1_m3[4];
  double bit_met_p3_p1[4]; double bit_met_p3_p3[4];
  double bit_met_p3_m1[4]; double bit_met_p3_m3[4];
  double bit_met_m1_p1[4]; double bit_met_m1_p3[4];
  double bit_met_m1_m1[4]; double bit_met_m1_m3[4];
  double bit_met_m3_p1[4]; double bit_met_m3_p3[4];
  double bit_met_m3_m1[4]; double bit_met_m3_m3[4];

  double logmax_num_re0 = 0.0;
  double logmax_den_re0 = 0.0;

  double llr_y0r[4];
  double llr_y0i[4];
  double llr_y1r[4];
  double llr_y1i[4];

  for (i=0; i<length>>1; i+=2)
    {// In one iteration, we deal with 4 complex samples or 8 real samples
      rho_re[0] = *(rho01+4*i+0);
      rho_im[0] = *(rho01+4*i+1);
      rho_re[1] = *(rho01+4*i+2);
      rho_im[1] = *(rho01+4*i+3);
      rho_re[2] = *(rho01+4*i+4);
      rho_im[2] = *(rho01+4*i+5);
      rho_re[3] = *(rho01+4*i+6);
      rho_im[3] = *(rho01+4*i+7);
     
      y0_re[0] = *(stream0_in+4*i+0);
      y0_im[0] = *(stream0_in+4*i+1);
      y0_re[1] = *(stream0_in+4*i+2);
      y0_im[1] = *(stream0_in+4*i+3);
      y0_re[2] = *(stream0_in+4*i+4);
      y0_im[2] = *(stream0_in+4*i+5);
      y0_re[3] = *(stream0_in+4*i+6);
      y0_im[3] = *(stream0_in+4*i+7);

      y1_re[0] = *(stream1_in+4*i+0);
      y1_im[0] = *(stream1_in+4*i+1);
      y1_re[1] = *(stream1_in+4*i+2);
      y1_im[1] = *(stream1_in+4*i+3);
      y1_re[2] = *(stream1_in+4*i+4);
      y1_im[2] = *(stream1_in+4*i+5);
      y1_re[3] = *(stream1_in+4*i+6);
      y1_im[3] = *(stream1_in+4*i+7);

      two_h1_square_over_sqrt_10[0] = *(ch_mag+4*i+0);
      two_h1_square_over_sqrt_10[1] = *(ch_mag+4*i+2);
      two_h1_square_over_sqrt_10[2] = *(ch_mag+4*i+4);
      two_h1_square_over_sqrt_10[3] = *(ch_mag+4*i+6);

      two_h2_square_over_sqrt_10[0] = *(ch_mag_i+4*i+0);
      two_h2_square_over_sqrt_10[1] = *(ch_mag_i+4*i+2);
      two_h2_square_over_sqrt_10[2] = *(ch_mag_i+4*i+4);
      two_h2_square_over_sqrt_10[3] = *(ch_mag_i+4*i+6);
      
      // psi_r and psi_i calculation
      for (j=0; j<4; j++)
	{
psi_r_p1_p1[j] = fabs(+1/sqrt(20)*rho_re[j]+1/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_p1_p1[j] = fabs(+1/sqrt(20)*rho_re[j]-1/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_p1_p3[j] = fabs(+1/sqrt(20)*rho_re[j]+3/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_p1_p3[j] = fabs(+3/sqrt(20)*rho_re[j]-1/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_p1_m1[j] = fabs(+1/sqrt(20)*rho_re[j]-1/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_p1_m1[j] = fabs(-1/sqrt(20)*rho_re[j]-1/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_p1_m3[j] = fabs(+1/sqrt(20)*rho_re[j]-3/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_p1_m3[j] = fabs(-3/sqrt(20)*rho_re[j]-1/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_p3_p1[j] = fabs(+3/sqrt(20)*rho_re[j]+1/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_p3_p1[j] = fabs(+1/sqrt(20)*rho_re[j]-3/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_p3_p3[j] = fabs(+3/sqrt(20)*rho_re[j]+3/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_p3_p3[j] = fabs(+3/sqrt(20)*rho_re[j]-3/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_p3_m1[j] = fabs(+3/sqrt(20)*rho_re[j]-1/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_p3_m1[j] = fabs(-1/sqrt(20)*rho_re[j]-3/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_p3_m3[j] = fabs(+3/sqrt(20)*rho_re[j]-3/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_p3_m3[j] = fabs(-3/sqrt(20)*rho_re[j]-3/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_m1_p1[j] = fabs(-1/sqrt(20)*rho_re[j]+1/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_m1_p1[j] = fabs(+1/sqrt(20)*rho_re[j]+1/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_m1_p3[j] = fabs(-1/sqrt(20)*rho_re[j]+3/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_m1_p3[j] = fabs(+3/sqrt(20)*rho_re[j]+1/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_m1_m1[j] = fabs(-1/sqrt(20)*rho_re[j]-1/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_m1_m1[j] = fabs(-1/sqrt(20)*rho_re[j]+1/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_m1_m3[j] = fabs(-1/sqrt(20)*rho_re[j]-3/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_m1_m3[j] = fabs(-3/sqrt(20)*rho_re[j]+1/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_m3_p1[j] = fabs(-3/sqrt(20)*rho_re[j]+1/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_m3_p1[j] = fabs(+1/sqrt(20)*rho_re[j]+3/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_m3_p3[j] = fabs(-3/sqrt(20)*rho_re[j]+3/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_m3_p3[j] = fabs(+3/sqrt(20)*rho_re[j]+3/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_m3_m1[j] = fabs(-3/sqrt(20)*rho_re[j]-1/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_m3_m1[j] = fabs(-1/sqrt(20)*rho_re[j]+3/sqrt(20)*rho_im[j] - y1_im[j]);
psi_r_m3_m3[j] = fabs(-3/sqrt(20)*rho_re[j]-3/sqrt(20)*rho_im[j] - y1_re[j]);psi_i_m3_m3[j] = fabs(-3/sqrt(20)*rho_re[j]+3/sqrt(20)*rho_im[j] - y1_im[j]);
	}
      for (j=0; j<4; j++)
	{
	  ch_mag_des[j] = two_h1_square_over_sqrt_10[j];      
	  ch_mag_int[j] = two_h2_square_over_sqrt_10[j];
	}

      // Detection of interference term
      for (j=0; j<4; j++)
	{
	  a_r_p1_p1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_p1_p1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_p1_p3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_p1_p3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_p1_m1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_p1_m1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_p1_m3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_p1_m3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_p3_p1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_p3_p1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_p3_p3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_p3_p3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_p3_m1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_p3_m1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_p3_m3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_p3_m3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_m1_p1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_m1_p1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_m1_p3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_m1_p3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_m1_m1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_m1_m1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_m1_m3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_m1_m3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_m3_p1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_m3_p1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_m3_p3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_m3_p3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_m3_m1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_m3_m1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_r_m3_m3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_r_m3_m3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  
	  a_i_p1_p1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_p1_p1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_p1_p3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_p1_p3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_p1_m1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_p1_m1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_p1_m3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_p1_m3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_p3_p1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_p3_p1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_p3_p3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_p3_p3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_p3_m1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_p3_m1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_p3_m3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_p3_m3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_m1_p1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_m1_p1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_m1_p3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_m1_p3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_m1_m1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_m1_m1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_m1_m3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_m1_m3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_m3_p1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_m3_p1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_m3_p3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_m3_p3[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_m3_m1[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_m3_m1[j]), 1/sqrt(2)*ch_mag_int[j])));
	  a_i_m3_m3[j] = 1/sqrt(10)*(2+pow(-1, I_flp(fabs(psi_i_m3_m3[j]), 1/sqrt(2)*ch_mag_int[j])));
	}      

      for (j=0; j<4; j++)
	{
	  psi_a_p1_p1[j] = 1/sqrt(2)*(fabs(psi_r_p1_p1[j])*a_r_p1_p1[j] + fabs(psi_i_p1_p1[j])*a_i_p1_p1[j]);
	  psi_a_p1_p3[j] = 1/sqrt(2)*(fabs(psi_r_p1_p3[j])*a_r_p1_p3[j] + fabs(psi_i_p1_p3[j])*a_i_p1_p3[j]);
	  psi_a_p1_m1[j] = 1/sqrt(2)*(fabs(psi_r_p1_m1[j])*a_r_p1_m1[j] + fabs(psi_i_p1_m1[j])*a_i_p1_m1[j]);
	  psi_a_p1_m3[j] = 1/sqrt(2)*(fabs(psi_r_p1_m3[j])*a_r_p1_m3[j] + fabs(psi_i_p1_m3[j])*a_i_p1_m3[j]);
	  psi_a_p3_p1[j] = 1/sqrt(2)*(fabs(psi_r_p3_p1[j])*a_r_p3_p1[j] + fabs(psi_i_p3_p1[j])*a_i_p3_p1[j]);
	  psi_a_p3_p3[j] = 1/sqrt(2)*(fabs(psi_r_p3_p3[j])*a_r_p3_p3[j] + fabs(psi_i_p3_p3[j])*a_i_p3_p3[j]);
	  psi_a_p3_m1[j] = 1/sqrt(2)*(fabs(psi_r_p3_m1[j])*a_r_p3_m1[j] + fabs(psi_i_p3_m1[j])*a_i_p3_m1[j]);
	  psi_a_p3_m3[j] = 1/sqrt(2)*(fabs(psi_r_p3_m3[j])*a_r_p3_m3[j] + fabs(psi_i_p3_m3[j])*a_i_p3_m3[j]);
	  psi_a_m1_p1[j] = 1/sqrt(2)*(fabs(psi_r_m1_p1[j])*a_r_m1_p1[j] + fabs(psi_i_m1_p1[j])*a_i_m1_p1[j]);
	  psi_a_m1_p3[j] = 1/sqrt(2)*(fabs(psi_r_m1_p3[j])*a_r_m1_p3[j] + fabs(psi_i_m1_p3[j])*a_i_m1_p3[j]);
	  psi_a_m1_m1[j] = 1/sqrt(2)*(fabs(psi_r_m1_m1[j])*a_r_m1_m1[j] + fabs(psi_i_m1_m1[j])*a_i_m1_m1[j]);
	  psi_a_m1_m3[j] = 1/sqrt(2)*(fabs(psi_r_m1_m3[j])*a_r_m1_m3[j] + fabs(psi_i_m1_m3[j])*a_i_m1_m3[j]);
	  psi_a_m3_p1[j] = 1/sqrt(2)*(fabs(psi_r_m3_p1[j])*a_r_m3_p1[j] + fabs(psi_i_m3_p1[j])*a_i_m3_p1[j]);
	  psi_a_m3_p3[j] = 1/sqrt(2)*(fabs(psi_r_m3_p3[j])*a_r_m3_p3[j] + fabs(psi_i_m3_p3[j])*a_i_m3_p3[j]);
	  psi_a_m3_m1[j] = 1/sqrt(2)*(fabs(psi_r_m3_m1[j])*a_r_m3_m1[j] + fabs(psi_i_m3_m1[j])*a_i_m3_m1[j]);
	  psi_a_m3_m3[j] = 1/sqrt(2)*(fabs(psi_r_m3_m3[j])*a_r_m3_m3[j] + fabs(psi_i_m3_m3[j])*a_i_m3_m3[j]);
	}
      
      for (j=0; j<4; j++)
	{
	  a_sq_p1_p1[j] = 1/2.0*(a_r_p1_p1[j]*a_r_p1_p1[j] + a_i_p1_p1[j]*a_i_p1_p1[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_p1_p3[j] = 1/2.0*(a_r_p1_p3[j]*a_r_p1_p3[j] + a_i_p1_p3[j]*a_i_p1_p3[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_p1_m1[j] = 1/2.0*(a_r_p1_m1[j]*a_r_p1_m1[j] + a_i_p1_m1[j]*a_i_p1_m1[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_p1_m3[j] = 1/2.0*(a_r_p1_m3[j]*a_r_p1_m3[j] + a_i_p1_m3[j]*a_i_p1_m3[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_p3_p1[j] = 1/2.0*(a_r_p3_p1[j]*a_r_p3_p1[j] + a_i_p3_p1[j]*a_i_p3_p1[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_p3_p3[j] = 1/2.0*(a_r_p3_p3[j]*a_r_p3_p3[j] + a_i_p3_p3[j]*a_i_p3_p3[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_p3_m1[j] = 1/2.0*(a_r_p3_m1[j]*a_r_p3_m1[j] + a_i_p3_m1[j]*a_i_p3_m1[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_p3_m3[j] = 1/2.0*(a_r_p3_m3[j]*a_r_p3_m3[j] + a_i_p3_m3[j]*a_i_p3_m3[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_m1_p1[j] = 1/2.0*(a_r_m1_p1[j]*a_r_m1_p1[j] + a_i_m1_p1[j]*a_i_m1_p1[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_m1_p3[j] = 1/2.0*(a_r_m1_p3[j]*a_r_m1_p3[j] + a_i_m1_p3[j]*a_i_m1_p3[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_m1_m1[j] = 1/2.0*(a_r_m1_m1[j]*a_r_m1_m1[j] + a_i_m1_m1[j]*a_i_m1_m1[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_m1_m3[j] = 1/2.0*(a_r_m1_m3[j]*a_r_m1_m3[j] + a_i_m1_m3[j]*a_i_m1_m3[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_m3_p1[j] = 1/2.0*(a_r_m3_p1[j]*a_r_m3_p1[j] + a_i_m3_p1[j]*a_i_m3_p1[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_m3_p3[j] = 1/2.0*(a_r_m3_p3[j]*a_r_m3_p3[j] + a_i_m3_p3[j]*a_i_m3_p3[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_m3_m1[j] = 1/2.0*(a_r_m3_m1[j]*a_r_m3_m1[j] + a_i_m3_m1[j]*a_i_m3_m1[j])*ch_mag_int[j]*sqrt(10)/2;
	  a_sq_m3_m3[j] = 1/2.0*(a_r_m3_m3[j]*a_r_m3_m3[j] + a_i_m3_m3[j]*a_i_m3_m3[j])*ch_mag_int[j]*sqrt(10)/2;
	}
      
      // Computing Metrics
      for (j=0; j<4; j++)
	{
bit_met_p1_p1[j] = -1/10.0*ch_mag_des[j]*sqrt(10)/2 + 2/sqrt(10)*y0_re[j]*1/sqrt(2) + 2/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_p1_p1[j] - a_sq_p1_p1[j];
bit_met_p1_p3[j] =  -1/2.0*ch_mag_des[j]*sqrt(10)/2 + 2/sqrt(10)*y0_re[j]*1/sqrt(2) + 6/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_p1_p3[j] - a_sq_p1_p3[j];
bit_met_p1_m1[j] = -1/10.0*ch_mag_des[j]*sqrt(10)/2 + 2/sqrt(10)*y0_re[j]*1/sqrt(2) - 2/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_p1_m1[j] - a_sq_p1_m1[j];
bit_met_p1_m3[j] =  -1/2.0*ch_mag_des[j]*sqrt(10)/2 + 2/sqrt(10)*y0_re[j]*1/sqrt(2) - 6/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_p1_m3[j] - a_sq_p1_m3[j];
bit_met_p3_p1[j] =  -1/2.0*ch_mag_des[j]*sqrt(10)/2 + 6/sqrt(10)*y0_re[j]*1/sqrt(2) + 2/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_p3_p1[j] - a_sq_p3_p1[j];
bit_met_p3_p3[j] = -9/10.0*ch_mag_des[j]*sqrt(10)/2 + 6/sqrt(10)*y0_re[j]*1/sqrt(2) + 6/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_p3_p3[j] - a_sq_p3_p3[j];
bit_met_p3_m1[j] =  -1/2.0*ch_mag_des[j]*sqrt(10)/2 + 6/sqrt(10)*y0_re[j]*1/sqrt(2) - 2/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_p3_m1[j] - a_sq_p3_m1[j];
bit_met_p3_m3[j] = -9/10.0*ch_mag_des[j]*sqrt(10)/2 + 6/sqrt(10)*y0_re[j]*1/sqrt(2) - 6/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_p3_m3[j] - a_sq_p3_m3[j];
bit_met_m1_p1[j] = -1/10.0*ch_mag_des[j]*sqrt(10)/2 - 2/sqrt(10)*y0_re[j]*1/sqrt(2) + 2/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_m1_p1[j] - a_sq_m1_p1[j];
bit_met_m1_p3[j] =  -1/2.0*ch_mag_des[j]*sqrt(10)/2 - 2/sqrt(10)*y0_re[j]*1/sqrt(2) + 6/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_m1_p3[j] - a_sq_m1_p3[j];
bit_met_m1_m1[j] = -1/10.0*ch_mag_des[j]*sqrt(10)/2 - 2/sqrt(10)*y0_re[j]*1/sqrt(2) - 2/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_m1_m1[j] - a_sq_m1_m1[j];
bit_met_m1_m3[j] =  -1/2.0*ch_mag_des[j]*sqrt(10)/2 - 2/sqrt(10)*y0_re[j]*1/sqrt(2) - 6/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_m1_m3[j] - a_sq_m1_m3[j];
bit_met_m3_p1[j] =  -1/2.0*ch_mag_des[j]*sqrt(10)/2 - 6/sqrt(10)*y0_re[j]*1/sqrt(2) + 2/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_m3_p1[j] - a_sq_m3_p1[j];
bit_met_m3_p3[j] = -9/10.0*ch_mag_des[j]*sqrt(10)/2 - 6/sqrt(10)*y0_re[j]*1/sqrt(2) + 6/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_m3_p3[j] - a_sq_m3_p3[j];
bit_met_m3_m1[j] =  -1/2.0*ch_mag_des[j]*sqrt(10)/2 - 6/sqrt(10)*y0_re[j]*1/sqrt(2) - 2/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_m3_m1[j] - a_sq_m3_m1[j];
bit_met_m3_m3[j] = -9/10.0*ch_mag_des[j]*sqrt(10)/2 - 6/sqrt(10)*y0_re[j]*1/sqrt(2) - 6/sqrt(10)*y0_im[j]*1/sqrt(2) + 2*psi_a_m3_m3[j] - a_sq_m3_m3[j];
	}
      
      for (j=0; j<4; j++)
	{      
	  // Detection for y0r i.e. 1st bit 
	  logmax_num_re0 = max(max(max(max(max(max(max(bit_met_m1_p1[j], bit_met_m1_p3[j]), bit_met_m1_m1[j]), bit_met_m1_m3[j]), bit_met_m3_p1[j]), bit_met_m3_p3[j]), bit_met_m3_m1[j]), bit_met_m3_m3[j]);
	  logmax_den_re0 = max(max(max(max(max(max(max(bit_met_p1_p1[j], bit_met_p1_p3[j]), bit_met_p1_m1[j]), bit_met_p1_m3[j]), bit_met_p3_p1[j]), bit_met_p3_p3[j]), bit_met_p3_m1[j]), bit_met_p3_m3[j]);
	  llr_y0r[j] = logmax_num_re0 - logmax_den_re0;
      
	  // Detection for y0i i.e. second bit
	  logmax_num_re0 = max(max(max(max(max(max(max(bit_met_p1_m1[j], bit_met_p3_m1[j]), bit_met_m1_m1[j]), bit_met_m3_m1[j]), bit_met_p1_m3[j]), bit_met_p3_m3[j]), bit_met_m1_m3[j]), bit_met_m3_m3[j]);
	  logmax_den_re0 = max(max(max(max(max(max(max(bit_met_p1_p1[j], bit_met_p3_p1[j]), bit_met_m1_p1[j]), bit_met_m3_p1[j]), bit_met_p1_p3[j]), bit_met_p3_p3[j]), bit_met_m1_p3[j]), bit_met_m3_p3[j]);
	  llr_y0i[j] = logmax_num_re0 - logmax_den_re0;
	  
	  // Detection for y1r i.e.  third bit
	  logmax_num_re0 = max(max(max(max(max(max(max(bit_met_m3_p1[j], bit_met_m3_p3[j]), bit_met_m3_m1[j]), bit_met_m3_m3[j]), bit_met_p3_p1[j]), bit_met_p3_p3[j]), bit_met_p3_m1[j]), bit_met_p3_m3[j]);
	  logmax_den_re0 = max(max(max(max(max(max(max(bit_met_m1_p1[j], bit_met_m1_p3[j]), bit_met_m1_m1[j]), bit_met_m1_m3[j]), bit_met_p1_p1[j]), bit_met_p1_p3[j]), bit_met_p1_m1[j]), bit_met_p1_m3[j]);
	  llr_y1r[j] = logmax_num_re0 - logmax_den_re0;
	  
	  // Detection for y1i i.e.  fourth bit
	  logmax_num_re0 = max(max(max(max(max(max(max(bit_met_p1_m3[j], bit_met_p3_m3[j]), bit_met_m1_m3[j]), bit_met_m3_m3[j]), bit_met_p1_p3[j]), bit_met_p3_p3[j]), bit_met_m1_p3[j]), bit_met_m3_p3[j]);
	  logmax_den_re0 = max(max(max(max(max(max(max(bit_met_p1_m1[j], bit_met_p3_m1[j]), bit_met_m1_m1[j]), bit_met_m3_m1[j]), bit_met_p1_p1[j]), bit_met_p3_p1[j]), bit_met_m1_p1[j]), bit_met_m3_p1[j]);
	  llr_y1i[j] = logmax_num_re0 - logmax_den_re0;
	}
      
      // Stream out
      *(stream0_out+0)  = -(short)(floor(llr_y0r[0]*32768.0));
      *(stream0_out+1)  = -(short)(floor(llr_y0i[0]*32768.0)); 
      *(stream0_out+2)  = -(short)(floor(llr_y1r[0]*32768.0));
      *(stream0_out+3)  = -(short)(floor(llr_y1i[0]*32768.0));
      
      *(stream0_out+4)  = -(short)(floor(llr_y0r[1]*32768.0));
      *(stream0_out+5)  = -(short)(floor(llr_y0i[1]*32768.0)); 
      *(stream0_out+6)  = -(short)(floor(llr_y1r[1]*32768.0));
      *(stream0_out+7)  = -(short)(floor(llr_y1i[1]*32768.0));
      
      *(stream0_out+8)  = -(short)(floor(llr_y0r[2]*32768.0));
      *(stream0_out+9)  = -(short)(floor(llr_y0i[2]*32768.0)); 
      *(stream0_out+10) = -(short)(floor(llr_y1r[2]*32768.0));
      *(stream0_out+11) = -(short)(floor(llr_y1i[2]*32768.0));
      
      *(stream0_out+12) = -(short)(floor(llr_y0r[3]*32768.0));
      *(stream0_out+13) = -(short)(floor(llr_y0i[3]*32768.0)); 
      *(stream0_out+14) = -(short)(floor(llr_y1r[3]*32768.0));
      *(stream0_out+15) = -(short)(floor(llr_y1i[3]*32768.0));
      
      stream0_out += 16;
    }
  }
  else
  */
  { // TBD w/ LCM MAP detector (Naive MLD here)
      float *rho01_fct      = (float *)rho01; // float type format is 4 bytes whereas double has an 8-bytes format
      float *stream0_in_fct = (float *)stream0_in;
      float *stream1_in_fct = (float *)stream1_in;
      //short *stream0_out_fct= (short *)stream0_out;
      float *ch_mag_fct     = (float *)ch_mag;
      float *ch_mag_i_fct   = (float *)ch_mag_i;
      
      /*
      msg("\nstream0[i] (h1'*y)=[%f,%f,%f,%f]\n", stream0_in_fct[0], stream0_in_fct[1], stream0_in_fct[2], stream0_in_fct[3]);
      msg("stream1[i] (h2'*y)=[%f,%f,%f,%f]\n", stream1_in_fct[0], stream1_in_fct[1], stream1_in_fct[2], stream1_in_fct[3]);
      msg("ch_mag[i] (2*||h1||^2/sqrt(10))=  [%f,%f,%f,%f]\n", ch_mag_fct[0], ch_mag_fct[1], ch_mag_fct[2], ch_mag_fct[3]);
      msg("ch_mag_i[i] (2*||h2||^2/sqrt(10))=[%f,%f,%f,%f]\n", ch_mag_i_fct[0], ch_mag_i_fct[1], ch_mag_i_fct[2], ch_mag_i_fct[3]);
      msg("rho[i] (h1'*h2)=[%f,%f,%f,%f]\n\n", rho01_fct[0], rho01_fct[1], rho01_fct[2], rho01_fct[3]);
      */
      
      int i, j, iLLR, k, l;
      /*
      msg("dl_ch_mag in detector forall sequence=[");
      for (i=0; i<2101; i++)
	msg("%f(%d),", ch_mag_fct[i], i);
      msg("]\n");
      */
      double rho_re, rho_im;
      /*double two_h1_square_over_sqrt_10, two_h2_square_over_sqrt_10;*/
      double /*ch_mag_des,*/ ch_mag_int;
      double h1_square, h2_square;
      
      double term_y1r, term_y1i, term_y2r, term_y2i;
      double term_2_y1r_x1r, term_2_y1i_x1i;  
      double psi_r, psi_i;
      double term_absx2r, term_absx2i;
      double term_2_psir_x2r, term_2_psii_x2i;
      
      double x1r, x1i, x2r, x2i;
      double sigma1 = 1.0/sqrt(2);
      double sigma2 = 1.0/sqrt(2);
      double d = 0.0;
      double dmax_bIs0 = 0.0;
      double dmax_bIs1 = 0.0;
      float llr[4];
      
      char mask = 0;
      double scaledConstellation[32] = { 1.0/sqrt(10),  1.0/sqrt(10),  1.0/sqrt(10),  3.0/sqrt(10),  3.0/sqrt(10),  1.0/sqrt(10),  3.0/sqrt(10),  3.0/sqrt(10),
					 1.0/sqrt(10), -1.0/sqrt(10),  1.0/sqrt(10), -3.0/sqrt(10),  3.0/sqrt(10), -1.0/sqrt(10),  3.0/sqrt(10), -3.0/sqrt(10),
					 -1.0/sqrt(10),  1.0/sqrt(10), -1.0/sqrt(10),  3.0/sqrt(10), -3.0/sqrt(10),  1.0/sqrt(10), -3.0/sqrt(10),  3.0/sqrt(10),
					 -1.0/sqrt(10), -1.0/sqrt(10), -1.0/sqrt(10), -3.0/sqrt(10), -3.0/sqrt(10), -1.0/sqrt(10), -3.0/sqrt(10), -3.0/sqrt(10)};
      
      for (i=0; i<length; i++)
	{// In one iteration, we deal with 1 complex samples (2 real ones)
	  int symbolMapping[256];
	  for (j=0; j<256; j++)
	    symbolMapping[j] = j;
	  
	  // From OpenAirInterfaceDebug.m (Sebastien Aubert, 2011.11.16)
	  term_y1r = *(stream0_in_fct+2*i);
	  term_y1i = *(stream0_in_fct+2*i+1);
	  
	  term_y2r = *(stream1_in_fct+2*i);
	  term_y2i = *(stream1_in_fct+2*i+1);
	  
	  rho_re = *(rho01_fct+2*i);  
	  rho_im = *(rho01_fct+2*i+1);
	  
	  h1_square = *(ch_mag_fct+2*i)/2.0*sqrt(10);
	  h2_square = *(ch_mag_i_fct+2*i)/2.0*sqrt(10);
	  
	  ch_mag_int = *(ch_mag_i_fct+2*i);
	  
	  mask = 8;	  
	  for (iLLR=0; iLLR<4; iLLR++)
	    {
	      dmax_bIs0 = -1.0E10; // -Inf
	      dmax_bIs1 = -1.0E10; // -Inf
	      
	      for (k=0; k<16; k++)
		{
		  x1r = sigma1*scaledConstellation[2*k];
		  x1i = sigma1*scaledConstellation[2*k+1];
		  
		  term_2_y1r_x1r = 2*term_y1r*x1r;
		  term_2_y1i_x1i = 2*term_y1i*x1i;
	      
		  psi_r = rho_re*x1r + rho_im*x1i - term_y2r;
		  psi_i = rho_re*x1i - rho_im*x1r - term_y2i;
		  
		  for (l=0; l<16; l++)
		    {
		      x2r = sigma2*scaledConstellation[2*l];
		      x2i = sigma2*scaledConstellation[2*l+1];
		      
		      term_absx2r = x2r;
		      term_absx2i = x2i;
		      
		      term_2_psir_x2r = 2*fabs(psi_r)*term_absx2r;
		      term_2_psii_x2i = 2*fabs(psi_i)*term_absx2i;
		      
		      d = - h1_square*x1r*x1r
			  - h1_square*x1i*x1i
			  + term_2_y1r_x1r
			  + term_2_y1i_x1i
			  + term_2_psir_x2r
			  - h2_square*x2r*x2r
			  + term_2_psii_x2i
			  - h2_square*x2i*x2i;
		      
		      if ( (symbolMapping[k] & mask)==0 ) // Current bit is 0
			{
			  if (d > dmax_bIs0)
			    dmax_bIs0 = d;
			}
		      else // Current bit is 1
			{
			  if (d > dmax_bIs1)
			    dmax_bIs1 = d;
			}
		    }
		}
	      llr[iLLR] = dmax_bIs1 - dmax_bIs0;
	      mask = mask>>1;
	    } // End iLLR
	  
	  /*
	  msg("llr[i]  =[%f,%f,%f,%f]\n", llr[0], llr[1], llr[2], llr[3]);
	  */
	  
	  // Stream out
	  *(stream0_out+0)  = -(short)(floor(llr[0]*pow(2, 22)));
	  *(stream0_out+1)  = -(short)(floor(llr[1]*pow(2, 22))); 
	  *(stream0_out+2)  = -(short)(floor(llr[2]*pow(2, 22)));
	  *(stream0_out+3)  = -(short)(floor(llr[3]*pow(2, 22)));
	  
	  msg("llr within detector=[%d,%d,%d,%d]\n", stream0_out[0], stream0_out[1], stream0_out[2], stream0_out[3]);
	  
	  stream0_out += 4;
	}
      /*
      stream0_out = stream0_out - (length>>1*4);
      int p = 0;
      msg("llr in detector forall sequence=[");
      for (p=0; p<2100; p++)
	msg("%d(%d),", stream0_out[p], p);
      msg("]\n");
      */
    }
}

void qam16_qam16_mu_mimo_only_flp(double *stream0_in,
				  double *stream1_in,
				  double *ch_mag,
				  double *ch_mag_i,
				  float *stream0_out,
				  double *rho01,
				  int length)
{  
  // TBD w/ LCM MAP detector (Naive MLD here)
  float *rho01_fct      = (float *)rho01; // float type format is 4 bytes whereas double has an 8-bytes format
  float *stream0_in_fct = (float *)stream0_in;
  float *stream1_in_fct = (float *)stream1_in;
  //float *stream0_out_fct= (float *)stream0_out;
  float *ch_mag_fct     = (float *)ch_mag;
  float *ch_mag_i_fct   = (float *)ch_mag_i;
  
  int i, j, iLLR, k, l;
  double rho_re, rho_im;
  /*double two_h1_square_over_sqrt_10, two_h2_square_over_sqrt_10;*/
  double /*ch_mag_des,*/ ch_mag_int;
  double h1_square, h2_square;
  
  double term_y1r, term_y1i, term_y2r, term_y2i;
  double term_2_y1r_x1r, term_2_y1i_x1i;  
  double psi_r, psi_i;
  double term_absx2r, term_absx2i;
  double term_2_psir_x2r, term_2_psii_x2i;
      
  double x1r, x1i, x2r, x2i;
  double sigma1 = 1.0/sqrt(2);
  double sigma2 = 1.0/sqrt(2);
  double d = 0.0;
  double dmax_bIs0 = 0.0;
  double dmax_bIs1 = 0.0;
  float llr[4];
  
  char mask = 0;
  double scaledConstellation[32] = { 1.0/sqrt(10),  1.0/sqrt(10),  1.0/sqrt(10),  3.0/sqrt(10),  3.0/sqrt(10),  1.0/sqrt(10),  3.0/sqrt(10),  3.0/sqrt(10),
				     1.0/sqrt(10), -1.0/sqrt(10),  1.0/sqrt(10), -3.0/sqrt(10),  3.0/sqrt(10), -1.0/sqrt(10),  3.0/sqrt(10), -3.0/sqrt(10),
				    -1.0/sqrt(10),  1.0/sqrt(10), -1.0/sqrt(10),  3.0/sqrt(10), -3.0/sqrt(10),  1.0/sqrt(10), -3.0/sqrt(10),  3.0/sqrt(10),
				    -1.0/sqrt(10), -1.0/sqrt(10), -1.0/sqrt(10), -3.0/sqrt(10), -3.0/sqrt(10), -1.0/sqrt(10), -3.0/sqrt(10), -3.0/sqrt(10)};
  
  for (i=0; i<length; i++)
    {// In one iteration, we deal with 1 complex samples (2 real ones)
      int symbolMapping[256];
      for (j=0; j<256; j++)
	symbolMapping[j] = j;
      
      // From OpenAirInterfaceDebug.m (Sebastien Aubert, 2011.11.16)
      term_y1r = *(stream0_in_fct+2*i);
      term_y1i = *(stream0_in_fct+2*i+1);
      
      term_y2r = *(stream1_in_fct+2*i);
      term_y2i = *(stream1_in_fct+2*i+1);
      
      rho_re = *(rho01_fct+2*i);  
      rho_im = *(rho01_fct+2*i+1);
      
      h1_square = *(ch_mag_fct+2*i)/2.0*sqrt(10);
      h2_square = *(ch_mag_i_fct+2*i)/2.0*sqrt(10);
      
      ch_mag_int = *(ch_mag_i_fct+2*i);
      
      mask = 8;	  
      for (iLLR=0; iLLR<4; iLLR++)
	{
	  dmax_bIs0 = -1.0E10; // -Inf
	  dmax_bIs1 = -1.0E10; // -Inf
	  
	  for (k=0; k<16; k++)
	    {
	      x1r = sigma1*scaledConstellation[2*k];
	      x1i = sigma1*scaledConstellation[2*k+1];
	      
	      term_2_y1r_x1r = 2*term_y1r*x1r;
	      term_2_y1i_x1i = 2*term_y1i*x1i;
	      
	      psi_r = rho_re*x1r + rho_im*x1i - term_y2r;
	      psi_i = rho_re*x1i - rho_im*x1r - term_y2i;
	      
	      for (l=0; l<16; l++)
		{
		  x2r = sigma2*scaledConstellation[2*l];
		  x2i = sigma2*scaledConstellation[2*l+1];
		  
		  term_absx2r = x2r;
		  term_absx2i = x2i;
		      
		  term_2_psir_x2r = 2*fabs(psi_r)*term_absx2r;
		  term_2_psii_x2i = 2*fabs(psi_i)*term_absx2i;
		  
		  d = - h1_square*x1r*x1r
		    - h1_square*x1i*x1i
		    + term_2_y1r_x1r
		    + term_2_y1i_x1i
		    + term_2_psir_x2r
		    - h2_square*x2r*x2r
		    + term_2_psii_x2i
		    - h2_square*x2i*x2i;
		  
		  if ( (symbolMapping[k] & mask)==0 ) // Current bit is 0
		    {
		      if (d > dmax_bIs0)
			dmax_bIs0 = d;
		    }
		  else // Current bit is 1
		    {
		      if (d > dmax_bIs1)
			dmax_bIs1 = d;
		    }
		}
	    }
	  llr[iLLR] = dmax_bIs1 - dmax_bIs0;
	  mask = mask>>1;
	} // End iLLR
      
      // Stream out
      *(stream0_out+0)  = -llr[0];
      *(stream0_out+1)  = -llr[1]; 
      *(stream0_out+2)  = -llr[2];
      *(stream0_out+3)  = -llr[3];
      
      // msg("llr16 within detector=[%f,%f,%f,%f]\n", *((float *)stream0_out+0), *((float *)stream0_out+1), *((float *)stream0_out+2), *((float *)stream0_out+3));

      stream0_out += 4;
    }
  msg("llr within detector=[%f,%f,%f,%f]\n", *(stream0_out-4*length+0), *(stream0_out-4*length+1), *(stream0_out-4*length+2), *(stream0_out-4*length+3));
}

int dlsch_16qam_16qam_llr_full_flp(LTE_DL_FRAME_PARMS *frame_parms,
				   double **rxdataF_comp,
				   double **rxdataF_comp_i,
				   double **dl_ch_mag,   //|h_1|^2*(2/sqrt{10})
				   double **dl_ch_mag_i, //|h_2|^2*(2/sqrt{10})
				   double **rho_i,
				   short *dlsch_llr,
				   unsigned char symbol,
				   unsigned char first_symbol_flag,  //first symbol has different structure due to more pilots
				   unsigned short nb_rb,
				   u16 pbch_pss_sss_adjust,
				   short **llr16p)
{ 
  double *rxF     = (double*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  double *rxF_i   = (double*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
  double *ch_mag  = (double*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
  double *ch_mag_i= (double*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
  double *rho     = (double*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];

  int len;
  u8 symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;

  if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp)))
    {
      // if symbol has pilots
      if (frame_parms->mode1_flag==0)
	// in 2 antenna ports we have 8 REs per symbol per RB
	len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
      else
	// for 1 antenna port we have 10 REs per symbol per RB 
	len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
  else
    {
      // symbol has no pilots
      len = (nb_rb*12) - pbch_pss_sss_adjust;
    }

  s16 *llr16;
  
  if (first_symbol_flag == 1)
    llr16 = (s16*)dlsch_llr;
  else
    llr16 = (s16*)(*llr16p);
  
  if (!llr16)
    {
      msg("dlsch_16qam_16qam_llr_full_flp: llr is null, symbol %d\n",symbol);
      return -1;
    }

  qam16_qam16_mu_mimo_full_flp((double *)rxF,
			       (double *)rxF_i,
			       (double *)ch_mag,
			       (double *)ch_mag_i,
			       (short *)llr16,
			       (double *)rho,
			       len);

  // msg("llr outside detector=[%d,%d,%d,%d]\n", *(llr16+0), *(llr16+1), *(llr16+2), *(llr16+3));  

  llr16 += (len<<2);
  *llr16p = (short *)llr16;

  return(0);
}

// LLR output set to a Flp format
int dlsch_16qam_16qam_llr_only_flp(LTE_DL_FRAME_PARMS *frame_parms,
				   double **rxdataF_comp,
				   double **rxdataF_comp_i,
				   double **dl_ch_mag,   //|h_1|^2*(2/sqrt{10})
				   double **dl_ch_mag_i, //|h_2|^2*(2/sqrt{10})
				   double **rho_i,
				   float *dlsch_llr_flp,
				   unsigned char symbol,
				   unsigned char first_symbol_flag,  //first symbol has different structure due to more pilots
				   unsigned short nb_rb,
				   u16 pbch_pss_sss_adjust,
				   float **llr16p_flp)
{ 
  double *rxF     = (double*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  double *rxF_i   = (double*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
  double *ch_mag  = (double*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
  double *ch_mag_i= (double*)&dl_ch_mag_i[0][(symbol*frame_parms->N_RB_DL*12)];
  double *rho     = (double*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
  
  int len;
  u8 symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;

  if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp)))
    {
      // if symbol has pilots
      if (frame_parms->mode1_flag==0)
	// in 2 antenna ports we have 8 REs per symbol per RB
	len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
      else
	// for 1 antenna port we have 10 REs per symbol per RB 
	len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
    }
  else
    {
      // symbol has no pilots
      len = (nb_rb*12) - pbch_pss_sss_adjust;
    }
  
  float *llr16_flp;
  
  if (first_symbol_flag == 1)
    llr16_flp = (float*)dlsch_llr_flp;
  else
    llr16_flp = (float*)(*llr16p_flp);
  
  if (!llr16_flp)
    {
      msg("dlsch_16qam_16qam_llr_only_flp: llr is null, symbol %d\n", symbol);
      return -1;
    }
    
  qam16_qam16_mu_mimo_only_flp((double *)rxF,
			       (double *)rxF_i,
			       (double *)ch_mag,
			       (double *)ch_mag_i,
			       (float *)llr16_flp,
			       (double *)rho,
			       len);

  // msg("llr outside detector=[%f,%f,%f,%f]\n", *(llr16_flp+0), *(llr16_flp+1), *(llr16_flp+2), *(llr16_flp+3));
  
  llr16_flp += (len<<2); // "<<2" since 4 LLRs per RE
  *llr16p_flp = (float *)llr16_flp;

  return(0);
}

void dlsch_detection_mrc_full_flp(LTE_DL_FRAME_PARMS *frame_parms,
				  double **rxdataF_comp,
				  double **rxdataF_comp_i,
				  double **rho,
				  double **rho_i,
				  double **dl_ch_mag,
				  double **dl_ch_magb,
				  unsigned char symbol,
				  unsigned short nb_rb,
				  unsigned char dual_stream_UE)
{
  /*
  if (0)
    {
  unsigned char aatx;
  int i, j;
  
  //__m128i *rxdataF_comp128_0,*rxdataF_comp128_1,*rxdataF_comp128_i0,*rxdataF_comp128_i1,*dl_ch_mag128_0,*dl_ch_mag128_1,*dl_ch_mag128_0b,*dl_ch_mag128_1b,*rho128_0,*rho128_1,*rho128_i0,*rho128_i1;
  
  double *p_rxdata_comp0, *p_rxdata_comp1, *p_rxdata_comp_i0, *p_rxdata_comp_i1, *p_rho0, *p_rho1, *p_rho_i0, *p_rho_i1, *p_dl_ch_mag0, *p_dl_ch_mag1, *p_dl_ch_magb0, *p_dl_ch_magb1;

  if (frame_parms->nb_antennas_rx>1)
    {
      for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
	{
	  //rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
	  //rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  
	  //dl_ch_mag128_0      = (__m128i *)&dl_ch_mag[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
	  //dl_ch_mag128_1      = (__m128i *)&dl_ch_mag[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  
	  //dl_ch_mag128_0b     = (__m128i *)&dl_ch_magb[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
	  //dl_ch_mag128_1b     = (__m128i *)&dl_ch_magb[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  
	  
	  p_rxdata_comp0 = (double *)&rxdataF_comp[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];
	  p_rxdata_comp1 = (double *)&rxdataF_comp[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];
	  p_dl_ch_mag0   = (double *)&dl_ch_mag[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];
	  p_dl_ch_mag1   = (double *)&dl_ch_mag[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];
	  p_dl_ch_magb0  = (double *)&dl_ch_magb[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];
	  p_dl_ch_magb1  = (double *)&dl_ch_magb[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];

	  // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
	  for (i=0;i<nb_rb*3;i++)
	    {
	      // rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
	      // dl_ch_mag128_0[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0[i],1),_mm_srai_epi16(dl_ch_mag128_1[i],1));
	      // dl_ch_mag128_0b[i]   = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0b[i],1),_mm_srai_epi16(dl_ch_mag128_1b[i],1));
	    
	      for (j=0; j<8; j++)
		{
		  *(p_rxdata_comp0+j) = *(p_rxdata_comp0+j) + *(p_rxdata_comp1+j);
		  *(p_dl_ch_mag0+j)   = *(p_dl_ch_mag0+j)   + *(p_dl_ch_mag1+j);
		  *(p_dl_ch_magb0+j)  = *(p_dl_ch_magb0+j)  + *(p_dl_ch_magb1+j);
		}
	      p_rxdata_comp0 = p_rxdata_comp0 +8;
	      p_rxdata_comp1 = p_rxdata_comp1 +8;
	      p_dl_ch_mag0   = p_dl_ch_mag0   +8;
	      p_dl_ch_mag1   = p_dl_ch_mag1   +8;
	      p_dl_ch_magb0  = p_dl_ch_magb0  +8;
	      p_dl_ch_magb1  = p_dl_ch_magb1  +8;
	    }
	}
  
      if (rho)
	{
	  // rho128_0 = (__m128i *) &rho[0][symbol*frame_parms->N_RB_DL*12];
	  // rho128_1 = (__m128i *) &rho[1][symbol*frame_parms->N_RB_DL*12];
	  
	  p_rho0 = (double *) &rho[0][symbol*frame_parms->N_RB_DL*12];
	  p_rho1 = (double *) &rho[1][symbol*frame_parms->N_RB_DL*12];
	  for (i=0;i<nb_rb*3;i++) 
	    {
	      // rho128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rho128_0[i],1),_mm_srai_epi16(rho128_1[i],1));
	    
	      for (j=0; j<8; j++)
		{
		  *(p_rho0+j) = *(p_rho0+j) + *(p_rho1+j);
		}
	      p_rho0 = p_rho0 +8;
	      p_rho1 = p_rho1 +8;
	    }
	}
      
      if (dual_stream_UE == 1)
	{
	  // rho128_i0 = (__m128i *) &rho_i[0][symbol*frame_parms->N_RB_DL*12];
	  // rho128_i1 = (__m128i *) &rho_i[1][symbol*frame_parms->N_RB_DL*12];
	  // rxdataF_comp128_i0   = (__m128i *)&rxdataF_comp_i[0][symbol*frame_parms->N_RB_DL*12];  
	  // rxdataF_comp128_i1   = (__m128i *)&rxdataF_comp_i[1][symbol*frame_parms->N_RB_DL*12];  
	  
	  p_rho_i0           = (double *) &rho_i[0][symbol*frame_parms->N_RB_DL*12];
	  p_rho_i1           = (double *) &rho_i[1][symbol*frame_parms->N_RB_DL*12];
	  p_rxdata_comp_i0   = (double *)&rxdataF_comp_i[0][symbol*frame_parms->N_RB_DL*12];  
	  p_rxdata_comp_i1   = (double *)&rxdataF_comp_i[1][symbol*frame_parms->N_RB_DL*12];  
	  
	  for (i=0;i<nb_rb*3;i++)
	    {
	      // rxdataF_comp128_i0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_i0[i],1),_mm_srai_epi16(rxdataF_comp128_i1[i],1));
	      // rho128_i0[i]          = _mm_adds_epi16(_mm_srai_epi16(rho128_i0[i],1),_mm_srai_epi16(rho128_i1[i],1));
	      
	      for (j=0; j<8; j++)
		{
		  *(p_rxdata_comp_i0+j) = *(p_rxdata_comp_i0+j) + *(p_rxdata_comp_i1+j);
		  *(p_rho_i0+j) = *(p_rho_i0+j) + *(p_rho_i1+j);
		}
	      p_rxdata_comp_i0 = p_rxdata_comp_i0 +8;
	      p_rxdata_comp_i1 = p_rxdata_comp_i1 +8;
	      p_rho_i0         = p_rho_i0 +8;	      
	      p_rho_i1         = p_rho_i1 +8;	      
	    }
	}
    }
    }
  else
  */
    {      
      unsigned char aatx;
      int i, rb;
      
      float *p_rxdata_comp0, *p_rxdata_comp1, *p_rxdata_comp_i0, *p_rxdata_comp_i1, *p_rho0, *p_rho1, *p_rho_i0, *p_rho_i1, *p_dl_ch_mag0, *p_dl_ch_mag1, *p_dl_ch_magb0, *p_dl_ch_magb1;
      
      // No distinction is made with presence of pilots or not, to consider!
      if (frame_parms->nb_antennas_rx>1)
	{
	  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
	    {
	      p_rxdata_comp0 = (float *)&rxdataF_comp[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];
	      p_rxdata_comp1 = (float *)&rxdataF_comp[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];
	      p_dl_ch_mag0   = (float *)&dl_ch_mag[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];
	      p_dl_ch_mag1   = (float *)&dl_ch_mag[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];
	      p_dl_ch_magb0  = (float *)&dl_ch_magb[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];
	      p_dl_ch_magb1  = (float *)&dl_ch_magb[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];
	      
	      // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
	      for (rb=0; rb<nb_rb; rb++)
		{
		  for (i=0; i<12; i++)
		    {
		      *(p_rxdata_comp0+2*i)   = *(p_rxdata_comp0+2*i)   + *(p_rxdata_comp1+2*i);
		      *(p_rxdata_comp0+2*i+1) = *(p_rxdata_comp0+2*i+1) + *(p_rxdata_comp1+2*i+1);
		      
		      *(p_dl_ch_mag0+2*i)   = *(p_dl_ch_mag0+2*i)   + *(p_dl_ch_mag1+2*i);
		      *(p_dl_ch_mag0+2*i+1) = *(p_dl_ch_mag0+2*i+1) + *(p_dl_ch_mag1+2*i+1);
		      
		      *(p_dl_ch_magb0+2*i)   = *(p_dl_ch_magb0+2*i)   + *(p_dl_ch_magb1+2*i);
		      *(p_dl_ch_magb0+2*i+1) = *(p_dl_ch_magb0+2*i+1) + *(p_dl_ch_magb1+2*i+1);
		    }
		  p_rxdata_comp0 = p_rxdata_comp0 +24;
		  p_rxdata_comp1 = p_rxdata_comp1 +24;
		  p_dl_ch_mag0   = p_dl_ch_mag0   +24;
		  p_dl_ch_mag1   = p_dl_ch_mag1   +24;
		  p_dl_ch_magb0  = p_dl_ch_magb0  +24;
		  p_dl_ch_magb1  = p_dl_ch_magb1  +24;
		}
	    }
	  
	  if (rho)
	    {
	      p_rho0 = (float *) &rho[0][symbol*frame_parms->N_RB_DL*12];
	      p_rho1 = (float *) &rho[1][symbol*frame_parms->N_RB_DL*12];
	      for (rb=0; rb<nb_rb; rb++) 
		{
		  for (i=0; i<12; i++)
		    {
		      *(p_rho0+2*i)   = *(p_rho0+2*i)   + *(p_rho1+2*i);
		      *(p_rho0+2*i+1) = *(p_rho0+2*i+1) + *(p_rho1+2*i+1);		      
		    }
		  p_rho0 = p_rho0 +24;
		  p_rho1 = p_rho1 +24;
		}
	    }
	  
	  if (dual_stream_UE == 1)
	    {
	      p_rho_i0           = (float *) &rho_i[0][symbol*frame_parms->N_RB_DL*12];
	      p_rho_i1           = (float *) &rho_i[1][symbol*frame_parms->N_RB_DL*12];
	      p_rxdata_comp_i0   = (float *)&rxdataF_comp_i[0][symbol*frame_parms->N_RB_DL*12];  
	      p_rxdata_comp_i1   = (float *)&rxdataF_comp_i[1][symbol*frame_parms->N_RB_DL*12];  
	      
	      for (rb=0; rb<nb_rb; rb++)
		{
		  for (i=0; i<12; i++)
		    {
		      *(p_rxdata_comp_i0+2*i)   = *(p_rxdata_comp_i0+2*i)   + *(p_rxdata_comp_i1+2*i);
		      *(p_rxdata_comp_i0+2*i+1) = *(p_rxdata_comp_i0+2*i+1) + *(p_rxdata_comp_i1+2*i+1);

		      *(p_rho_i0+2*i)   = *(p_rho_i0+2*i)   + *(p_rho_i1+2*i);
		      *(p_rho_i0+2*i+1) = *(p_rho_i0+2*i+1) + *(p_rho_i1+2*i+1);
		    }
		  p_rxdata_comp_i0 = p_rxdata_comp_i0 +24;
		  p_rxdata_comp_i1 = p_rxdata_comp_i1 +24;
		  p_rho_i0         = p_rho_i0 +24;	      
		  p_rho_i1         = p_rho_i1 +24;	      
		}
	    }
	}      
    }
}

void dlsch_dual_stream_correlation_full_flp(LTE_DL_FRAME_PARMS *frame_parms,
					    unsigned char symbol,
					    unsigned short nb_rb,
					    int **dl_ch_estimates_ext,
					    int **dl_ch_estimates_ext_i,
					    double **dl_ch_rho_ext,
					    unsigned char output_shift)
{
  /*
  if (0) // NOK, new version
  {
  unsigned short rb;
  unsigned char aarx,symbol_mod,pilots=0;
  int i;
  
  short *p_dl_ch, *p_dl_chi; // input
  double *p_dl_ch_rho; // output
  double dl_ch_temp[24], dl_chi_temp[24], dl_ch_rho_temp[24];
  double temp1_16bits[8]; // Correspond to 8*16 bits
  double temp0_32bits[4], temp1_32bits[4], temp2_32bits[4], temp3_32bits[4]; // Correspond to 4*32 bits
  
  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  
  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
    pilots=1;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
    {
      p_dl_ch      = (short *)&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12];
      p_dl_chi     = (short *)&dl_ch_estimates_ext_i[aarx][symbol_mod*frame_parms->N_RB_DL*12];
      
      p_dl_ch_rho  = (double *)&dl_ch_rho_ext[aarx][symbol*frame_parms->N_RB_DL*12];
      // pointers are first casted to short for 16-bits format reading/writing
      
      for (rb=0;rb<nb_rb;rb++)
	{
	  for (i=0; i<16; i++)
	    {
	      dl_ch_temp[i]   = *(p_dl_ch+i)/pow(2, 15);
	      dl_chi_temp[i]  = *(p_dl_chi+i)/pow(2, 15);
	    }
	  if (pilots==0)
	    {
	      for (i=16; i<24; i++)
		{
		  dl_ch_temp[i]  = *(p_dl_ch+i)/pow(2, 15);
		  dl_chi_temp[i] = *(p_dl_chi+i)/pow(2, 15);
		}
	    }

	  // multiply by conjugated channel
	  for (i=0; i<8>>1; i++)
	    temp0_32bits[i] = dl_ch_temp[2*i]*dl_chi_temp[2*i] + dl_ch_temp[2*i+1]*dl_chi_temp[2*i+1];
	  
	  temp1_16bits[0] = -dl_ch_temp[1]; // shuffle low
	  temp1_16bits[1] =  dl_ch_temp[0]; // + conjugate
	  temp1_16bits[2] = -dl_ch_temp[3];
	  temp1_16bits[3] =  dl_ch_temp[2];
	  temp1_16bits[4] = -dl_ch_temp[4+1]; // shuffle high
	  temp1_16bits[5] =  dl_ch_temp[4+0];
	  temp1_16bits[6] = -dl_ch_temp[4+3];
	  temp1_16bits[7] =  dl_ch_temp[4+2];
	  
	  for (i=0; i<8>>1; i++)
	    temp1_32bits[i] = temp1_16bits[2*i]*dl_chi_temp[2*i] + temp1_16bits[2*i+1]*dl_chi_temp[2*i+1];
	  
	  temp2_32bits[0] = temp0_32bits[0]; // unpacklo_epi32
	  temp2_32bits[1] = temp1_32bits[0];
	  temp2_32bits[2] = temp0_32bits[1];
	  temp2_32bits[3] = temp1_32bits[1];
	  
	  temp3_32bits[0] = temp0_32bits[2]; // unpacklo_epi32
	  temp3_32bits[1] = temp1_32bits[2];
	  temp3_32bits[2] = temp0_32bits[3];
	  temp3_32bits[3] = temp1_32bits[3];
	  
	  for (i=0; i<8>>1; i++)
	    {
	      dl_ch_rho_temp[i]   = temp2_32bits[i];
	      dl_ch_rho_temp[4+i] = temp3_32bits[i];
	    }	      
	  
	  // multiply by conjugated channel
	  for (i=0; i<8>>1; i++)
	    temp0_32bits[i] = dl_ch_temp[8+2*i]*dl_chi_temp[8+2*i] + dl_ch_temp[8+2*i+1]*dl_chi_temp[8+2*i+1];
	  
	  temp1_16bits[0] = -dl_ch_temp[8+1]; // shuffle low
	  temp1_16bits[1] =  dl_ch_temp[8+0]; // + conjugate
	  temp1_16bits[2] = -dl_ch_temp[8+3];
	  temp1_16bits[3] =  dl_ch_temp[8+2];
	  temp1_16bits[4] = -dl_ch_temp[8+4+1]; // shuffle high
	  temp1_16bits[5] =  dl_ch_temp[8+4+0];
	  temp1_16bits[6] = -dl_ch_temp[8+4+3];
	  temp1_16bits[7] =  dl_ch_temp[8+4+2];
	  
	  for (i=0; i<8>>1; i++)
	    temp1_32bits[i] = temp1_16bits[2*i]*dl_chi_temp[8+2*i] + temp1_16bits[2*i+1]*dl_chi_temp[8+2*i+1];
	  
	  temp2_32bits[0] = temp0_32bits[0]; // unpacklo_epi32
	  temp2_32bits[1] = temp1_32bits[0];
	  temp2_32bits[2] = temp0_32bits[1];
	  temp2_32bits[3] = temp1_32bits[1];
	  
	  temp3_32bits[0] = temp0_32bits[2]; // unpacklo_epi32
	  temp3_32bits[1] = temp1_32bits[2];
	  temp3_32bits[2] = temp0_32bits[3];
	  temp3_32bits[3] = temp1_32bits[3];
	  
	  for (i=0; i<8>>1; i++)
	    {
	      dl_ch_rho_temp[8+i]   = temp2_32bits[i];
	      dl_ch_rho_temp[8+4+i] = temp3_32bits[i];
	    }	      
	  if (pilots==0)
	    {  
	      // multiply by conjugated channel
	      for (i=0; i<8>>1; i++)
		temp0_32bits[i] = dl_ch_temp[16+2*i]*dl_chi_temp[16+2*i] + dl_ch_temp[16+2*i+1]*dl_chi_temp[16+2*i+1];
	      
	      temp1_16bits[0] = -dl_ch_temp[16+1]; // shuffle low
	      temp1_16bits[1] =  dl_ch_temp[16+0]; // + conjugate
	      temp1_16bits[2] = -dl_ch_temp[16+3];
	      temp1_16bits[3] =  dl_ch_temp[16+2];
	      temp1_16bits[4] = -dl_ch_temp[16+4+1]; // shuffle high
	      temp1_16bits[5] =  dl_ch_temp[16+4+0];
	      temp1_16bits[6] = -dl_ch_temp[16+4+3];
	      temp1_16bits[7] =  dl_ch_temp[16+4+2];
	      
	      for (i=0; i<8>>1; i++)
		temp1_32bits[i] = temp1_16bits[2*i]*dl_chi_temp[16+2*i] + temp1_16bits[2*i+1]*dl_chi_temp[16+2*i+1];
	      
	      temp2_32bits[0] = temp0_32bits[0]; // unpacklo_epi32
	      temp2_32bits[1] = temp1_32bits[0];
	      temp2_32bits[2] = temp0_32bits[1];
	      temp2_32bits[3] = temp1_32bits[1];
	      
	      temp3_32bits[0] = temp0_32bits[2]; // unpacklo_epi32
	      temp3_32bits[1] = temp1_32bits[2];
	      temp3_32bits[2] = temp0_32bits[3];
	      temp3_32bits[3] = temp1_32bits[3];
	      
	      for (i=0; i<8>>1; i++)
		{
		  dl_ch_rho_temp[16+i]   = temp2_32bits[i];
		  dl_ch_rho_temp[16+4+i] = temp3_32bits[i];
		}	      		  
	      
	      for (i=0; i<24; i++)
		{
		  // Should be casted to int
		  *(p_dl_ch+i)     = (short)floor(dl_ch_temp[i]*pow(2, output_shift));
		  *(p_dl_chi+i)    = (short)floor(dl_chi_temp[i]*pow(2, output_shift));
		  *(p_dl_ch_rho+i) = (double) dl_ch_rho_temp[i];
		}
	      p_dl_ch     = p_dl_ch+24;
	      p_dl_chi    = p_dl_chi+24;
	      p_dl_ch_rho = p_dl_ch_rho+24;
	    }
	  else
	    {      
	      for (i=0; i<16; i++)
		{
		  *(p_dl_ch+i)     = (short)floor(dl_ch_temp[i]*pow(2, output_shift));
		  *(p_dl_chi+i)    = (short)floor(dl_chi_temp[i]*pow(2, output_shift));
		  *(p_dl_ch_rho+i) = (double) dl_ch_rho_temp[i];
		}
	      p_dl_ch     = p_dl_ch+16;
	      p_dl_chi    = p_dl_chi+16;
	      p_dl_ch_rho = p_dl_ch_rho+16;
	    }
	}	
    }
  }
  else
   */
    {
      unsigned short rb;
      unsigned char aarx,symbol_mod,pilots=0;
      int i;
      
      short *p_dl_ch/*input*/, *p_dl_chi/*input*/;
      float *p_dl_ch_rho/*output*/;
  
      symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
      
      if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
	pilots=1;
      
      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
	{
	  int index=0;
	  
	  p_dl_ch      = (short *)&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12]; // short is a 2 bytes-format element
	  p_dl_chi     = (short *)&dl_ch_estimates_ext_i[aarx][symbol_mod*frame_parms->N_RB_DL*12];
	  
	  p_dl_ch_rho  = (float *)&dl_ch_rho_ext[aarx][symbol*frame_parms->N_RB_DL*12]; // float is a 4 bytes-format element
	  
	  for (rb=0;rb<nb_rb;rb++)
	    {
	      // 12 REs (in freq) per function call when no pilots (data only), 8 REs (in freq) otherwise (pilots previously removed)
	      if (pilots==0) 
		{  
		  for (i=0; i<12; i++)
		    {
		      *(p_dl_ch_rho+2*i) = (float) ((*(p_dl_ch+2*i)/pow(2, 15))*(*(p_dl_chi+2*i)/pow(2, 15)) + (*(p_dl_ch+2*i+1)/pow(2, 15))*(*(p_dl_chi+2*i+1)/pow(2, 15))); // Re{rho} = Re{h1}Re{h2} + Im{h1}Im{h2}
		      *(p_dl_ch_rho+2*i+1) = (float) ((*(p_dl_ch+2*i)/pow(2, 15))*(*(p_dl_chi+2*i+1)/pow(2, 15)) - (*(p_dl_ch+2*i+1)/pow(2, 15))*(*(p_dl_chi+2*i)/pow(2, 15))); // Im{rho} = Re{h1}Im{h2} - Im{h1}Re{h2}
		      
		    }
		  p_dl_ch     = p_dl_ch+24;
		  p_dl_chi    = p_dl_chi+24;
		  p_dl_ch_rho = p_dl_ch_rho+24;
		  
		  index += 24;
		}	      
	      else
		{
		  for (i=0; i<8; i++)
		    {
		      *(p_dl_ch_rho+2*i)   = (float) ((*(p_dl_ch+2*i)/pow(2, 15))*(*(p_dl_chi+2*i)/pow(2, 15)) + (*(p_dl_ch+2*i+1)/pow(2, 15))*(*(p_dl_chi+2*i+1)/pow(2, 15)));
		      *(p_dl_ch_rho+2*i+1) = (float) ((*(p_dl_ch+2*i)/pow(2, 15))*(*(p_dl_chi+2*i+1)/pow(2, 15)) - (*(p_dl_ch+2*i+1)/pow(2, 15))*(*(p_dl_chi+2*i)/pow(2, 15)));
		    }
		  p_dl_ch     = p_dl_ch+16;
		  p_dl_chi    = p_dl_chi+16;
		  p_dl_ch_rho = p_dl_ch_rho+!6;
		  
		  index += 16;
		}
	      //msg("symbol=%d, symbol_mod=%d, rb=%d, pilots=%d, index=%d\n", symbol, symbol_mod, rb, pilots, index);
	    }	
	}
    }
}
 
void dlsch_channel_compensation_prec_full_flp(int **rxdataF_ext,
					      int **dl_ch_estimates_ext,
					      double **dl_ch_mag, // Used in the sequel with an Flp format
					      double **dl_ch_magb,
					      double **rxdataF_comp,
					      unsigned char *pmi_ext,
					      LTE_DL_FRAME_PARMS *frame_parms,
					      PHY_MEASUREMENTS *phy_measurements,
					      int eNB_id,
					      unsigned char symbol,
					      u8 first_symbol_flag,
					      unsigned char mod_order,
					      unsigned short nb_rb,
					      unsigned char output_shift,
					      unsigned char dl_power_off)
{  
  /*
  if (0) // NOK, new version
  {
  unsigned short rb,Nre;
  unsigned char aarx=0,symbol_mod,pilots=0;
  int precoded_signal_strength=0,rx_power_correction;
  int i;
  double QAM_amp, QAM_ampb;
  short *p_dl_ch_0, *p_dl_ch_1, *p_rxdataF; // input
  double *p_dl_ch_magb, *p_dl_ch_mag, *p_rxdataF_comp; // output

  double dl_ch_0_temp[24], dl_ch_1_temp[24], dl_ch_mag_temp[24], dl_ch_magb_temp[24], rxdataF_temp[24], rxdataF_comp_temp[24];
  double temp0_16bits[8], temp1_16bits[8]; // Correspond to 8*16 bits
  double temp0_32bits[4], temp1_32bits[4], temp2_32bits[4], temp3_32bits[4]; // Correspond to 4*32 bits

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  // msg("symbol_mod=%d\n", symbol_mod);

  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
    pilots=1;

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;

  if (dl_power_off==1) { // IA receiver
    if (mod_order == 4)
      {
	QAM_amp = 2.0/sqrt(10);
	QAM_ampb = 0.0; // Un-initialized (while used) for 16-QAM
      }
    else if (mod_order == 6)
      {
	QAM_amp  = 4.0/sqrt(42);
	QAM_ampb = 2.0/sqrt(42);
      }
  }
  else { // Non-IA receiver
    if (mod_order == 4)
      {
	QAM_amp = 2.0/sqrt(10)/sqrt(2);
	QAM_ampb = 0.0;
      }
    else if (mod_order == 6)
      {
	QAM_amp  = 4.0/sqrt(42)/sqrt(2);
	QAM_ampb = 2.0/sqrt(42)/sqrt(2);
      }
  }
  
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
    {
      p_dl_ch_0 = (short *)&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12];
      p_dl_ch_1 = (short *)&dl_ch_estimates_ext[2+aarx][symbol_mod*frame_parms->N_RB_DL*12];
      
      p_dl_ch_mag  = (double *)&dl_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
      p_dl_ch_magb = (double *)&dl_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
      
      p_rxdataF      = (short *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
      p_rxdataF_comp = (double *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];
      // pointers are first casted to short for 16-bits format reading/writing
      
      for (rb=0;rb<nb_rb;rb++)
	{
	  for (i=0; i<16; i++)
	    {
	      dl_ch_0_temp[i] = *(p_dl_ch_0+i)/pow(2, 15);
	      dl_ch_1_temp[i] = *(p_dl_ch_1+i)/pow(2, 15);
	      rxdataF_temp[i] = *(p_rxdataF+i)/pow(2, 15);
	    }
	  if (pilots==0)
	    {
	      for (i=16; i<24; i++)
		{
		  dl_ch_0_temp[i] = *(p_dl_ch_0+i)/pow(2, 15);
		  dl_ch_1_temp[i] = *(p_dl_ch_1+i)/pow(2, 15);
		  rxdataF_temp[i] = *(p_rxdataF+i)/pow(2, 15);
		}
	    }
	  
	  // combine TX channels using precoder from pmi
	  prec2A_flp(pmi_ext[rb], &dl_ch_0_temp[0], &dl_ch_1_temp[0]);
	  prec2A_flp(pmi_ext[rb], &dl_ch_0_temp[8], &dl_ch_1_temp[8]);
	  if (pilots==0)
	    prec2A_flp(pmi_ext[rb], &dl_ch_0_temp[16], &dl_ch_1_temp[16]);
	  
	  if (mod_order>2)
	    {
	      // get channel amplitude if not QPSK
	      for (i=0; i<8>>1; i++)
		{
		  temp0_32bits[i] = dl_ch_0_temp[2*i]*dl_ch_0_temp[2*i]     + dl_ch_0_temp[2*i+1]*dl_ch_0_temp[2*i+1];
		  temp1_32bits[i] = dl_ch_0_temp[8+2*i]*dl_ch_0_temp[8+2*i] + dl_ch_0_temp[8+2*i+1]*dl_ch_0_temp[8+2*i+1];
		}
	      
	      for (i=0; i<8>>1; i++)
		{
		  temp0_16bits[i]   = temp0_32bits[i];
		  temp0_16bits[4+i] = temp1_32bits[i];
		}
	      for (i=0; i<8>>1; i++)
		{
		  dl_ch_mag_temp[2*i]     = temp0_16bits[i]; // unpacklo
		  dl_ch_mag_temp[2*i+1]   = temp0_16bits[i];
		  dl_ch_mag_temp[8+2*i]   = temp0_16bits[4+i]; // unpachhi
		  dl_ch_mag_temp[8+2*i+1] = temp0_16bits[4+i];
		  
		  dl_ch_magb_temp[2*i]     = dl_ch_mag_temp[2*i];
		  dl_ch_magb_temp[2*i+1]   = dl_ch_mag_temp[2*i+1];
		  dl_ch_magb_temp[8+2*i]   = dl_ch_mag_temp[8+2*i];
		  dl_ch_magb_temp[8+2*i+1] = dl_ch_mag_temp[8+2*i+1];
		  
		  dl_ch_mag_temp[2*i]     = dl_ch_mag_temp[2*i]*QAM_amp;
		  dl_ch_mag_temp[2*i+1]   = dl_ch_mag_temp[2*i+1]*QAM_amp;
		  dl_ch_mag_temp[8+2*i]   = dl_ch_mag_temp[8+2*i]*QAM_amp;
		  dl_ch_mag_temp[8+2*i+1] = dl_ch_mag_temp[8+2*i+1]*QAM_amp;
		}
	      
	      if (pilots==0)
		{
		  for (i=0; i<8>>1; i++)
		    temp0_32bits[i] = dl_ch_0_temp[16+2*i]*dl_ch_0_temp[16+2*i] + dl_ch_0_temp[16+2*i+1]*dl_ch_0_temp[16+2*i+1];
		  
		  for (i=0; i<8>>1; i++)
		    {
		      temp1_16bits[i]   = temp0_32bits[i];
		      temp1_16bits[4+i] = temp0_32bits[i];
		    }
		  for (i=0; i<8>>1; i++)
		    {
		      dl_ch_mag_temp[16+2*i]   = temp1_16bits[i]; // unpacklo
		      dl_ch_mag_temp[16+2*i+1] = temp1_16bits[i];
		      
		      dl_ch_magb_temp[16+2*i]   = dl_ch_mag_temp[16+2*i];
		      dl_ch_magb_temp[16+2*i+1] = dl_ch_mag_temp[16+2*i+1];
		      
		      dl_ch_mag_temp[16+2*i]   = dl_ch_mag_temp[16+2*i]*QAM_amp;
		      dl_ch_mag_temp[16+2*i+1] = dl_ch_mag_temp[16+2*i+1]*QAM_amp;
		    }
		}
	      
	      for (i=0; i<8>>1; i++)
		{
		  dl_ch_magb_temp[2*i]   = dl_ch_magb_temp[2*i]*QAM_ampb;
		  dl_ch_magb_temp[2*i+1] = dl_ch_magb_temp[2*i+1]*QAM_ampb;
		  
		  dl_ch_magb_temp[8+2*i]   = dl_ch_magb_temp[8+2*i]*QAM_ampb;
		  dl_ch_magb_temp[8+2*i+1] = dl_ch_magb_temp[8+2*i+1]*QAM_ampb;
		}
	      
	      if (pilots==0)
		{
		  for (i=0; i<8>>1; i++)
		    {
		      dl_ch_magb_temp[16+2*i]   = dl_ch_magb_temp[16+2*i]*QAM_ampb;
		      dl_ch_magb_temp[16+2*i+1] = dl_ch_magb_temp[16+2*i+1]*QAM_ampb;
		    }
		}
	    }
	  
	  for (i=0; i<8>>1; i++)
	    temp0_32bits[i] = dl_ch_0_temp[2*i]*rxdataF_temp[2*i] + dl_ch_0_temp[2*i+1]*rxdataF_temp[2*i+1];
	  
	  temp1_16bits[0] = -dl_ch_0_temp[1]; // shuffle low
	  temp1_16bits[1] =  dl_ch_0_temp[0]; // + conjugate
	  temp1_16bits[2] = -dl_ch_0_temp[3];
	  temp1_16bits[3] =  dl_ch_0_temp[2];
	  temp1_16bits[4] = -dl_ch_0_temp[4+1]; // shuffle high
	  temp1_16bits[5] =  dl_ch_0_temp[4+0];
	  temp1_16bits[6] = -dl_ch_0_temp[4+3];
	  temp1_16bits[7] =  dl_ch_0_temp[4+2];
	  
	  for (i=0; i<8>>1; i++)
	    temp1_32bits[i] = temp1_16bits[2*i]*rxdataF_temp[2*i] + temp1_16bits[2*i+1]*rxdataF_temp[2*i+1];
	  
	  temp2_32bits[0] = temp0_32bits[0]; // unpacklo_epi32
	  temp2_32bits[1] = temp1_32bits[0];
	  temp2_32bits[2] = temp0_32bits[1];
	  temp2_32bits[3] = temp1_32bits[1];
	  
	  temp3_32bits[0] = temp0_32bits[2]; // unpacklo_epi32
	  temp3_32bits[1] = temp1_32bits[2];
	  temp3_32bits[2] = temp0_32bits[3];
	  temp3_32bits[3] = temp1_32bits[3];
	  
	  for (i=0; i<8>>1; i++)
	    {
	      rxdataF_comp_temp[i]   = temp2_32bits[i];
	      rxdataF_comp_temp[4+i] = temp3_32bits[i];
	    }
	  
	  for (i=0; i<8>>1; i++)
	    temp0_32bits[i] = dl_ch_0_temp[8+2*i]*rxdataF_temp[8+2*i] + dl_ch_0_temp[8+2*i+1]*rxdataF_temp[8+2*i+1];
	  
	  temp1_16bits[0] = -dl_ch_0_temp[8+1]; // shuffle low
	  temp1_16bits[1] =  dl_ch_0_temp[8+0]; // + conjugate
	  temp1_16bits[2] = -dl_ch_0_temp[8+3];
	  temp1_16bits[3] =  dl_ch_0_temp[8+2];
	  temp1_16bits[4] = -dl_ch_0_temp[8+4+1]; // shuffle high
	  temp1_16bits[5] =  dl_ch_0_temp[8+4+0];
	  temp1_16bits[6] = -dl_ch_0_temp[8+4+3];
	  temp1_16bits[7] =  dl_ch_0_temp[8+4+2];
	  
	  for (i=0; i<8>>1; i++)
	    temp1_32bits[i] = temp1_16bits[2*i]*rxdataF_temp[8+2*i] + temp1_16bits[2*i+1]*rxdataF_temp[8+2*i+1];
	  
	  temp2_32bits[0] = temp0_32bits[0]; // unpacklo_epi32
	  temp2_32bits[1] = temp1_32bits[0];
	  temp2_32bits[2] = temp0_32bits[1];
	  temp2_32bits[3] = temp1_32bits[1];
	  
	  temp3_32bits[0] = temp0_32bits[2]; // unpacklo_epi32
	  temp3_32bits[1] = temp1_32bits[2];
	  temp3_32bits[2] = temp0_32bits[3];
	  temp3_32bits[3] = temp1_32bits[3];
	  
	  for (i=0; i<8>>1; i++)
	    {
	      rxdataF_comp_temp[8+i]   = temp2_32bits[i];
	      rxdataF_comp_temp[8+4+i] = temp3_32bits[i];
	    }
	  
	  if (pilots==0)
	    {
	      for (i=0; i<8>>1; i++)
		temp0_32bits[i] = dl_ch_0_temp[16+2*i]*rxdataF_temp[16+2*i] + dl_ch_0_temp[16+2*i+1]*rxdataF_temp[16+2*i+1];
	      
	      temp1_16bits[0] = -dl_ch_0_temp[16+1]; // shuffle low
	      temp1_16bits[1] =  dl_ch_0_temp[16+0]; // + conjugate
	      temp1_16bits[2] = -dl_ch_0_temp[16+3];
	      temp1_16bits[3] =  dl_ch_0_temp[16+2];
	      temp1_16bits[4] = -dl_ch_0_temp[16+4+1]; // shuffle high
	      temp1_16bits[5] =  dl_ch_0_temp[16+4+0];
	      temp1_16bits[6] = -dl_ch_0_temp[16+4+3];
	      temp1_16bits[7] =  dl_ch_0_temp[16+4+2];
	      
	      for (i=0; i<8>>1; i++)
		temp1_32bits[i] = temp1_16bits[2*i]*rxdataF_temp[16+2*i] + temp1_16bits[2*i+1]*rxdataF_temp[16+2*i+1];
	      
	      temp2_32bits[0] = temp0_32bits[0]; // unpacklo_epi32
	      temp2_32bits[1] = temp1_32bits[0];
	      temp2_32bits[2] = temp0_32bits[1];
	      temp2_32bits[3] = temp1_32bits[1];
	      
	      temp3_32bits[0] = temp0_32bits[2]; // unpacklo_epi32
	      temp3_32bits[1] = temp1_32bits[2];
	      temp3_32bits[2] = temp0_32bits[3];
	      temp3_32bits[3] = temp1_32bits[3];
	      
	      for (i=0; i<8>>1; i++)
		{
		  rxdataF_comp_temp[16+i]   = temp2_32bits[i];
		  rxdataF_comp_temp[16+4+i] = temp3_32bits[i];
		}
	      
	      for (i=0; i<24; i++)
		{
		  // Should be casted to int
		  *(p_dl_ch_0+i)      = (short)floor(dl_ch_0_temp[i]*pow(2, output_shift));
		  *(p_dl_ch_1+i)      = (short)floor(dl_ch_1_temp[i]*pow(2, output_shift));
		  *(p_dl_ch_mag+i)    = (double) dl_ch_mag_temp[i];
		  *(p_dl_ch_magb+i )  = (double) dl_ch_magb_temp[i];
		  *(p_rxdataF+i)      = (short)floor(rxdataF_temp[i]*pow(2, output_shift));
		  *(p_rxdataF_comp+i) = (double) rxdataF_comp_temp[i];
		}
	      
	      p_dl_ch_0      = p_dl_ch_0+24;
	      p_dl_ch_1      = p_dl_ch_1+24;
	      p_dl_ch_mag    = p_dl_ch_mag+24;
	      p_dl_ch_magb   = p_dl_ch_magb+24;
	      p_rxdataF      = p_rxdataF+24;
	      p_rxdataF_comp = p_rxdataF_comp+24;
	    }
	  else
	    {
	      for (i=0; i<16; i++)
		{
		  *(p_dl_ch_0+i)      = (short)floor(dl_ch_0_temp[i]*pow(2, output_shift));
		  *(p_dl_ch_1+i)      = (short)floor(dl_ch_1_temp[i]*pow(2, output_shift));
		  *(p_dl_ch_mag+i)    = (double) dl_ch_mag_temp[i];
		  *(p_dl_ch_magb+i )  = (double) dl_ch_magb_temp[i];
		  *(p_rxdataF+i)      = (short)floor(rxdataF_temp[i]*pow(2, output_shift));
		  *(p_rxdataF_comp+i) = (double) rxdataF_comp_temp[i];
		}
	      
	      p_dl_ch_0      = p_dl_ch_0+16;
	      p_dl_ch_1      = p_dl_ch_1+16;
	      p_dl_ch_mag    = p_dl_ch_mag+16;
	      p_dl_ch_magb   = p_dl_ch_magb+16;
	      p_rxdataF      = p_rxdataF+16;
	      p_rxdataF_comp = p_rxdataF_comp+16;
	    } 
	}
      Nre = (pilots==0) ? 12 : 8;
      
      precoded_signal_strength += ((signal_energy_nodc(&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*Nre],
						       (nb_rb*Nre))*rx_power_correction) - (phy_measurements->n0_power[aarx]));
      //msg("precoded_signal_strength=%d\n", precoded_signal_strength);
    } // rx_antennas
  
  phy_measurements->precoded_cqi_dB[eNB_id][0] = dB_fixed2(precoded_signal_strength,phy_measurements->n0_power_tot);
  }
  else
    */
    {
      unsigned short rb,Nre;
      unsigned char aarx=0,symbol_mod,pilots=0;
      int precoded_signal_strength=0,rx_power_correction;
      int i;
      double QAM_amp, QAM_ampb;
      short *p_dl_ch_0/*input*/, *p_dl_ch_1/*input*/, *p_rxdataF/*input*/;
      float *p_dl_ch_magb/*output*/, *p_dl_ch_mag/*output*/, *p_rxdataF_comp/*output*/;
            
      symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
      
      if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
	pilots=1;
      
      if ( (frame_parms->ofdm_symbol_size == 128) ||
	   (frame_parms->ofdm_symbol_size == 512) )
	rx_power_correction = 2;
      else
	rx_power_correction = 1;
      
      if (dl_power_off==1) { /* IA receiver */
	if (mod_order == 4)
	  {
	    QAM_amp = 2.0/sqrt(10);
	    QAM_ampb = 0.0; // Un-initialized (while used) for 16-QAM
	  }
	else if (mod_order == 6)
	  {
	    QAM_amp  = 4.0/sqrt(42);
	    QAM_ampb = 2.0/sqrt(42);
	  }
      }
      else { /* Non-IA receiver */
	if (mod_order == 4)
	  {
	    QAM_amp = 2.0/sqrt(10)/sqrt(2);
	    QAM_ampb = 0.0;
	  }
	else if (mod_order == 6)
	  {
	    QAM_amp  = 4.0/sqrt(42)/sqrt(2);
	    QAM_ampb = 2.0/sqrt(42)/sqrt(2);
	  }
      }
      
      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
	{
	  p_dl_ch_0 = (short *)&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12];
	  p_dl_ch_1 = (short *)&dl_ch_estimates_ext[2+aarx][symbol_mod*frame_parms->N_RB_DL*12];
	  
	  p_dl_ch_mag  = (float *)&dl_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
	  p_dl_ch_magb = (float *)&dl_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
	  
	  p_rxdataF      = (short *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
	  p_rxdataF_comp = (float *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];
	  // pointers are first casted to short for 16-bits format reading/writing

	  for (rb=0;rb<nb_rb;rb++)
	    {		  
	      // 12 REs (in freq) per function call when no pilots (data only), 8 REs (in freq) otherwise (pilots previously removed)
	      if (pilots==0) 
		{ 
		  // Combine TX channels using precoder from pmi
		  switch (pmi_ext[rb])
		    {
		    case 0 : // +1 +1
		      for (i=0; i<12; i++)
			{
			  *(p_dl_ch_0+2*i)   = *(p_dl_ch_0+2*i)+*(p_dl_ch_1+2*i);
			  *(p_dl_ch_0+2*i+1) = *(p_dl_ch_0+2*i+1)+*(p_dl_ch_1+2*i+1); // Add Re and Im independently
			}
		      break;
		    case 1 : // +1 -1
		      for (i=0; i<12; i++)
			{
			  *(p_dl_ch_0+2*i)   = *(p_dl_ch_0+2*i)-*(p_dl_ch_1+2*i);
			  *(p_dl_ch_0+2*i+1) = *(p_dl_ch_0+2*i+1)-*(p_dl_ch_1+2*i+1); // Sub Re and Im independently
			}
		      break;
		    case 2 : // +1 +j
		      for (i=0; i<12; i++)
			{
			  *(p_dl_ch_0+2*i)   = *(p_dl_ch_0+2*i)-*(p_dl_ch_1+2*i+1);
			  *(p_dl_ch_0+2*i+1) = *(p_dl_ch_0+2*i+1)+*(p_dl_ch_1+2*i);
			}
		      break;
		    case 3 : // +1 -j
		      for (i=0; i<12; i++)
			{
			  *(p_dl_ch_0+2*i)   = *(p_dl_ch_0+2*i)+*(p_dl_ch_1+2*i+1);
			  *(p_dl_ch_0+2*i+1) = *(p_dl_ch_0+2*i+1)-*(p_dl_ch_1+2*i);
			}
		      break;
		    }
						
		  for (i=0; i<12; i++)
		    {
		      if (mod_order>2)
			{
			  // Get channel amplitude if not QPSK
			  *(p_dl_ch_mag+2*i)   = (float) ((*(p_dl_ch_0+2*i)/pow(2, 15))*(*(p_dl_ch_0+2*i)/pow(2, 15)) + (*(p_dl_ch_0+2*i+1)/pow(2, 15))*(*(p_dl_ch_0+2*i+1)/pow(2, 15)))*QAM_amp; // Re{dl_ch_mag} = ||h1||^2*2/sqrt(10)
			  *(p_dl_ch_mag+2*i+1) = *(p_dl_ch_mag+2*i) ;// Im{dl_ch_mag} = Re{dl_ch_mag}
			  
			  *(p_dl_ch_magb+2*i)   = (float) ((*(p_dl_ch_1+2*i)/pow(2, 15))*(*(p_dl_ch_1+2*i)/pow(2, 15)) + (*(p_dl_ch_1+2*i+1)/pow(2, 15))*(*(p_dl_ch_1+2*i+1)/pow(2, 15)))*QAM_ampb; // Re{dl_ch_magb} = ||h2||^2*2/sqrt(10)
			  *(p_dl_ch_magb+2*i+1) = *(p_dl_ch_magb+2*i) ;// Im{dl_ch_magb} = Re{dl_ch_magb}
			}
		      
		      *(p_rxdataF_comp+2*i)   = (float) ((*(p_dl_ch_0+2*i)/pow(2, 15))*(*(p_rxdataF+2*i)/pow(2, 15)) + (*(p_dl_ch_0+2*i+1)/pow(2, 15))*(*(p_rxdataF+2*i+1)/pow(2, 15))); // Re{rxdataF_comp} = Re{h1}Re{y} + Im{h1}Im{y}
		      *(p_rxdataF_comp+2*i+1) = (float) ((*(p_dl_ch_0+2*i)/pow(2, 15))*(*(p_rxdataF+2*i+1)/pow(2, 15)) - (*(p_dl_ch_0+2*i+1)/pow(2, 15))*(*(p_rxdataF+2*i)/pow(2, 15))); // Re{rxdataF_comp} = Re{h1}Im{y} - Im{h1}Re{y}
		    }
		  p_dl_ch_0      = p_dl_ch_0+24;
		  p_dl_ch_1      = p_dl_ch_1+24;
		  
		  p_dl_ch_mag    = p_dl_ch_mag+24;
		  p_dl_ch_magb   = p_dl_ch_magb+24;
		  
		  p_rxdataF      = p_rxdataF+24;
		  p_rxdataF_comp = p_rxdataF_comp+24;
		}	      
	      else
		{
		  // Combine TX channels using precoder from pmi
		  switch (pmi_ext[rb])
		    {
		    case 0 : // +1 +1
		      for (i=0; i<8; i++)
			{
			  *(p_dl_ch_0+2*i)   = *(p_dl_ch_0+2*i)+*(p_dl_ch_1+2*i);
			  *(p_dl_ch_0+2*i+1) = *(p_dl_ch_0+2*i+1)+*(p_dl_ch_1+2*i+1); // Add Re and Im independently
			}
		      break;
		    case 1 : // +1 -1
		      for (i=0; i<8; i++)
			{
			  *(p_dl_ch_0+2*i)   = *(p_dl_ch_0+2*i)-*(p_dl_ch_1+2*i);
			  *(p_dl_ch_0+2*i+1) = *(p_dl_ch_0+2*i+1)-*(p_dl_ch_1+2*i+1); // Sub Re and Im independently
			}
		      break;
		    case 2 : // +1 +j
		      for (i=0; i<8; i++)
			{
			  *(p_dl_ch_0+2*i)   = *(p_dl_ch_0+2*i)-*(p_dl_ch_1+2*i+1);
			  *(p_dl_ch_0+2*i+1) = *(p_dl_ch_0+2*i+1)+*(p_dl_ch_1+2*i);
			}
		      break;
		    case 3 : // +1 -j
		      for (i=0; i<8; i++)
			{
			  *(p_dl_ch_0+2*i)   = *(p_dl_ch_0+2*i)+*(p_dl_ch_1+2*i+1);
			  *(p_dl_ch_0+2*i+1) = *(p_dl_ch_0+2*i+1)-*(p_dl_ch_1+2*i);
			}
		      break;
		    }
						
		  for (i=0; i<8; i++)
		    {
		      if (mod_order>2)
			{
			  // Get channel amplitude if not QPSK
			  *(p_dl_ch_mag+2*i)   = (float) ((*(p_dl_ch_0+2*i)/pow(2, 15))*(*(p_dl_ch_0+2*i)/pow(2, 15)) + (*(p_dl_ch_0+2*i+1)/pow(2, 15))*(*(p_dl_ch_0+2*i+1)/pow(2, 15)))*QAM_amp; // Re{dl_ch_mag} = ||h1||^2*2/sqrt(10)
			  *(p_dl_ch_mag+2*i+1) = *(p_dl_ch_mag+2*i) ;// Im{dl_ch_mag} = Re{dl_ch_mag}
			  
			  *(p_dl_ch_magb+2*i)   = (float) ((*(p_dl_ch_1+2*i)/pow(2, 15))*(*(p_dl_ch_1+2*i)/pow(2, 15)) + (*(p_dl_ch_1+2*i+1)/pow(2, 15))*(*(p_dl_ch_1+2*i+1)/pow(2, 15)))*QAM_ampb; // Re{dl_ch_magb} = ||h2||^2*2/sqrt(10)
			  *(p_dl_ch_magb+2*i+1) = *(p_dl_ch_magb+2*i) ;// Im{dl_ch_magb} = Re{dl_ch_magb}
			}
		      
		      *(p_rxdataF_comp+2*i)   = (float) ((*(p_dl_ch_0+2*i)/pow(2, 15))*(*(p_rxdataF+2*i)/pow(2, 15)) + (*(p_dl_ch_0+2*i+1)/pow(2, 15))*(*(p_rxdataF+2*i+1)/pow(2, 15))); // Re{rxdataF_comp} = Re{h1}Re{y} + Im{h1}Im{y}
		      *(p_rxdataF_comp+2*i+1) = (float) ((*(p_dl_ch_0+2*i)/pow(2, 15))*(*(p_rxdataF+2*i+1)/pow(2, 15)) - (*(p_dl_ch_0+2*i+1)/pow(2, 15))*(*(p_rxdataF+2*i)/pow(2, 15))); // Re{rxdataF_comp} = Re{h1}Im{y} - Im{h1}Re{y}
		    }
		  p_dl_ch_0      = p_dl_ch_0+16;
		  p_dl_ch_1      = p_dl_ch_1+16;
		  
		  p_dl_ch_mag    = p_dl_ch_mag+16;
		  p_dl_ch_magb   = p_dl_ch_magb+16;
		  
		  p_rxdataF      = p_rxdataF+16;
		  p_rxdataF_comp = p_rxdataF_comp+16;		  
		}
	    } // End nb_rb
	  Nre = (pilots==0) ? 12 : 8;
	  
	  precoded_signal_strength += ((signal_energy_nodc(&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*Nre],
						       (nb_rb*Nre))*rx_power_correction) - (phy_measurements->n0_power[aarx]));
	  //msg("precoded_signal_strength=%d\n", precoded_signal_strength);
	} // rx_antennas
      
      phy_measurements->precoded_cqi_dB[eNB_id][0] = dB_fixed2(precoded_signal_strength,phy_measurements->n0_power_tot);
    }
}

int rx_pdsch_full_flp(PHY_VARS_UE *phy_vars_ue,
		      PDSCH_t type,
		      unsigned char eNB_id,
		      unsigned char eNB_id_i, //if this == NUMBER_OF_eNB_MAX, we assume MU interference
		      u8 subframe,
		      unsigned char symbol,
		      unsigned char first_symbol_flag,
		      unsigned char dual_stream_flag,
		      unsigned char i_mod)
{  
  LTE_UE_COMMON *lte_ue_common_vars  = &phy_vars_ue->lte_ue_common_vars;
  LTE_UE_PDSCH_FLP **lte_ue_pdsch_vars_flp;
  LTE_DL_FRAME_PARMS *frame_parms    = &phy_vars_ue->lte_frame_parms;
  PHY_MEASUREMENTS *phy_measurements = &phy_vars_ue->PHY_measurements;
  LTE_UE_DLSCH_t   **dlsch_ue;

  unsigned short nb_rb;

  unsigned char aatx,aarx,symbol_mod;
  int avgs, rb;

  unsigned char harq_pid0;

  switch (type)
    {
    case SI_PDSCH:
      msg("SI_PDSCH => Not considered\n");
      return(-1);
      // lte_ue_pdsch_vars_flp = &phy_vars_ue->lte_ue_pdsch_vars_SI[eNB_id];
      // dlsch_ue              = &phy_vars_ue->dlsch_ue_SI[eNB_id];
      break;
    case RA_PDSCH:
      msg("RA_PDSCH => Not considered\n");
      return(-1);
      // lte_ue_pdsch_vars_flp = &phy_vars_ue->lte_ue_pdsch_vars_ra[eNB_id];
      // dlsch_ue              = &phy_vars_ue->dlsch_ue_ra[eNB_id];
      break;
    case PDSCH:
      msg("PDSCH\n");
      lte_ue_pdsch_vars_flp = &phy_vars_ue->lte_ue_pdsch_vars_flp[eNB_id];
      dlsch_ue              = phy_vars_ue->dlsch_ue[eNB_id];
      break;
    case PMCH:
      msg("PMCH => Not considered\n");
      return(-1);
      // msg("[PHY][UE %d][FATAL] Frame %d subframe %d: PMCH not supported yet\n",phy_vars_ue->frame,subframe,type);
      // mac_xface->macphy_exit("");
      // return(-1);
      break;
    default:
      msg("default => Not considered\n");
      return(-1);
      // msg("[PHY][UE %d][FATAL] Frame %d subframe %d: Unknown PDSCH format %d\n",phy_vars_ue->frame,subframe,type);
      // mac_xface->macphy_exit("");
      // return(-1);
      break;
    }
  
  if (eNB_id > 2)
    {
      msg("dlsch_demodulation_flp.c: Illegal eNB_id %d\n",eNB_id);
      return(-1);
    }

  if (!lte_ue_common_vars)
    {
      msg("dlsch_demodulation_flp.c: Null lte_ue_common_vars\n");
      return(-1);
    }

  if (!dlsch_ue[0])
    {
      msg("dlsch_demodulation_flp.c: Null dlsch_ue pointer\n");
      return(-1);
    }
  
  if (!lte_ue_pdsch_vars_flp)
    {
      msg("dlsch_demodulation_flp.c: Null lte_ue_pdsch_vars_flp pointer\n");
      return(-1);
    }
  
  if (!frame_parms)
    {
      msg("dlsch_demodulation_flp.c: Null lte_frame_parms\n");
      return(-1);
    }
  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  harq_pid0 = dlsch_ue[0]->current_harq_pid;

  if (frame_parms->nb_antennas_tx>1)
    {
      msg("dlsch_extract_rbs_dual, step 0.\n");
      nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				     lte_ue_common_vars->dl_ch_estimates[eNB_id],
				     lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext,
				     lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext,
				     dlsch_ue[0]->pmi_alloc,
				     lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext,
				     dlsch_ue[0]->rb_alloc,
				     symbol,
				     frame_parms);
            
      if (dual_stream_flag==1)
	{
	  if (eNB_id_i!=NUMBER_OF_eNB_MAX)
	    {
	      msg("dlsch_extract_rbs_dual, step 1.\n");
	      nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
					     lte_ue_common_vars->dl_ch_estimates[eNB_id_i],
					     lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_ext,
					     lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext,
					     dlsch_ue[0]->pmi_alloc,
					     lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext,
					     dlsch_ue[0]->rb_alloc,
					     symbol,
					     frame_parms);
	    }
	  else
	    {
	      msg("dlsch_extract_rbs_dual, step 2.\n");
	      nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
					     lte_ue_common_vars->dl_ch_estimates[eNB_id],
					     lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_ext,
					     lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext,
					     dlsch_ue[0]->pmi_alloc,
					     lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext,
					     dlsch_ue[0]->rb_alloc,
					     symbol,
					     frame_parms);
	    }
	}
    } // if n_tx>1
  else
    { 
      msg("dlsch_extract_rbs_single, step 0.\n");
      nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
				       lte_ue_common_vars->dl_ch_estimates[eNB_id],
				       lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext,
				       lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext,
				       dlsch_ue[0]->pmi_alloc,
				       lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext,
				       dlsch_ue[0]->rb_alloc,
				       symbol,
				       subframe,
				       frame_parms);
      
      if (dual_stream_flag==1)
	{
	  if (eNB_id_i!=NUMBER_OF_eNB_MAX)
	    {
	      msg("dlsch_extract_rbs_single, step 1.\n");
	      nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
					       lte_ue_common_vars->dl_ch_estimates[eNB_id_i],
					       lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_ext,
					       lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext,    
					       dlsch_ue[0]->pmi_alloc,
					       lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext,
					       dlsch_ue[0]->rb_alloc,
					       symbol,
					       subframe,
					       frame_parms);
	    }
	  else
	    {
	      msg("dlsch_extract_rbs_single, step 2.\n");
	      nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
					       lte_ue_common_vars->dl_ch_estimates[eNB_id],
					       lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_ext,
					       lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext,    
					       dlsch_ue[0]->pmi_alloc,
					       lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext,
					       dlsch_ue[0]->rb_alloc,
					       symbol,
					       subframe,
					       frame_parms);
	    }
	}
    } //else n_tx>1
  
  if (nb_rb==0) 
    {
      msg("dlsch_modulation.c: nb_rb=0\n");
      return(-1);
    }
  
  if (first_symbol_flag==1)
    {
      msg("dlsch_channel_level, step 0.\n");
      // Used for log2_maxh calculation only, thus removed
      //dlsch_channel_level(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext,
      //		  frame_parms,
      //		  avg,
      //		  symbol_mod,
      //		  nb_rb);
      
      //lte_ue_pdsch_vars_flp[eNB_id]->log2_maxh = 15; // Used in flp release (but for Fxp ins/outs only)
    }
  aatx = frame_parms->nb_antennas_tx;
  aarx = frame_parms->nb_antennas_rx;
  
  if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<UNIFORM_PRECODING11)
    {// no precoding
      msg("dlsch_channel_compensation, step 0.\n");
      return(-1);
      /*
      dlsch_channel_compensation(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext,
				 lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext,
				 lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag,
				 lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb,
				 lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,
				 (aatx>1) ? lte_ue_pdsch_vars_flp[eNB_id]->rho : NULL,
				 frame_parms,
				 symbol,
				 first_symbol_flag,
				 get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
				 nb_rb,
				 lte_ue_pdsch_vars_flp[eNB_id]->log2_maxh,
				 phy_measurements); // log2_maxh+I0_shift
      */
      
      if ((dual_stream_flag==1) && (eNB_id_i!=NUMBER_OF_eNB_MAX))
	{
	  // get MF output for interfering stream
	  msg("dlsch_channel_compensation, step 2.\n");
	  return(-1);
	  /*
	  dlsch_channel_compensation(lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_ext,
				     lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext,
				     lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_mag,
				     lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_magb,
				     lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp,
				     (aatx>1) ? lte_ue_pdsch_vars_flp[eNB_id_i]->rho : NULL,
				     frame_parms,
				     symbol,
				     first_symbol_flag,
				     i_mod,
				     nb_rb,
				     lte_ue_pdsch_vars_flp[eNB_id_i]->log2_maxh,
				     phy_measurements); // log2_maxh+I0_shift
	  */
	  
	  // Compute correlation between signal and interference channels
	  /*
	  dlsch_dual_stream_correlation_full_flp(frame_parms,
						 symbol,
						 nb_rb,
						 lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext,
						 lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext,
						 lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext,
						 lte_ue_pdsch_vars_flp[eNB_id]->log2_maxh);
	
	  */
	}
    }
  else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<DUALSTREAM_UNIFORM_PRECODING1)
    {// single-layer precoding
      //    msg("Channel compensation for precoding\n");
      if ((dual_stream_flag==1) && (eNB_id_i==NUMBER_OF_eNB_MAX))
	{
	  msg("\ndlsch_channel_compensation_prec_full_flp, step 0.\n");
	  
	  /*
	  int i;
	  int index = 8; // Up to 300
	  msg("dl_ch_0_ext before full flp (fxp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("dl_ch_1 before full flp (fxp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[2][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  */
	  
	  dlsch_channel_compensation_prec_full_flp(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext,
						   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext,
						   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag,
						   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb,
						   lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,
						   lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext,
						   frame_parms,
						   phy_measurements,
						   eNB_id,
						   symbol,
						   first_symbol_flag,
						   get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
						   nb_rb,
						   lte_ue_pdsch_vars_flp[eNB_id]->log2_maxh,
						   1);
	  
	  /*
	  msg("dl_ch_0 after full flp (fxp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("dl_ch_1 after full flp (fxp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[2][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("rxdataF_ext (fxp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("dl_ch_mag after full flp (flp)=[");
	  for (i=0; i<index; i++)
	    msg("%f,", *((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("dl_ch_magb after full flp (flp)=[");
	  for (i=0; i<index; i++)
	    msg("%f,", *((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("rxdataF_comp after full flp (flp)=[");
	  for (i=0; i<index; i++)
	    msg("%f,", *((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  */
	    
	  int i;
	  msg("dl_ch_0 flp forall=[");
	  for (i=0; i<8400; i++)
	    {
	      if (*(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[0][0]))+i) != 0)
		msg("%d(%d),", *(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[0][0]))+i), i);
	    }
	  msg("]\n");
	  
	  msg("dl_ch_1 flp forall=[");
	  for (i=0; i<8400; i++)
	    {
	      if (*(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[2][0]))+i) != 0)
		msg("%d(%d),", *(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[2][0]))+i), i);
	    }
	  msg("]\n");
	  
	  /*
	  msg("dl_ch_mag flp forall (symbol=%d)=[", symbol);
	  for (i=0; i<8400; i++)
	    {
	      if (*(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag[0][0]))+i) != 0.0)
		msg("%f(%d),", *(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag[0][0]))+i), i);
	    }
	  msg("]\n");
	  
	  msg("dl_ch_magb flp forall (symbol=%d)=[", symbol);
	  for (i=0; i<8400; i++)
	    {
	      if (*(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb[0][0]))+i) != 0.0)
		msg("%f(%d),", *(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb[0][0]))+i), i);
	    }
	  msg("]\n");
	  */
	  
	  msg("rxdataF flp forall (symbol=%d)=[", symbol);
	  for (i=0; i<8400; i++)
	    {
	      if (*(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext[0][0]))+i) != 0.0)
		msg("%d(%d),", *(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext[0][0]))+i), i);
	    }
	  msg("]\n");
	  
	  msg("h1'*y flp forall (symbol=%d)=[", symbol);
	  for (i=0; i<8400; i++)
	    {
	      if (*(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp[0][0]))+i) != 0.0)
		msg("%f(%d),", *(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp[0][0]))+i), i);
	    }
	  msg("]\n");
	  	  
	  // If interference source is MU interference, assume opposite precoder was used at eNB
	  // Calculate opposite PMI
	  for (rb=0;rb<nb_rb;rb++)
	    {
	      switch(lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[rb])
		{
		case 0:
		  lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext[rb]=1;
		  break;
		case 1:
		  lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext[rb]=0;
		  break;
		case 2:
		  lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext[rb]=3;
		  break;
		case 3:
		  lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext[rb]=2;
		  break;
		}
	    }
	  
	  // Apply opposite precoder to calculate interfering stream
	  msg("\ndlsch_channel_compensation_prec_full_flp, step 1.\n");
	  
	  dlsch_channel_compensation_prec_full_flp(lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_ext,
						   lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext,
						   lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_mag,
						   lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_magb,
						   lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp,
						   lte_ue_pdsch_vars_flp[eNB_id_i]->pmi_ext,
						   frame_parms,
						   phy_measurements,
						   eNB_id_i,
						   symbol,
						   first_symbol_flag,
						   i_mod,
						   nb_rb,
						   lte_ue_pdsch_vars_flp[eNB_id]->log2_maxh,
						   1);
	  
	  /*
	  msg("dl_ch_estimates_ext after full flp (fxp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("rxdataF_ext after full flp (fxp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("dl_ch_mag after full flp (flp)=[");
	  for (i=0; i<index; i++)
	    msg("%f,", *((double *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  
	  msg("rxdataF_comp after full flp (flp)=[");
	  for (i=0; i<index; i++)
	    msg("%f,", *((double *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12])+i));
	  msg("], pmi=%d\n", lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext[0]);
	  */
	  
	  // Compute correlation between precoded channel and channel precoded with opposite PMI
	  msg("\ndlsch_dual_stream_correlation_full_flp\n");
	  
	  /*
	  int symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
	  msg("dl_ch_0 before dlsch_dual_stream_correlation_full_flp full flp (flp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext[0][symbol_mod*frame_parms->N_RB_DL*12])+i));
	  msg("]\n");
	  
	  msg("dl_ch_1] before dlsch_dual_stream_correlation_full_flp full flp (flp)=[");
	  for (i=0; i<index; i++)
	    msg("%d,", *((short *)&(lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext[0][symbol_mod*frame_parms->N_RB_DL*12])+i));
	  msg("]\n");
	  */

	  dlsch_dual_stream_correlation_full_flp(frame_parms,
						 symbol,
						 nb_rb,
						 lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext,
						 lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_estimates_ext,
						 lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext,
						 lte_ue_pdsch_vars_flp[eNB_id]->log2_maxh);
	  
	  /*
	  msg("rho flp forall (symbol=%d)=[", symbol);
	  for (i=0; i<8400; i++)
	    {
	      if (*(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext[0][0]))+i) != 0.0)
		msg("%f(%d),", *(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext[0][0]))+i), i);
	    }
	  msg("]\n");
	  */
	}
      else
	{
	  msg("dlsch_channel_compensation_prec, step 3.\n");
	  dlsch_channel_compensation_prec_full_flp(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_ext,
						   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_estimates_ext,
						   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag,
						   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb,
						   lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,
						   lte_ue_pdsch_vars_flp[eNB_id]->pmi_ext,
						   frame_parms,
						   phy_measurements,
						   eNB_id,
						   symbol,
						   first_symbol_flag,
						   get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
						   nb_rb,
						   lte_ue_pdsch_vars_flp[eNB_id]->log2_maxh,
						   dlsch_ue[0]->dl_power_off);
	}
    }
  
  //  msg("MRC\n");
  if (frame_parms->nb_antennas_rx > 1)
    {
      msg("dlsch_detection_mrc, step 0.\n");
      
      dlsch_detection_mrc_full_flp(frame_parms,
				   lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,
				   lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp,
				   lte_ue_pdsch_vars_flp[eNB_id]->rho,
				   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext,
				   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag,
				   lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb,
				   symbol,
				   nb_rb,
				   dual_stream_flag);
      
      /*
      int i;
      if (symbol==13)
	{
	  msg("h1'y full flp (flp)=[");
	  for (i=0; i<8400; i++)
	    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp[0][0])+i), i);
	  msg("]\n");
	  
	  msg("h2'y full flp (flp)=[");
	  for (i=0; i<8400; i++)
	    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp[0][0])+i), i);
	  msg("]\n");
	  
	  msg("mag flp (flp)=[");
	  for (i=0; i<8400; i++)
	    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag[0][0])+i), i);
	  msg("]\n");
	  
	  msg("magb flp (flp)=[");
	  for (i=0; i<8400; i++)
	    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb[0][0])+i), i);
	  msg("]\n");
	  
	  msg("rho flp (flp)=[");
	  for (i=0; i<8400; i++)
	    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext[0][0])+i), i);
	  msg("]\n");		  
	}
      */
    }
  //  msg("Combining");
  // Single-layer transmission formats
  if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<DUALSTREAM_UNIFORM_PRECODING1)
    {
      if ((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == SISO) ||
	  ((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode >= UNIFORM_PRECODING11) &&
	   (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode <= PUSCH_PRECODING0)))
	{}//dlsch_siso(frame_parms,lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp,symbol,nb_rb);
      else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == ALAMOUTI)
	{
	  msg("Alamouti\n");
	  return(-1);
	  /*
	  dlsch_alamouti(frame_parms,lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag,lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb,symbol,nb_rb);
	  */
	  }
      else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == ANTCYCLING)
	{
	  msg("antcyc\n");
	  return(-1);
	  /*
	  dlsch_antcyc(frame_parms,lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag,lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_magb,symbol,nb_rb);
	  */
	}
      else
	{
	  msg("dlsch_rx: Unknown MIMO mode\n");
	  return (-1);
	}
      
      // msg("LLR");
      msg("switch (get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs))\n");
      switch (get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs))
	{
	  case 2 : 
	  msg("rx_dlsch.c : flp-based (IC) receiver not implemented yet for 4QAM(4-QAM)\n");
	  return(-1);
	  break;
	case 4 :
	  if (dual_stream_flag == 0)
	    {
	      msg("rx_dlsch.c : flp-based receiver not implemented yet for 16-QAM\n");
	      return(-1);
	    }
	  else if (i_mod == 2)
	    {
	      msg("rx_dlsch.c : IC receiver only implemented for 16QAM-16QAM\n");
	      return(-1);
	    }
	  else if (i_mod == 4)
	    {
	      msg("\ndlsch_16qam_16qam_llr_full_flp, step 0.\n");
	      
	      /*
	      msg("rxdataF_comp[eNB_id] before dlsch_16qam_16qam_llr full flp (flp)=[");
	      int i;
	      int index = 8; // Up to 300
	      for (i=0; i<index; i++)
		msg("%f,", *((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12])+i));
	      msg("]\n");
	      
	      msg("rxdataF_comp[eNB_id_i] before dlsch_16qam_16qam_llr full flp (flp)=[");
	      for (i=0; i<index; i++)
		msg("%f,", *((float *)&(lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12])+i));
	      msg("]\n");
	      
	      msg("dl_ch_mag[eNB_id] before dlsch_16qam_16qam_llr full flp (flp)=[");
	      for (i=0; i<index; i++)
		msg("%f,", *((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag[0][symbol*frame_parms->N_RB_DL*12])+i));
	      msg("]\n");
	      
	      msg("dl_ch_mag[eNB_id_i] before dlsch_16qam_16qam_llr full flp (flp)=[");
	      for (i=0; i<index; i++)
		msg("%f,", *((float *)&(lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_mag[0][symbol*frame_parms->N_RB_DL*12])+i));
	      msg("]\n");
	      
	      msg("dl_ch_rho_ext[eNB_id] before dlsch_16qam_16qam_llr full flp (flp)=[");
	      for (i=0; i<index; i++)
		msg("%f,", *((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext[0][symbol*frame_parms->N_RB_DL*12])+i));
	      msg("]\n");
	      */

	      
	      int i;
	      if (symbol==13)
		{
		  msg("h1'y full flp (flp)=[");
		  for (i=0; i<8400; i++)
		    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp[0][0])+i), i);
		  msg("]\n");
		  
		  msg("h2'y full flp (flp)=[");
		  for (i=0; i<8400; i++)
		    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp[0][0])+i), i);
		  msg("]\n");
		  
		  msg("2/sqrt(10)*||h1||^2 fxp (fxp)=[");
		  for (i=0; i<8400; i++)
		    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag[0][0])+i), i);
		  msg("]\n");
		  
		  msg("2/sqrt(10)*||h2||^2 fxp (fxp)=[");
		  for (i=0; i<8400; i++)
		    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_mag[0][0])+i), i);
		  msg("]\n");
		  
		  msg("rho fxp (fxp)=[");
		  for (i=0; i<8400; i++)
		    msg("%f(%d),", *(((float *)&lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext[0][0])+i), i);
		  msg("]\n");		  
		}
	      
		  
#ifdef LLR_FLP
	      dlsch_16qam_16qam_llr_only_flp(frame_parms,
					     lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,
					     lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp,
					     lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag,
					     lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_mag,
					     lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext,
					     lte_ue_pdsch_vars_flp[eNB_id]->llr_flp[0],
					     symbol,first_symbol_flag,nb_rb,
					     adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
					     lte_ue_pdsch_vars_flp[eNB_id]->llr128_flp);

	      
	      if (symbol==13)
		{
		  msg("llr only flp forall (symbol=%d)=[", symbol);
		  for (i=0; i<(300*8+200*3)*4; i++)
		    {
		      if (*(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->llr_flp[0][0]))+i) != 0.0)
			msg("%f(%d),", *(((float *)&(lte_ue_pdsch_vars_flp[eNB_id]->llr_flp[0][0]))+i), i);
		    }
		  msg("]\n");
		}
	      
#else
	      dlsch_16qam_16qam_llr_full_flp(frame_parms,
					     lte_ue_pdsch_vars_flp[eNB_id]->rxdataF_comp,
					     lte_ue_pdsch_vars_flp[eNB_id_i]->rxdataF_comp,
					     lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_mag,
					     lte_ue_pdsch_vars_flp[eNB_id_i]->dl_ch_mag,
					     lte_ue_pdsch_vars_flp[eNB_id]->dl_ch_rho_ext,
					     lte_ue_pdsch_vars_flp[eNB_id]->llr[0],
					     symbol,first_symbol_flag,nb_rb,
					     adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
					     lte_ue_pdsch_vars_flp[eNB_id]->llr128);

	      /*
	      if (symbol==13)
		{
		  msg("llr full flp forall (symbol=%d)=[", symbol);
		  for (i=0; i<(300*8+200*3)*4; i++)
		    {
		      if (*(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->llr[0][0]))+i) != 0)
			msg("%d(%d),", *(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->llr[0][0]))+i), i);
		    }
		  msg("]\n");
		}
	      */
#endif      

	      if (0)
		{
		  /*
		  msg("llr[0] before normalization by max_llr (flp)=[");
		  for (i=0; i<index; i++)
		    msg("%d,", *(lte_ue_pdsch_vars_flp[eNB_id]->llr[0]+i));
		  msg("]\n");
		  */
		  
		  // Normalization: max llr value set to 2^15-1
		  float max_abs_llr = 0.0;
		  int iLLR;
		  for (iLLR=0; iLLR<(300*8+200*3)*4; iLLR++)
		    {
		      if (fabs(*(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->llr[0][0]))+iLLR)/(pow(2, 15)-1))>max_abs_llr)
			max_abs_llr = fabs(*(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->llr[0][0]))+iLLR)/(pow(2, 15)-1));
		    }
		  for (iLLR=0; iLLR<(300*8+200*3)*4; iLLR++)
		    {
		      *(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->llr[0][0]))+iLLR) = (short)(((float)(*(((short *)&(lte_ue_pdsch_vars_flp[eNB_id]->llr[0][0]))+iLLR))/(pow(2, 15)-1)/max_abs_llr)*(pow(2, 15)-1));
		    }
		  
		  /*
		  msg("llr[0] after normalization by max_llr (flp)=[");
		  for (i=0; i<index; i++)
		    msg("%d,", *(lte_ue_pdsch_vars_flp[eNB_id]->llr[0]+i));
		  msg("]\n");
		  */
		}
	    }
	  else
	    {
	      msg("rx_dlsch.c : IC receiver only implemented for 16QAM-16QAM\n");
	      return(-1);
	    }
	  break;
	case 6 :
	  msg("rx_dlsch.c : flp-based (IC) receiver not implemented yet for 64QAM(64-QAM)\n");
	  return(-1);
	  break;
	default:
	  msg("rx_dlsch.c : Unknown mod_order!!!!\n");
	  return(-1);
	  break;
	}
    } // single-layer transmission
  else 
    {
      msg("rx_dlsch.c : Dualstream not yet implemented\n");
      return(-1);
    }
  
  return(0);    
}
