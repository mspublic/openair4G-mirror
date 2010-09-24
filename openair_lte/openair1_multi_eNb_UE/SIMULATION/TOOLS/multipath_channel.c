#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "defs.h"
#include "SIMULATION/RF/defs.h"

#define MAX_CHANNEL_LENGTH 200
//struct complex a[6][4][4][MAX_CHANNEL_LENGTH];
// 6 = #number of different channels between 4 units (f.ex 2 eNbs and 2 UEs)

void multipath_channel(channel_desc_t *desc,
		       double **tx_sig_re, 
		       double **tx_sig_im, 
		       double **rx_sig_re,
		       double **rx_sig_im,
		       u16 length,
		       u8 keep_channel) {
 
  int i,ii,j,l;
  struct complex rx_tmp,tx;
  struct complex phase;
  double path_loss = pow(10,-desc->path_loss_dB/20);
  int dd;
  dd = -desc->channel_offset;
#ifdef DEBUG_PHY
  //  printf("path_loss = %g\n",path_loss);
#endif
  
  for (i=0;i<desc->nb_rx;i++)      // RX Antenna loop
    for (j=0;j<desc->nb_tx;j++) {  // TX Antenna loop
      
      if (keep_channel) {
	// do nothing - keep channel
      } else {

	
	memset(desc->ch[i + (j*desc->nb_rx)], 0,desc->channel_length * sizeof(struct complex));


      }
      //ch[i + (j*nb_antennas_rx)][0].r=1;
      //ch[i + (j*nb_antennas_rx)][0].i=0;

    }

  random_channel(desc);

  for (i=dd;i<((int)length+dd);i++) {
    for (ii=0;ii<desc->nb_rx;ii++) {
      rx_tmp.r = 0;
      rx_tmp.i = 0;
      for (j=0;j<desc->nb_tx;j++) {


	for (l = 0;l<desc->channel_length;l++) {
	  if ((i>=0) && (i-l)>=0) {
	    tx.r = tx_sig_re[j][i-l];
	    tx.i = tx_sig_im[j][i-l];
	  }
	  else {
	    tx.r =0;
	    tx.i =0;
	  }
	  rx_tmp.r += (tx.r * desc->ch[ii+(j*desc->nb_rx)][l].r) - (tx.i * desc->ch[ii+(j*desc->nb_rx)][l].i);
	  rx_tmp.i += (tx.i * desc->ch[ii+(j*desc->nb_rx)][l].r) + (tx.r * desc->ch[ii+(j*desc->nb_rx)][l].i);
	} //l
      }  // j
      rx_sig_re[ii][i-dd] = rx_tmp.r*path_loss;
      rx_sig_im[ii][i-dd] = rx_tmp.i*path_loss;
      //rx_sig_re[ii][i] = sqrt(.5)*(tx_sig_re[0][i] + tx_sig_re[1][i]);
      //rx_sig_im[ii][i] = sqrt(.5)*(tx_sig_im[0][i] + tx_sig_im[1][i]);
      
    } // ii
  } // i
}



