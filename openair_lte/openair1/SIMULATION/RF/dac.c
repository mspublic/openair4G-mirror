//#define DEBUG_DAC 1
#include <math.h>

void dac(double **s_re,
	 double **s_im,
	 unsigned int **input,
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
      s_re[aa][i] = ((double)(((short *)input[aa]))[(i<<1)])/(1<<(B-1));
      s_im[aa][i] = ((double)(((short *)input[aa]))[(i<<1)+1])/(1<<(B-1));

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
  amp = pow(10.0,.1*amp_dBm)/V;
  amp = sqrt(amp);

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      s_re[aa][i] *= amp;
      s_im[aa][i] *= amp;
    }
  }
}
