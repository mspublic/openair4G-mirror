#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "PHY/TOOLS/defs.h"
#include "defs.h"

//#define DEBUG_CH

channel_desc_t *new_channel_desc(u8 nb_tx,u8 nb_rx, u8 nb_taps, u8 channel_length, double *amps, double *delays, double Td, double BW, double ricean_factor, double aoa, double forgetting_factor, double max_Doppler, s32 channel_offset, double path_loss_dB) {

  channel_desc_t *chan_desc = (channel_desc_t *)malloc(sizeof(channel_desc_t));
  u16 i;
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
  chan_desc->channel_offset = channel_offset;
  chan_desc->path_loss_dB   = path_loss_dB;
  chan_desc->first_run      = 1;
  chan_desc->ip             = 0.0;
  chan_desc->ch = (struct complex**) malloc(nb_tx*nb_rx*sizeof(struct complex*));
  chan_desc->a = (struct complex**) malloc(nb_tx*nb_rx*sizeof(struct complex*));
  for (i = 0; i<chan_desc->nb_tx*chan_desc->nb_rx; i++) {
    chan_desc->ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex)); 
    chan_desc->a[i]  = (struct complex*) malloc(nb_taps * sizeof(struct complex));
  }
  chan_desc->R_tx_sqrt = (struct complex **) malloc(chan_desc->nb_tx*chan_desc->nb_tx*sizeof(struct complex *));
  for (i=0; i<chan_desc->nb_tx*chan_desc->nb_tx; i++) {
    chan_desc->R_tx_sqrt[i] = (struct complex *) malloc(chan_desc->channel_length*sizeof(struct complex));
  }
  chan_desc->R_rx_sqrt = (struct complex **) malloc(chan_desc->nb_rx*chan_desc->nb_rx*sizeof(struct complex *));
  for (i=0; i<chan_desc->nb_rx*chan_desc->nb_rx; i++) {
    chan_desc->R_rx_sqrt[i] = (struct complex *) malloc(chan_desc->channel_length*sizeof(struct complex));
  }

  printf("[CHANNEL] RF %f\n",chan_desc->ricean_factor);
  for (i=0;i<chan_desc->nb_taps;i++)
    printf("[CHANNEL] tap %d: amp %f, delay %f\n",i,chan_desc->amps[i],chan_desc->delays[i]);

  return(chan_desc);
}

void random_channel(channel_desc_t *desc) {
		    
  double s;
  int i,k,l,aarx,aatx;
  struct complex anew,phase;

  for (aarx=0;aarx<desc->nb_rx;aarx++) {
    for (aatx=0;aatx<desc->nb_tx;aatx++) {
      for (i=0;i<(int)desc->nb_taps;i++) {

	anew.r = sqrt(desc->ricean_factor*desc->amps[i]/2) * gaussdouble(0.0,1.0);
	anew.i = sqrt(desc->ricean_factor*desc->amps[i]/2) * gaussdouble(0.0,1.0);

	if (i==0) {
	  // this assumes that both RX and TX have linear antenna arrays with lambda/2 antenna spacing. 
	  // Furhter it is assumed that the arrays are parallel to each other and that they are far enough apart so 
	  // that we can safely assume plane wave propagation.
	  phase.r = cos(M_PI*((aarx-aatx)*sin(desc->aoa)));
	  phase.i = sin(M_PI*((aarx-aatx)*sin(desc->aoa)));
	  
	  anew.r += phase.r * sqrt(1-desc->ricean_factor);
	  anew.i += phase.i * sqrt(1-desc->ricean_factor);
	}
#ifdef DEBUG_CH
	printf("(%d,%d,%d) %f->(%f,%f) (%f,%f) phase (%f,%f)\n",aarx,aatx,i,desc->amps[i],anew.r,anew.i,desc->aoa,desc->ricean_factor,phase.r,phase.i);
#endif	
	
	if (desc->first_run==1){
	  desc->a[aarx+(aatx*desc->nb_rx)][i].r = anew.r;
	  desc->a[aarx+(aatx*desc->nb_rx)][i].i = anew.i;
	}
	else {
	  desc->a[aarx+(aatx*desc->nb_rx)][i].r = (sqrt(desc->forgetting_factor)*desc->a[aarx+(aatx*desc->nb_rx)][i].r) + sqrt(1-desc->forgetting_factor)*anew.r;
	  desc->a[aarx+(aatx*desc->nb_rx)][i].i = (sqrt(desc->forgetting_factor)*desc->a[aarx+(aatx*desc->nb_rx)][i].i) + sqrt(1-desc->forgetting_factor)*anew.i;
	}
      } //nb_taps      

      //memset((void *)desc->ch[aarx+(aatx*desc->nb_rx)],0,(int)(desc->channel_length)*sizeof(struct complex));

      for (k=0;k<(int)desc->channel_length;k++) {
	desc->ch[aarx+(aatx*desc->nb_rx)][k].r = 0.0;
	desc->ch[aarx+(aatx*desc->nb_rx)][k].i = 0.0;

	for (l=0;l<desc->nb_taps;l++) {
	  s = sin(M_PI*(k - (desc->delays[l]*desc->BW)-(desc->BW*desc->Td/2)))/(M_PI*(k-(desc->delays[l]*desc->BW)-(desc->BW*desc->Td/2)));
	  
	  desc->ch[aarx+(aatx*desc->nb_rx)][k].r += s*desc->a[aarx+(aatx*desc->nb_rx)][l].r;
	  desc->ch[aarx+(aatx*desc->nb_rx)][k].i += s*desc->a[aarx+(aatx*desc->nb_rx)][l].i;
	} //nb_taps
#ifdef DEBUG_CH
	printf("(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,desc->ch[aarx+(aatx*desc->nb_rx)][k].r,desc->ch[aarx+(aatx*desc->nb_rx)][k].i);
#endif
      } //channel_length
    } //aatx
  } //aarx
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
