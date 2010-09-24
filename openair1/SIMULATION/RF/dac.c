#define DEBUG_DAC 1
#include <math.h>
#include "PHY/TOOLS/defs.h"

void dac(double **s_re,
	 double **s_im,
	 unsigned int **input,
	 unsigned int input_offset,
	 unsigned int nb_tx_antennas,
	 unsigned int length,
	 double amp_dBm,
	 unsigned char B,
	 unsigned int meas_length,
	 unsigned int meas_offset) {

  int i;
  int aa;
  double V=0.0,amp;

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      s_re[aa][i] = ((double)(((short *)input[aa]))[((i+input_offset)<<1)])/(1<<(B-1));
      s_im[aa][i] = ((double)(((short *)input[aa]))[((i+input_offset)<<1)+1])/(1<<(B-1));

    }
  }

  for (i=meas_offset;i<meas_offset+meas_length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      V= V + (s_re[aa][i]*s_re[aa][i]) + (s_im[aa][i]*s_im[aa][i]); 
    }
  }
  V /= (meas_length);
#ifdef DEBUG_DAC
  printf("DAC: 10*log10(V)=%f (%f)\n",10*log10(V),V);
#endif
  if (V) {
    amp = pow(10.0,.1*amp_dBm)/V;
    amp = sqrt(amp);
  } else {
    amp = 1;
  }

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      s_re[aa][i] *= amp;
      s_im[aa][i] *= amp;
    }
  }
}


double dac_fixed_gain(double **s_re,
		      double **s_im,
		      unsigned int **input,
		      unsigned int input_offset,
		      unsigned int nb_tx_antennas,
		      unsigned int length,
		      unsigned char B,
		      double gain_dB) {

  int i;
  int aa;
  double V=0.0,amp;

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      s_re[aa][i] = ((double)(((short *)input[aa]))[((i+input_offset)<<1)])/(1<<(B-1));
      s_im[aa][i] = ((double)(((short *)input[aa]))[((i+input_offset)<<1)+1])/(1<<(B-1));

    }
    //        if (i<32)
    //          printf("%f,%f\n",s_re[0][i],s_im[0][i]);
  }

  amp = pow(10.0,.1*gain_dB);
  amp = sqrt(amp);


  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      s_re[aa][i] *= amp;
      s_im[aa][i] *= amp;
    }
  }

  //  printf("ener %e\n",signal_energy_fp(s_re,s_im,nb_tx_antennas,length,0));

  return(signal_energy_fp(s_re,s_im,nb_tx_antennas,length,0));
}
