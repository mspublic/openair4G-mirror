/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "defs.h"
#include "SIMULATION/RF/defs.h"

//#define DEBUG_CH
uint8_t multipath_channel_nosigconv(channel_desc_t *desc){
  
  random_channel(desc,0);
       return(1);
}

#define CHANNEL_SSE
#ifdef CHANNEL_SSE
void multipath_channel(channel_desc_t *desc,
		       double **tx_sig_re, 
		       double **tx_sig_im, 
		       double **rx_sig_re,
		       double **rx_sig_im,
		       uint32_t length,
		       uint8_t keep_channel) {
 
  int i,ii,j,l;
  int length1, length2, tail;
  __m128d rx_tmp128_re_f,rx_tmp128_im_f,rx_tmp128_re,rx_tmp128_im, rx_tmp128_1,rx_tmp128_2,rx_tmp128_3,rx_tmp128_4,tx128_re,tx128_im,ch128_x,ch128_y,pathloss128;

  double path_loss = pow(10,desc->path_loss_dB/20);
  int dd = abs(desc->channel_offset);

  pathloss128 = _mm_set1_pd(path_loss);

#ifdef DEBUG_CH
  printf("[CHANNEL] keep = %d : path_loss = %g (%f), nb_rx %d, nb_tx %d, dd %d, len %d \n",keep_channel,path_loss,desc->path_loss_dB,desc->nb_rx,desc->nb_tx,dd,desc->channel_length);
#endif

    if (keep_channel) {
      // do nothing - keep channel
    } else {
      random_channel(desc,0);
    }

    start_meas(&desc->convolution);

#ifdef DEBUG_CH
  for (l = 0;l<(int)desc->channel_length;l++) {
    printf("%p (%f,%f) ",desc->ch[0],desc->ch[0][l].x,desc->ch[0][l].y);
  }
  printf("\n");
#endif

  tail = ((int)length-dd)%2;
  if(tail)
    length1 = ((int)length-dd)-1;
  else
    length1 = ((int)length-dd);
  length2 = length1/2;
  for (i=0;i<length2;i++) { //
    for (ii=0;ii<desc->nb_rx;ii++) {
      // rx_tmp.x = 0;
      // rx_tmp.y = 0;
      rx_tmp128_re_f = _mm_setzero_pd();
      rx_tmp128_im_f = _mm_setzero_pd();
      for (j=0;j<desc->nb_tx;j++) {
	for (l = 0;l<(int)desc->channel_length;l++) {
	  if ((i>=0) && (i-l)>=0) { //SIMD correct only if length1 > 2*channel_length...which is almost always satisfied
	    // tx.x = tx_sig_re[j][i-l];
	    // tx.y = tx_sig_im[j][i-l];
	    tx128_re = _mm_loadu_pd(&tx_sig_re[j][2*i-l]); // tx_sig_re[j][i-l+1], tx_sig_re[j][i-l]
	    tx128_im = _mm_loadu_pd(&tx_sig_im[j][2*i-l]);
	  }
	  else {
	    //tx.x =0;
	    //tx.y =0;
	    tx128_re = _mm_setzero_pd();
	    tx128_im = _mm_setzero_pd();
	  }
	  ch128_x = _mm_set1_pd(desc->ch[ii+(j*desc->nb_rx)][l].x);
	  ch128_y = _mm_set1_pd(desc->ch[ii+(j*desc->nb_rx)][l].y);
	  //  rx_tmp.x += (tx.x * desc->ch[ii+(j*desc->nb_rx)][l].x) - (tx.y * desc->ch[ii+(j*desc->nb_rx)][l].y);
	  //  rx_tmp.y += (tx.y * desc->ch[ii+(j*desc->nb_rx)][l].x) + (tx.x * desc->ch[ii+(j*desc->nb_rx)][l].y);
	  rx_tmp128_1 = _mm_mul_pd(tx128_re,ch128_x);
	  rx_tmp128_2 = _mm_mul_pd(tx128_re,ch128_y);
	  rx_tmp128_3 = _mm_mul_pd(tx128_im,ch128_x);
	  rx_tmp128_4 = _mm_mul_pd(tx128_im,ch128_y);
	  rx_tmp128_re = _mm_sub_pd(rx_tmp128_1,rx_tmp128_4);
	  rx_tmp128_im = _mm_add_pd(rx_tmp128_2,rx_tmp128_3);
	  rx_tmp128_re_f = _mm_add_pd(rx_tmp128_re_f,rx_tmp128_re);
	  rx_tmp128_im_f = _mm_add_pd(rx_tmp128_im_f,rx_tmp128_im);
	} //l
      }  // j
      //rx_sig_re[ii][i+dd] = rx_tmp.x*path_loss;
      //rx_sig_im[ii][i+dd] = rx_tmp.y*path_loss;
      rx_tmp128_re_f = _mm_mul_pd(rx_tmp128_re_f,pathloss128);
      rx_tmp128_im_f = _mm_mul_pd(rx_tmp128_im_f,pathloss128);
      _mm_storeu_pd(&rx_sig_re[ii][2*i+dd],rx_tmp128_re_f); // max index: length-dd -1 + dd = length -1
      _mm_storeu_pd(&rx_sig_im[ii][2*i+dd],rx_tmp128_im_f);
      /*
      if ((ii==0)&&((i%32)==0)) {
	printf("%p %p %f,%f => %e,%e\n",rx_sig_re[ii],rx_sig_im[ii],rx_tmp.x,rx_tmp.y,rx_sig_re[ii][i-dd],rx_sig_im[ii][i-dd]);
      }
      */
      //rx_sig_re[ii][i] = sqrt(.5)*(tx_sig_re[0][i] + tx_sig_re[1][i]);
      //rx_sig_im[ii][i] = sqrt(.5)*(tx_sig_im[0][i] + tx_sig_im[1][i]);
      
    } // ii
  } // i

  stop_meas(&desc->convolution);

}

#else
void multipath_channel(channel_desc_t *desc,
		       double **tx_sig_re, 
		       double **tx_sig_im, 
		       double **rx_sig_re,
		       double **rx_sig_im,
		       uint32_t length,
		       uint8_t keep_channel) {
 
  int i,ii,j,l;
  struct complex rx_tmp,tx;

  double path_loss = pow(10,desc->path_loss_dB/20);
  int dd;
  dd = abs(desc->channel_offset);

#ifdef DEBUG_CH
  printf("[CHANNEL] keep = %d : path_loss = %g (%f), nb_rx %d, nb_tx %d, dd %d, len %d \n",keep_channel,path_loss,desc->path_loss_dB,desc->nb_rx,desc->nb_tx,dd,desc->channel_length);
#endif

    if (keep_channel) {
      // do nothing - keep channel
    } else {
      random_channel(desc,0);
    }

#ifdef DEBUG_CH
  for (l = 0;l<(int)desc->channel_length;l++) {
    printf("%p (%f,%f) ",desc->ch[0],desc->ch[0][l].x,desc->ch[0][l].y);
  }
  printf("\n");
#endif

  for (i=0;i<((int)length-dd);i++) {
    for (ii=0;ii<desc->nb_rx;ii++) {
      rx_tmp.x = 0;
      rx_tmp.y = 0;
      for (j=0;j<desc->nb_tx;j++) {
	for (l = 0;l<(int)desc->channel_length;l++) {
	  if ((i>=0) && (i-l)>=0) {
	    tx.x = tx_sig_re[j][i-l];
	    tx.y = tx_sig_im[j][i-l];
	  }
	  else {
	    tx.x =0;
	    tx.y =0;
	  }
	  rx_tmp.x += (tx.x * desc->ch[ii+(j*desc->nb_rx)][l].x) - (tx.y * desc->ch[ii+(j*desc->nb_rx)][l].y);
	  rx_tmp.y += (tx.y * desc->ch[ii+(j*desc->nb_rx)][l].x) + (tx.x * desc->ch[ii+(j*desc->nb_rx)][l].y);
	} //l
      }  // j
      rx_sig_re[ii][i+dd] = rx_tmp.x*path_loss;
      rx_sig_im[ii][i+dd] = rx_tmp.y*path_loss;
      /*
      if ((ii==0)&&((i%32)==0)) {
	printf("%p %p %f,%f => %e,%e\n",rx_sig_re[ii],rx_sig_im[ii],rx_tmp.x,rx_tmp.y,rx_sig_re[ii][i-dd],rx_sig_im[ii][i-dd]);
      }
      */
      //rx_sig_re[ii][i] = sqrt(.5)*(tx_sig_re[0][i] + tx_sig_re[1][i]);
      //rx_sig_im[ii][i] = sqrt(.5)*(tx_sig_im[0][i] + tx_sig_im[1][i]);
      
    } // ii
  } // i
}
#endif


