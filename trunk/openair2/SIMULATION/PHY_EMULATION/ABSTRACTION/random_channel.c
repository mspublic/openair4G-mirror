#include <math.h>
#include "complex.h"
#include "misc_proto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

     // amps = amplitudes 
     // t_max = maximum path delay
     // channel_length = channel length
     // bw = channel bandwidth in MHz
/*This routine generates a random OFDM channel response (frequency domain) 
@param amps
@param t_max 
@param channel_length 
@param bw 
@param ch 
*/

void random_channel(double *amps,
		    double t_max, 
		    int channel_length,
		    double bw,
		    struct complex *ch,
		    double ricean_factor,
		    struct complex *phase) {

  //amps = amps/sum(amps);
  struct complex a[channel_length];
  double delta_tau,delay,s;
  int i,k,l;
  
  //a = sqrt(amps/2) .* (randn(1,length(amps)) + j*randn(1,length(amps)));
  for (i=0;i<channel_length;i++) {

    a[i].r = sqrt(ricean_factor*amps[i]/2) * gaussdouble(0.0,1.0);
    a[i].i = sqrt(ricean_factor*amps[i]/2) * gaussdouble(0.0,1.0);
  }
  
  a[0].r += phase->r * sqrt(1-ricean_factor);
  a[0].i += phase->i * sqrt(1-ricean_factor);

  //delta_tau=t_max/(channel_length-1);
  
  delta_tau = t_max/channel_length;

  //delays=(0:delta_tau:t_max);
  
  //  ch=zeros(1,Nb_tones);
  memset((void *)ch,0,(int)((11+2*bw*t_max))*sizeof(struct complex));

  //  for k=1:Nb_tones,
  //	 for l=1:channel_length,
  //		ch(k)=ch(k)+a(l).*exp(-j*2*pi*delays(l)*k*bw*10^6/Nb_tones);
  //  end
  //   end

  for (k=0;k<(int)(11+2*bw*t_max);k++) {
    ch[k].r = 0.0;
    ch[k].i = 0.0;

    for (l=0;l<channel_length;l++) {
      
 
      delay = ((double)l+.159)*delta_tau;
      s = sin(M_PI*(k - (delay*bw)-(bw*t_max/2)))/(M_PI*(k-(delay*bw)-(bw*t_max/2)));
 
      ch[k].r += s*a[l].r;
      ch[k].i += s*a[l].i;
      //      printf("delay %f (%f,%f) : %f [%f=%f,%f,%f]\n",delay,a[l].r,a[l].i,s,(double)k - (delay*bw)-(bw*t_max),(double)k,delay*bw,bw*t_max);
    }
    //    printf("tap  %d : (%f,%f)\n",k,ch[k].r,ch[k].i);
  }
}


#ifdef RANDOM_CHANNEL_MAIN
#define BW 5.0
#define Td 2.0
main(int argc,char **argv) {

  double amps[4] = {1.0,.2,.1,.05};
  struct complex ch[(int)(1+2*BW*Td)];
  int i;
  
  randominit();
  random_channel(amps,Td, 4,BW,ch);
  for (i=0;i<(1+2*BW*Td);i++){
    printf("(%f,%f)\n",ch[i].r,ch[i].i);
  }

}

#endif
