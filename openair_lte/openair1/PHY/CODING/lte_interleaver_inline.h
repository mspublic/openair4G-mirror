extern unsigned short threegpplte_interleaver_output;
extern unsigned int threegpplte_interleaver_tmp;

extern inline void threegpplte_interleaver_reset() {
  threegpplte_interleaver_output = 0;
  threegpplte_interleaver_tmp    = 0;
}

extern inline unsigned short threegpplte_interleaver(unsigned short f1,
					      unsigned short f2,
					      unsigned short K) {

  threegpplte_interleaver_tmp = (threegpplte_interleaver_tmp+(f2<<1));

  threegpplte_interleaver_output = (threegpplte_interleaver_output + threegpplte_interleaver_tmp + f1 - f2)%K;

#ifdef DEBUG_TURBO_ENCODER

  //  printf("Interleaver output %d\n",threegpplte_interleaver_output);
#endif
  return(threegpplte_interleaver_output);
}
