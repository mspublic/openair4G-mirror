#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "PHY/TOOLS/defs.h"
#include "defs.h"


/*!
\brief This routine generates a random channel response (time domain) according to a tapped delay line model. 
\param amps Linear amplitudes of the taps (length(amps)=channel_length). The taps are assumed to be spaced equidistantly between 0 and t_max. The values should sum up to 1.
\param t_max Maximum path delay in mus.
\param a state vector of length channel_length
\param channel_length Number of taps.
\param bw Channel bandwidth in MHz.
\param ch Returned channel (length(ch)=(int)11+2*bw*t_max).
\param ricean_factor Ricean factor applied to all taps.
\param phase Phase of the first tap.
\param forgetting_factor (0..1) controls temporal variation of the channel (block stationary)
\param clear Intialize channel memory to 0 
*/

void random_channel(double *amps,
		    double t_max, 
		    struct complex *a,
		    int channel_length,
		    double bw,
		    struct complex *ch,
		    double ricean_factor,
		    struct complex *phase,
		    double forgetting_factor,
		    unsigned char clear) {

  //amps = amps/sum(amps);

  double delta_tau,delay,s;
  int i,k,l;
  struct complex anew;

  //a = sqrt(amps/2) .* (randn(1,length(amps)) + j*randn(1,length(amps)));
  for (i=0;i<channel_length;i++) {

    anew.r = sqrt(ricean_factor*amps[i]/2) * gaussdouble(0.0,1.0);
    anew.i = sqrt(ricean_factor*amps[i]/2) * gaussdouble(0.0,1.0);

    if (i==0) {
      anew.r += phase->r * sqrt(1-ricean_factor);
      anew.i += phase->i * sqrt(1-ricean_factor);
    }

    if (clear==1){
      a[i].r = anew.r;
      a[i].i = anew.i;
    }
    else {
      a[i].r = (sqrt(forgetting_factor)*a[i].r) + sqrt(1-forgetting_factor)*anew.r;
      a[i].i = (sqrt(forgetting_factor)*a[i].i) + sqrt(1-forgetting_factor)*anew.i;
    }
  }



  
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
      //printf("delay %f (%f,%f) : %f [%f=%f,%f,%f]\n",delay,a[l].r,a[l].i,s,(double)k - (delay*bw*2)-(bw*t_max),(double)k,delay*bw*2,bw*t_max);
    }
    //printf("tap  %d : (%f,%f)\n",k,ch[k].r,ch[k].i);
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
