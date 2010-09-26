#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "defs.h"
#include "SIMULATION/RF/defs.h"

#define MAX_CHANNEL_LENGTH 200

//#define DEBUG_CH

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
  double path_loss = pow(10,desc->path_loss_dB/20);
  int dd;
  dd = -desc->channel_offset;
#ifdef DEBUG_CH
    printf("[CHANNEL] path_loss = %g (%f), nb_rx %d, nb_tx %d, dd %d, len %d \n",path_loss,desc->path_loss_dB,desc->nb_rx,desc->nb_tx,dd,desc->channel_length);
#endif
  /*  
  for (i=0;i<desc->nb_rx;i++)      // RX Antenna loop
    for (j=0;j<desc->nb_tx;j++) {  // TX Antenna loop
      
      if (keep_channel) {
	// do nothing - keep channel
      } else {

	
	memset(desc->ch[i + (j*desc->nb_rx)], 0,desc->channel_length * sizeof(struct complex));


      }
    }
  */



  random_channel(desc);
#ifdef DEBUG_CH
  for (l = 0;l<(int)desc->channel_length;l++) {
    printf("%p (%f,%f) ",desc->ch[0],desc->ch[0][l].r,desc->ch[0][l].i);
  }
  printf("\n");
#endif

  for (i=dd;i<((int)length+dd);i++) {
    for (ii=0;ii<desc->nb_rx;ii++) {
      rx_tmp.r = 0;
      rx_tmp.i = 0;
      for (j=0;j<desc->nb_tx;j++) {


	for (l = 0;l<(int)desc->channel_length;l++) {
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
      /*
      if ((ii==0)&&((i%32)==0)) {
	printf("%p %p %f,%f => %e,%e\n",rx_sig_re[ii],rx_sig_im[ii],rx_tmp.r,rx_tmp.i,rx_sig_re[ii][i-dd],rx_sig_im[ii][i-dd]);
      }
      */
      //rx_sig_re[ii][i] = sqrt(.5)*(tx_sig_re[0][i] + tx_sig_re[1][i]);
      //rx_sig_im[ii][i] = sqrt(.5)*(tx_sig_im[0][i] + tx_sig_im[1][i]);
      
    } // ii
  } // i
}



