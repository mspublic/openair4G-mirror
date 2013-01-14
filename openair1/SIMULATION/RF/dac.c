#define DEBUG_DAC 1
#include <math.h>
#include "PHY/TOOLS/defs.h"

void dac(double **s_re,
	 double **s_im,
	 u32 **input,
	 u32 input_offset,
	 u32 nb_tx_antennas,
	 u32 length,
	 double amp_dBm,
	 u8 B,
	 u32 meas_length,
	 u32 meas_offset) {

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
		      u32 **input,
		      u32 input_offset,
		      u32 nb_tx_antennas,
		      u32 length,
		      u32 input_offset_meas,
		      u32 length_meas,
		      u8 B,
		      double txpwr_dBm) {

  int i;
  int aa;
  double amp,amp1;
 
  amp1 = sqrt((double)signal_energy((s32*)&input[0][input_offset_meas],length_meas));//(1.0/sqrt(2.0)) * AMP*sqrt(300.0/512); 
  //  printf("amp1 %f dB (%d,%d)\n",20*log10(amp1),input_offset,input_offset_meas);
  amp = pow(10.0,.05*txpwr_dBm);
  amp = amp/sqrt(nb_tx_antennas);

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      s_re[aa][i] = amp*((double)(((short *)input[aa]))[((i+input_offset)<<1)])/amp1; ///(1<<(B-1));
      s_im[aa][i] = amp*((double)(((short *)input[aa]))[((i+input_offset)<<1)+1])/amp1; ///(1<<(B-1));

    }
  }

  //  printf("ener %e\n",signal_energy_fp(s_re,s_im,nb_tx_antennas,length,0));

  return(signal_energy_fp(s_re,s_im,nb_tx_antennas,512,0));
}
