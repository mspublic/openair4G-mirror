void adc(double **r_re,
	 double **r_im,
	 unsigned int input_offset,
	 unsigned int output_offset,
	 unsigned int **output,
	 unsigned int nb_rx_antennas,
	 unsigned int length,
	 unsigned char B) {

  int i;
  int aa;
  double gain = (double)(1<<(B-1));

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_rx_antennas;aa++) {
      ((short *)output[aa])[((i+output_offset)<<1)]   = (short)(r_re[aa][i+input_offset]*gain);
      ((short *)output[aa])[1+((i+output_offset)<<1)] = (short)(r_im[aa][i+input_offset]*gain);
      if ((r_re[aa][i+input_offset]*gain) > 30000) {
	//("Adc outputs %d %e  %d \n",i,((short *)output[0])[((i+output_offset)<<1)], ((i+output_offset)<<1) );
      }
    }
    //printf("Adc outputs %d %e  %d \n",i,((short *)output[0])[((i+output_offset)<<1)], ((i+output_offset)<<1) );
  }
}
