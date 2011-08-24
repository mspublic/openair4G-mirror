#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"
#include <emmintrin.h>

#define max(a,b) (a<b?b:a)


int phy_channel_interpolation(int *channel_est_f,
			      int *channel_est_f_interp,
			      int antenna_tx){
  // interpolate the channel
  // the even carriers contain the channel estimate from the 1st tx antenna
  // transfer the frequency response of the channel of each tx antenna to the time domain, 
  // zero padd to NUMBER_OF_OFDM_CARRIERS and retranstransform to the frequency domain, scaling appropriately

  int channel_est_f_tmp[2*512] __attribute__((aligned(16)));
  int channel_est_t_tmp[2*1024] __attribute__((aligned(16)));
  int i;

  if (NB_ANTENNAS_TX == 2 && NUMBER_OF_OFDM_CARRIERS<=1024 ) {

    Zero_Buffer(channel_est_f_tmp,4*NUMBER_OF_OFDM_CARRIERS);
    Zero_Buffer(channel_est_t_tmp,8*NUMBER_OF_OFDM_CARRIERS);
    
    for (i=0;i<NUMBER_OF_OFDM_CARRIERS/NB_ANTENNAS_TX;i++) {
      channel_est_f_tmp[2*i] = channel_est_f[2*(NB_ANTENNAS_TX*i+antenna_tx)];
      channel_est_f_tmp[2*i+1] = channel_est_f[2*(NB_ANTENNAS_TX*i+antenna_tx)+1];
    }
      
/*     write_output("channel_est_f_tmp.m","h_f", */
/* 		 (s16 *)channel_est_f_tmp, */
/* 		 NUMBER_OF_OFDM_CARRIERS, */
/* 		 2, */
/* 		 1); */

    fft((short*)channel_est_f_tmp,
	(short*)channel_est_t_tmp,
	(short *)twiddle_ifft_half,
	rev_half,
	LOG2_NUMBER_OF_OFDM_CARRIERS-1,
	4,
	1);
    
/*     write_output("channel_est_t_tmp.m","h_t", */
/* 		 (s16 *)channel_est_t_tmp, */
/* 		 NUMBER_OF_OFDM_CARRIERS, */
/* 		 2, */
/* 		 1); */
    
    fft((short*)channel_est_t_tmp,
	(short*)channel_est_f_interp,
	(short *)twiddle_fft,
	rev,
	LOG2_NUMBER_OF_OFDM_CARRIERS,
	0,
	1);
    
    return 0;
  }
  else {
    msg("[openair][PHY][CHANEST] Cannot calculate MMSE filter for NB_ANTENNAS_TX != 2 or NUMBER_OF_OFDM_CARRIERS>1024 \n");
    return -1;
  }
}


