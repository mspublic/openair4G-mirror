#include <math.h>
#include <cblas.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "PHY/TOOLS/defs.h"
#include "defs.h"

//#define DEBUG_CH

channel_desc_t *new_channel_desc(u8 nb_tx,
				 u8 nb_rx, 
				 u8 nb_taps, 
				 u8 channel_length, 
				 double *amps, 
				 double *delays, 
				 struct complex** R_sqrt, 
				 double Td, 
				 double BW, 
				 double ricean_factor, 
				 double aoa, 
				 double forgetting_factor, 
				 double max_Doppler, 
				 s32 channel_offset, 
				 double path_loss_dB) {

  channel_desc_t *chan_desc = (channel_desc_t *)malloc(sizeof(channel_desc_t));
  u16 i,j;
  double delta_tau;

  chan_desc->nb_tx          = nb_tx;
  chan_desc->nb_rx          = nb_rx;
  chan_desc->nb_taps        = nb_taps;
  chan_desc->channel_length = channel_length;
  chan_desc->amps           = amps;
  if (delays==NULL) {
    chan_desc->delays = (double*) malloc(nb_taps*sizeof(double));
    delta_tau = Td/nb_taps;
    for (i=0; i<nb_taps; i++)
      chan_desc->delays[i] = ((double)i+0.159)*delta_tau;
  }
  else
    chan_desc->delays         = delays;
  chan_desc->Td             = Td;
  chan_desc->BW             = BW;
  chan_desc->ricean_factor  = ricean_factor;
  chan_desc->aoa            = aoa;
  chan_desc->forgetting_factor = forgetting_factor;
  chan_desc->channel_offset = channel_offset;
  chan_desc->path_loss_dB   = path_loss_dB;
  chan_desc->first_run      = 1;
  chan_desc->ip             = 0.0;
  chan_desc->ch             = (struct complex**) malloc(nb_tx*nb_rx*sizeof(struct complex*));
  chan_desc->a              = (struct complex**) malloc(nb_taps*sizeof(struct complex*));
  for (i = 0; i<nb_tx*nb_rx; i++) 
    chan_desc->ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex)); 
  for (i = 0; i<nb_taps; i++) {
    chan_desc->a[i]         = (struct complex*) malloc(nb_tx*nb_rx * sizeof(struct complex));
  }
  if (R_sqrt == NULL) {
    chan_desc->R_sqrt         = (struct complex**) malloc(nb_taps*sizeof(struct complex*));
    for (i = 0; i<nb_taps; i++) {
      chan_desc->R_sqrt[i]    = (struct complex*) malloc(nb_tx*nb_rx*nb_tx*nb_rx * sizeof(struct complex));
      for (j = 0; j<nb_tx*nb_rx*nb_tx*nb_rx; j+=(nb_tx*nb_rx+1)) {
	chan_desc->R_sqrt[i][j].r = 1.0;
	chan_desc->R_sqrt[i][j].i = 0.0;
      }
    }
  }
  else {
    chan_desc->R_sqrt = R_sqrt;
  }

  printf("[CHANNEL] RF %f\n",chan_desc->ricean_factor);
  for (i=0;i<chan_desc->nb_taps;i++)
    printf("[CHANNEL] tap %d: amp %f, delay %f\n",i,chan_desc->amps[i],chan_desc->delays[i]);

  return(chan_desc);
}

double scm_c_delays[] = {0, 0.0125, 0.0250, 0.3625, 0.3750, 0.3875, 0.2500, 0.2625, 0.2750, 1.0375, 1.0500, 1.0625, 2.7250, 2.7375, 2.7500, 4.6000, 4.6125, 4.6250};
double scm_c_amps_dB[] = {0.00, -2.22, -3.98, -1.86, -4.08, -5.84, -1.08, -3.30, -5.06, -9.08, -11.30, -13.06, -15.14, -17.36, -19.12, -20.64, -22.85, -24.62};

channel_desc_t *new_channel_desc_scm(u8 nb_tx, 
				     u8 nb_rx, 
				     SCM_t channel_model, 
				     double BW, 
				     double forgetting_factor, 
				     s32 channel_offset, 
				     double path_loss_dB) {

  channel_desc_t *chan_desc = (channel_desc_t *)malloc(sizeof(channel_desc_t));
  u16 i,j;
  double sum_amps;

  chan_desc->nb_tx          = nb_tx;
  chan_desc->nb_rx          = nb_rx;
  chan_desc->BW             = BW;
  chan_desc->forgetting_factor = forgetting_factor;
  chan_desc->channel_offset = channel_offset;
  chan_desc->path_loss_dB   = path_loss_dB;
  chan_desc->first_run      = 1;
  chan_desc->ip             = 0.0;

  switch (channel_model) {
  case SCM_A:
    msg("channel model not yet supported\n");
    free(chan_desc);
    return(NULL);
  case SCM_B:
    msg("channel model not yet supported\n");
    free(chan_desc);
    return(NULL);
  case SCM_C:
    chan_desc->nb_taps        = 18;
    chan_desc->Td             = 4.625;
    chan_desc->channel_length = (int) (2*chan_desc->BW*chan_desc->Td + 1 + 2/(M_PI*M_PI)*log(4*M_PI*chan_desc->BW*chan_desc->Td));
    sum_amps = 0;
    chan_desc->amps           = (double*) malloc(chan_desc->nb_taps*sizeof(double));
    for (i = 0; i<chan_desc->nb_taps; i++) {
      chan_desc->amps[i]      = pow(10,.1*scm_c_amps_dB[i]); 
      sum_amps += chan_desc->amps[i];
    }
    for (i = 0; i<chan_desc->nb_taps; i++)
      chan_desc->amps[i] /= sum_amps;
    chan_desc->delays         = scm_c_delays;
    chan_desc->ricean_factor  = 1;
    chan_desc->aoa            = 0;
    chan_desc->ch             = (struct complex**) malloc(nb_tx*nb_rx*sizeof(struct complex*));
    chan_desc->a              = (struct complex**) malloc(chan_desc->nb_taps*sizeof(struct complex*));
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->ch[i] = (struct complex*) malloc(chan_desc->channel_length * sizeof(struct complex)); 
    for (i = 0; i<chan_desc->nb_taps; i++) {
      chan_desc->a[i]         = (struct complex*) malloc(nb_tx*nb_rx * sizeof(struct complex));
    }
    chan_desc->R_sqrt         = (struct complex**) malloc(chan_desc->nb_taps*sizeof(struct complex*));
    for (i = 0; i<chan_desc->nb_taps; i++) {
      chan_desc->R_sqrt[i]    = (struct complex*) malloc(nb_tx*nb_rx*nb_tx*nb_rx * sizeof(struct complex));
      for (j = 0; j<nb_tx*nb_rx*nb_tx*nb_rx; j+=(nb_tx*nb_rx+1)) {
	chan_desc->R_sqrt[i][j].r = 1.0;
	chan_desc->R_sqrt[i][j].i = 0.0;
      }
    }
    break;
  case SCM_D:
    msg("channel model not yet supported\n");
    free(chan_desc);
    return(NULL);
  default:
    msg("channel model not yet supported\n");
    free(chan_desc);
    return(NULL);
  }
  printf("[CHANNEL] RF %f\n",chan_desc->ricean_factor);
  for (i=0;i<chan_desc->nb_taps;i++)
    printf("[CHANNEL] tap %d: amp %f, delay %f\n",i,chan_desc->amps[i],chan_desc->delays[i]);

  return(chan_desc);
}


int random_channel(channel_desc_t *desc) {
		    
  double s;
  int i,k,l,aarx,aatx;
  struct complex anew[NB_ANTENNAS_TX*NB_ANTENNAS_RX],acorr[NB_ANTENNAS_TX*NB_ANTENNAS_RX];
  struct complex phase, alpha, beta;

  if ((desc->nb_tx>NB_ANTENNAS_TX) || (desc->nb_rx > NB_ANTENNAS_RX)) {
    msg("random_channel.c: Error: temporary buffer for channel not big enough\n");
    return(-1);
  }

  for (i=0;i<(int)desc->nb_taps;i++) {
    for (aarx=0;aarx<desc->nb_rx;aarx++) {
      for (aatx=0;aatx<desc->nb_tx;aatx++) {

	anew[aarx+(aatx*desc->nb_rx)].r = sqrt(desc->ricean_factor*desc->amps[i]/2) * gaussdouble(0.0,1.0);
	anew[aarx+(aatx*desc->nb_rx)].i = sqrt(desc->ricean_factor*desc->amps[i]/2) * gaussdouble(0.0,1.0);

	if (i==0) {
	  // this assumes that both RX and TX have linear antenna arrays with lambda/2 antenna spacing. 
	  // Furhter it is assumed that the arrays are parallel to each other and that they are far enough apart so 
	  // that we can safely assume plane wave propagation.
	  phase.r = cos(M_PI*((aarx-aatx)*sin(desc->aoa)));
	  phase.i = sin(M_PI*((aarx-aatx)*sin(desc->aoa)));
	  
	  anew[aarx+(aatx*desc->nb_rx)].r += phase.r * sqrt(1-desc->ricean_factor);
	  anew[aarx+(aatx*desc->nb_rx)].i += phase.i * sqrt(1-desc->ricean_factor);
	}
#ifdef DEBUG_CH
	printf("(%d,%d,%d) %f->(%f,%f) (%f,%f) phase (%f,%f)\n",aarx,aatx,i,desc->amps[i],anew[aarx+(aatx*desc->nb_rx)].r,anew[aarx+(aatx*desc->nb_rx)].i,desc->aoa,desc->ricean_factor,phase.r,phase.i);
#endif	
      } //aatx
    } //aarx

    //apply correlation matrix
    //compute acorr = R_sqrt[i] * anew
    alpha.r = 1.0;
    alpha.i = 0.0;
    beta.r = 0.0;
    beta.i = 0.0;
    cblas_zgemv(CblasRowMajor, CblasNoTrans, desc->nb_tx*desc->nb_rx, desc->nb_tx*desc->nb_rx, 
		(void*) &alpha, (void*) desc->R_sqrt[i], desc->nb_rx*desc->nb_tx,
		(void*) anew, 1, (void*) &beta, (void*) acorr, 1);

	
    if (desc->first_run==1){
      cblas_zcopy(desc->nb_tx*desc->nb_rx, (void*) acorr, 1, (void*) desc->a[i], 1);
    }
    else {
      alpha.r = sqrt(1-desc->forgetting_factor);
      alpha.i = 0;
      beta.r = sqrt(desc->forgetting_factor);
      beta.i = 0;
      cblas_zscal(desc->nb_tx*desc->nb_rx, (void*) &beta,  (void*) desc->a[i], 1);
      cblas_zaxpy(desc->nb_tx*desc->nb_rx, (void*) &alpha, (void*) acorr, 1, (void*) desc->a[i], 1);

      //  desc->a[i][aarx+(aatx*desc->nb_rx)].r = (sqrt(desc->forgetting_factor)*desc->a[i][aarx+(aatx*desc->nb_rx)].r) + sqrt(1-desc->forgetting_factor)*anew.r;
      //  desc->a[i][aarx+(aatx*desc->nb_rx)].i = (sqrt(desc->forgetting_factor)*desc->a[i][aarx+(aatx*desc->nb_rx)].i) + sqrt(1-desc->forgetting_factor)*anew.i;
    }
  } //nb_taps      

  //memset((void *)desc->ch[aarx+(aatx*desc->nb_rx)],0,(int)(desc->channel_length)*sizeof(struct complex));

  for (aarx=0;aarx<desc->nb_rx;aarx++) {
    for (aatx=0;aatx<desc->nb_tx;aatx++) {
      for (k=0;k<(int)desc->channel_length;k++) {
	desc->ch[aarx+(aatx*desc->nb_rx)][k].r = 0.0;
	desc->ch[aarx+(aatx*desc->nb_rx)][k].i = 0.0;
	
	for (l=0;l<desc->nb_taps;l++) {
	  s = sin(M_PI*(k - (desc->delays[l]*desc->BW)-(desc->BW*desc->Td/2)))/(M_PI*(k-(desc->delays[l]*desc->BW)-(desc->BW*desc->Td/2)));
	  
	  desc->ch[aarx+(aatx*desc->nb_rx)][k].r += s*desc->a[l][aarx+(aatx*desc->nb_rx)].r;
	  desc->ch[aarx+(aatx*desc->nb_rx)][k].i += s*desc->a[l][aarx+(aatx*desc->nb_rx)].i;
	} //nb_taps
#ifdef DEBUG_CH
	printf("(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,desc->ch[aarx+(aatx*desc->nb_rx)][k].r,desc->ch[aarx+(aatx*desc->nb_rx)][k].i);
#endif
      } //channel_length
    } //aatx
  } //aarx

  return (0);
}

#ifdef RANDOM_CHANNEL_MAIN
#define BW 5.0
#define Td 2.0
main(int argc,char **argv) {

  double amps[8] = {.8,.2,.1,.04,.02,.01,.005};
  struct complex ch[(int)(1+2*BW*Td)],phase;
  int i;
  
  randominit();
  phase.r = 1.0;
  phase.i = 0;
  random_channel(amps,Td, 8,BW,ch,(double)1.0,&phase);
  /*
  for (i=0;i<(11+2*BW*Td);i++){
    printf("%f + sqrt(-1)*%f\n",ch[i].r,ch[i].i);
  }
  */
}

#endif
