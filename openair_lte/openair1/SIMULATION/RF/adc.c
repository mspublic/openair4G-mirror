void adc(double **r_re,
	 double **r_im,
	 unsigned int **output,
	 unsigned int nb_rx_antennas,
	 unsigned int length,
	 unsigned char B) {

  int i;
  int aa;

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_rx_antennas;aa++) {
      ((short *)output[aa])[(i<<1)]   = (short)(r_re[aa][i]*(double)(1<<(B-1)));
      ((short *)output[aa])[1+(i<<1)] = (short)(r_im[aa][i]*(double)(1<<(B-1)));
    }
  }
}
