#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "PHY/TOOLS/defs.h"
#include "defs.h"

channel_desc_t *new_channel_desc(u8 nb_tx,u8 nb_rx, u8 nb_taps, u8 channel_length, double *amps, double Td, double BW, double ricean_factor, double aoa, double forgetting_factor, double max_Doppler, s32 channel_offset, double path_loss_dB) {

  channel_desc_t *chan_desc = (channel_desc_t *)malloc(sizeof(channel_desc_t));
  u16 i;
 
  chan_desc->nb_tx          = nb_tx;
  chan_desc->nb_rx          = nb_rx;
  chan_desc->nb_taps        = nb_taps;
  chan_desc->channel_length = channel_length;
  chan_desc->amps           = amps;
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
  return(chan_desc);
}

void random_channel(channel_desc_t *desc) {
		    

  //amps = amps/sum(amps);

  double delta_tau,delay,s;
  int i,k,l,aarx,aatx;
  struct complex anew,phase;

  delta_tau = desc->Td/desc->channel_length;

  for (i=0;i<desc->channel_length;i++) {

    for (aarx=0;aarx<desc->nb_rx;aarx++) {
      for (aatx=0;aatx<desc->nb_tx;aatx++) {
	anew.r = sqrt(desc->ricean_factor*desc->amps[i]/2) * gaussdouble(0.0,1.0);
	anew.i = sqrt(desc->ricean_factor*desc->amps[i]/2) * gaussdouble(0.0,1.0);

	// this assumes that both RX and TX have linear antenna arrays with lambda/2 antenna spacing. 
	// Furhter it is assumed that the arrays are parallel to each other and that they are far enough apart so 
	// that we can safely assume plane wave propagation.
	phase.r = cos(M_PI*((aarx-aatx)*sin(desc->aoa)));
	phase.i = sin(M_PI*((aarx-aatx)*sin(desc->aoa)));
	
	if (i==0) {
	  anew.r += phase.r * sqrt(1-desc->ricean_factor);
	  anew.i += phase.i * sqrt(1-desc->ricean_factor);
	}
	
	if (desc->first_run==1){
	  desc->a[aarx+(aatx*desc->nb_rx)][i].r = anew.r;
	  desc->a[aarx+(aatx*desc->nb_rx)][i].i = anew.i;
	}
	else {
	  desc->a[aarx+(aatx*desc->nb_rx)][i].r = (sqrt(desc->forgetting_factor)*desc->a[aarx+(aatx*desc->nb_rx)][i].r) + sqrt(1-desc->forgetting_factor)*anew.r;
	  desc->a[aarx+(aatx*desc->nb_rx)][i].i = (sqrt(desc->forgetting_factor)*desc->a[aarx+(aatx*desc->nb_rx)][i].i) + sqrt(1-desc->forgetting_factor)*anew.i;
	}
      

	memset((void *)desc->ch[aarx+(aatx*desc->nb_rx)],0,(int)(desc->channel_length)*sizeof(struct complex));

	for (k=0;k<(int)desc->channel_length;k++) {
	  desc->ch[aarx+(aatx*desc->nb_rx)][k].r = 0.0;
	  desc->ch[aarx+(aatx*desc->nb_rx)][k].i = 0.0;

	  for (l=0;l<desc->channel_length;l++) {
	    delay = ((double)l+.159)*delta_tau;
	    s = sin(M_PI*(k - (delay*desc->BW)-(desc->BW*desc->Td/2)))/(M_PI*(k-(delay*desc->BW)-(desc->BW*desc->Td/2)));
	    
	    desc->ch[aarx+(aatx*desc->nb_rx)][k].r += s*desc->a[aarx+(aatx*desc->nb_rx)][l].r;
	    desc->ch[aarx+(aatx*desc->nb_rx)][k].i += s*desc->a[aarx+(aatx*desc->nb_rx)][l].i;
	  }
	}
      }
    }
  }
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
