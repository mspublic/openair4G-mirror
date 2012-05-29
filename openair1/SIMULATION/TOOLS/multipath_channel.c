#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "defs.h"
#include "SIMULATION/RF/defs.h"

//#define DEBUG_CH
/*
int init_multipath_channel(channel_desc_t *channel) {
  int i;
  if (channel->nb_taps<=0) {
    msg("init_multipath_channel: nb_taps must be > 0\n");
    return(-1);
  }
  if (channel->channel_length<=0) {
    msg("init_multipath_channel: channel_length must be > 0\n");
    return(-1);
  }
  if (channel->nb_tx<=0) {
    msg("init_multipath_channel: nb_tx must be > 0\n");
    return(-1);
  }
  if (channel->nb_rx<=0) {
    msg("init_multipath_channel: nb_rx must be > 0\n");
    return(-1);
  }
  channel->amps = (double*) malloc(channel->nb_taps*sizeof(double));
  if (!channel->amps) {
    msg("init_multipath_channel: cannot allocate amps\n");
    return(-1);
  }
  channel->delays = (double*) malloc(channel->nb_taps*sizeof(double));
  if (!channel->delays) {
    msg("init_multipath_channel: cannot allocate delays\n");
    return(-1);
  }
  channel->state = (struct complex **) malloc(channel->nb_tx*channel->nb_rx*sizeof(struct complex *));
  if (!channel->state) {
    msg("init_multipath_channel: cannot allocate state\n");
    return(-1);
  }
  for (i=0; i<channel->nb_tx*channel->nb_rx; i++) {
    channel->state[i] = (struct complex *) malloc(channel->nb_taps*sizeof(struct complex));
    if (!channel->state[i]) {
      msg("init_multipath_channel: cannot allocate state\n");
      return(-1);
    }
  }
  channel->ch = (struct complex **) malloc(channel->nb_tx*channel->nb_rx*sizeof(struct complex *));
  if (!channel->ch) {
    msg("init_multipath_channel: cannot allocate ch\n");
    return(-1);
  }
  for (i=0; i<channel->nb_tx*channel->nb_rx; i++) {
    channel->ch[i] = (struct complex *) malloc(channel->channel_length*sizeof(struct complex));
    if (!channel->ch[i]) {
      msg("init_multipath_channel: cannot allocate ch\n");
      return(-1);
    }
  }
  channel->R_tx_sqrt = (struct complex **) malloc(channel->nb_tx*channel->nb_tx*sizeof(struct complex *));
  if (!channel->Rx_tx_sqrt) {
    msg("init_multipath_channel: cannot allocate R_tx_sqrt\n");
    return(-1);
  }
  for (i=0; i<channel->nb_tx*channel->nb_tx; i++) {
    channel->R_tx_sqrt[i] = (struct complex *) malloc(channel->channel_length*sizeof(struct complex));
    if (!channel->R_tx_sqrt[i]) {
      msg("init_multipath_channel: cannot allocate R_tx_sqrt\n");
      return(-1);
    }
  }
  channel->R_rx_sqrt = (struct complex **) malloc(channel->nb_rx*channel->nb_rx*sizeof(struct complex *));
  if (!channel->R_rx_sqrt) {
    msg("init_multipath_channel: cannot allocate R_rx_sqrt\n");
    return(-1);
  }
  for (i=0; i<channel->nb_rx*channel->nb_rx; i++) {
    channel->R_rx_sqrt[i] = (struct complex *) malloc(channel->channel_length*sizeof(struct complex));
    if (!channel->R_rx_sqrt[i]) {
      msg("init_multipath_channel: cannot allocate R_rx_sqrt\n");
      return(-1);
    }
  }
  return(0);
}

int free_multipath_channel(channel_desc_t *channel) {
  int i;
  free(channel->amps);
  free(channel->delays);
  for (i=0; i<channel->nb_tx*channel->nb_rx; i++) {
    free(channel->state[i] = (struct complex *) malloc(channel->nb_taps*sizeof(struct complex));
    if (!channel->state[i]) {
      msg("init_multipath_channel: cannot allocate state\n");
      return(-1);
    }
  }
  free(channel->state);
  channel->ch = (struct complex **) malloc(channel->nb_tx*channel->nb_rx*sizeof(struct complex *));
  if (!channel->ch) {
    msg("init_multipath_channel: cannot allocate ch\n");
    return(-1);
  }
  for (i=0; i<channel->nb_tx*channel->nb_rx; i++) {
    channel->ch[i] = (struct complex *) malloc(channel->channel_length*sizeof(struct complex));
    if (!channel->ch[i]) {
      msg("init_multipath_channel: cannot allocate ch\n");
      return(-1);
    }
  }
  channel->R_tx_sqrt = (struct complex *) malloc(channel->nb_tx*channel->nb_tx*sizeof(struct complex));
  if (!channel->Rx_tx_sqrt) {
    msg("init_multipath_channel: cannot allocate Rx_tx_sqrt\n");
    return(-1);
  }
  channel->R_rx_sqrt = (struct complex *) malloc(channel->nb_rx*channel->nb_rx*sizeof(struct complex));
  if (!channel->R_rx_sqrt) {
    msg("init_multipath_channel: cannot allocate R_rx_sqrt\n");
    return(-1);
  }
  return(0);
}
*/

void multipath_channel(channel_desc_t *desc,
		       double **tx_sig_re, 
		       double **tx_sig_im, 
		       double **rx_sig_re,
		       double **rx_sig_im,
		       u32 length,
		       u8 keep_channel) {
 
  int i,ii,j,l;
  double complex rx_tmp,tx;
  double path_loss = pow(10,desc->path_loss_dB/20);
  int dd;
  dd = -desc->channel_offset;

#ifdef DEBUG_CH
  printf("[CHANNEL] keep = %d : path_loss = %g (%f), nb_rx %d, nb_tx %d, dd %d, len %d \n",keep_channel,path_loss,desc->path_loss_dB,desc->nb_rx,desc->nb_tx,dd,desc->channel_length);
#endif

    if (keep_channel) {
      // do nothing - keep channel
    } else {
      random_channel(desc);
    }

#ifdef DEBUG_CH
  for (l = 0;l<(int)desc->channel_length;l++) {
    printf("%p (%f,%f) ",desc->ch[0],creal(desc->ch[0][l]),cimag(desc->ch[0][l]));
  }
  printf("\n");
#endif

  for (i=dd;i<((int)length+dd);i++) {
    for (ii=0;ii<desc->nb_rx;ii++) {
      rx_tmp = 0;
      for (j=0;j<desc->nb_tx;j++) {
	for (l = 0;l<(int)desc->channel_length;l++) {
	  if ((i>=0) && (i-l)>=0) {
	    tx = tx_sig_re[j][i-l]+I*tx_sig_im[j][i-l];
	  }
	  else {
	    tx =0;
	  }
	  rx_tmp += (creal(tx) * creal(desc->ch[ii+(j*desc->nb_rx)][l]) - cimag(tx) * cimag(desc->ch[ii+(j*desc->nb_rx)][l]))+I*(cimag(tx) * creal(desc->ch[ii+(j*desc->nb_rx)][l]) + creal(tx) * cimag(desc->ch[ii+(j*desc->nb_rx)][l]));
	} //l
      }  // j
      rx_sig_re[ii][i-dd] = creal(rx_tmp)*path_loss;
      rx_sig_im[ii][i-dd] = cimag(rx_tmp)*path_loss;
      /*
      if ((ii==0)&&((i%32)==0)) {
	printf("%p %p %f,%f => %e,%e\n",rx_sig_re[ii],rx_sig_im[ii],creal(rx_tmp),cimag(rx_tmp),rx_sig_re[ii][i-dd],rx_sig_im[ii][i-dd]);
      }
      */
    } // ii
  } // i
}

void multipath_tv_channel(channel_desc_t *desc,
			  double **tx_sig_re, 
			  double **tx_sig_im, 
			  double **rx_sig_re,
			  double **rx_sig_im,
			  u16 length,
			  u8 keep_channel) {
  
  double complex **tx,**rx,***H_t,*rx_temp; //**H,*H_tmp;
  double path_loss = pow(10,desc->path_loss_dB/20);
  int i,j,k;
  tx = (double complex **)malloc(desc->nb_tx*sizeof(double complex));
  rx = (double complex **)malloc(desc->nb_rx*sizeof(double complex));
  
  //H_tmp = (double complex*) calloc(length,sizeof(double complex));
  //H = (double complex **) malloc(length*sizeof(double complex *));
  H_t= (double complex ***) malloc(desc->nb_tx*desc->nb_rx*sizeof(double complex **));
  rx_temp= (double complex *) calloc(length,sizeof(double complex));
  
  for(i=0;i<desc->nb_tx;i++){
    tx[i] = (double complex *)calloc(length,sizeof(double complex));
  }
  
  for(i=0;i<desc->nb_rx;i++){
    rx[i] = (double complex *)calloc(length,sizeof(double complex));
  }
  
  //for(i=0;i<length;i++) {
  //  H[i] = (double complex *) calloc (desc->nb_taps,sizeof(double complex));
  //}
  
  for(i=0;i<desc->nb_tx*desc->nb_rx;i++){
    H_t[i] = (double complex **) malloc(length*sizeof(double complex *));
    for(j=0;j<length;j++) {
      H_t[i][j] = (double complex *) calloc (desc->nb_taps,sizeof(double complex));
    }
  }
  
  for (i=0;i<desc->nb_tx;i++){
    for(j=0;j<length;j++) {
      tx[i][j] = tx_sig_re[i][j] +I*tx_sig_im[i][j];   
    }
  }
  
  if (keep_channel) {
    // do nothing - keep channel
  } else {
    tv_channel(desc,H_t,length);
  }
  
  //tv_conv(H,tx[0],rx[0],length,desc->nb_taps);
   
  for(i=0;i<desc->nb_rx;i++){
     for(j=0;j<desc->nb_tx;j++){
 	tv_conv(H_t[i+(j*desc->nb_rx)],tx[j],rx_temp,length,desc->nb_taps);
	for(k=0;k<length;k++){
	  rx[i][k] += rx_temp[k];
	}
     }	
  }

  for(i=0;i<desc->nb_rx;i++){
    for(j=0;j<length;j++){
      rx_sig_re[i][j] = creal(rx[i][j])*path_loss;
      rx_sig_im[i][j] = cimag(rx[i][j])*path_loss;
    }
  }
  
  /*
    for(i=0;i<length;i++){
    rx_sig_re[0][i] = creal(rx[0][i])*path_loss;
    rx_sig_im[0][i] = cimag(rx[0][i])*path_loss;
    }
  */
  //printf("%f vs %f", rx_sig_re[0][0],tx_sig_re[0][0]);
  
  for(i=0;i<desc->nb_tx;i++){
    free(tx[i]);
  }
  free(tx);
  for(i=0;i<desc->nb_rx;i++) {
    free(rx[i]);
  }
  free(rx);
  
  //for(i=0;i<length;i++) {
  //  free(H[i]);
  //}
  //free(H);
  
  for(i=0;i<desc->nb_rx*desc->nb_tx;i++){
    for(j=0;j<length;j++){
      free(H_t[i][j]);
    }
    free(H_t[i]);
  }
  free(H_t);
  
  free(rx_temp);
}

//TODO: make phi_rad a parameter of this function
void tv_channel(channel_desc_t *desc,double complex ***H,u16 length){
  
  int i,j,p,l,k;
  double *alpha,*phi_rad,pi=acos(-1),*w_Hz;
  
  alpha = (double *)calloc(desc->nb_paths,sizeof(double));
  phi_rad = (double *)calloc(desc->nb_paths,sizeof(double));
  w_Hz = (double *)calloc(desc->nb_paths,sizeof(double));
  
  //printf("max Doppler = %f\n",desc->max_Doppler);
  
  //srand(0);
  for(i=0;i<desc->nb_paths;i++) {
    w_Hz[i]=desc->max_Doppler*cos(frand_a_b(0,2*pi));
    phi_rad[i]=frand_a_b(0,2*pi);
  }
  
  if(desc->ricean_factor == 1){
    for(i=0;i<desc->nb_paths;i++) {
      alpha[i]=1/sqrt(desc->nb_paths);
    }
  }
  else {
    alpha[0]=sqrt(desc->ricean_factor/(desc->ricean_factor+1));
    for(i=1;i<desc->nb_paths;i++) {
      alpha[i] = (1/sqrt(desc->nb_paths-1))*(sqrt(1/(desc->ricean_factor+1)));
    }
  }
  /*
  // This is the code when we only consider a SISO case
  for(i=0;i<length;i++)
  {
	for(j=0;j<desc->nb_taps;j++)
	   {
		for(p=0;p<desc->nb_paths;p++)
		   {
		     H[i][j] += sqrt(desc->amps[j]/2)*alpha[p]*cexp(-I*(2*pi*w_Hz[p]*i*(1/(desc->BW*1e6))+phi_rad[p]));
		   }
   	   }
   }
for(j=0;j<desc->nb_paths;j++)
   {
	phi_rad[j] = fmod(2*pi*w_Hz[j]*(length-1)*(1/desc->BW)+phi_rad[j],2*pi);
   }
  */

  // if MIMO
  for (i=0;i<desc->nb_rx;i++){
    for(j=0;j<desc->nb_tx;j++){
      for(k=0;k<length;k++){
	for(l=0;l<desc->nb_taps;l++){
	  H[i+(j*desc->nb_rx)][k][l] = 0;
	  for(p=0;p<desc->nb_paths;p++){
	    H[i+(j*desc->nb_rx)][k][l] += sqrt(desc->amps[l]/2)*alpha[p]*cexp(I*(2*pi*w_Hz[p]*k*(1/(desc->BW*1e6))+phi_rad[p]));
	  }
	}
      }
      for(j=0;j<desc->nb_paths;j++){
	phi_rad[j] = fmod(2*pi*w_Hz[j]*(length-1)*(1/desc->BW)+phi_rad[j],2*pi);
      }
    }
  }
  


  free(alpha);
  free(w_Hz);
  free(phi_rad);
}

// time varying convolution 
void tv_conv(double complex **h, double complex *x, double complex *y, u16 nb_samples, u8 nb_taps){
  
  int i,j;
  for(i=0;i<nb_samples;i++)
    {
      for(j=0;j<nb_taps;j++)
	{
	  if(i>j)
	    y[i] += creal(h[i][j])*creal(x[i-j])-cimag(h[i][j])*cimag(x[i-j]) + I*(creal(h[i][j])*cimag(x[i-j])+cimag(h[i][j])*creal(x[i-j]));
	  
	}
    }
}

double frand_a_b(double a, double b){
  return (rand()/(double)RAND_MAX)*(b-a)+a;
}


