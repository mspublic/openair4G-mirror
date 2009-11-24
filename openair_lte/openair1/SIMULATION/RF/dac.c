void dac(double **s_re,
	 double **s_im,
	 unsigned int **input,
	 unsigned int nb_tx_antennas,
	 unsigned int length,
	 double amp_dBm,
	 unsigned char B) {

  int i;
  int aa;
  double V=0.0,amp;

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      s_re[aa][i] = ((double)(((short *)input[aa]))[(i<<1)])/(1<<(B-1));
      s_im[aa][i] = ((double)(((short *)input[aa]))[(i<<1)+1])/(1<<(B-1));
      V= V + (s_re[aa][i]*s_re[aa][i]) + (s_im[aa][i]*s_im[aa][i]); 
    }
  }
  V /= (aa*length);
  printf("DAC: 10*log10(V) %f (%f)\n",10*log10(V),V);
  amp = pow(10.0,.05*amp_dBm)/V;
  amp = sqrt(amp);

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_tx_antennas;aa++) {
      s_re[aa][i] *= amp;
      s_im[aa][i] *= amp;
    }
  }
}
