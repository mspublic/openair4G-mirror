/* file: lte_sync_time.c
   purpose: coarse timing synchronization for LTE (using PSS)
   author: florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 22.10.2009 
*/

//#include <string.h>
#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

int* sync_corr = NULL;
short syncF_tmp[2048*2] __attribute__((aligned(16)));
//short sync1F_tmp[256*2] __attribute__((aligned(16)));
//short sync2F_tmp[256*2] __attribute__((aligned(16)));

int lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms, LTE_UE_COMMON *common_vars ) {

  int i,k;
  //unsigned short ds = frame_parms->log2_symbol_size - 7;
  
  sync_corr = (int *)malloc16(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int)*frame_parms->samples_per_tti);
  if (sync_corr) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][SYNC] sync_corr allocated at %p\n", sync_corr);
#endif
    common_vars->sync_corr = sync_corr;
  }
  else {
    msg("[openair][LTE_PHY][SYNC] sync_corr not allocated\n");
    return(-1);
  }

  primary_synch0_time = (int *)malloc16(2*frame_parms->ofdm_symbol_size*sizeof(int));
  if (primary_synch0_time) {
    bzero(primary_synch0_time,frame_parms->ofdm_symbol_size*sizeof(int));
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][SYNC] primary_synch0_time allocated at %p\n", primary_synch0_time);
#endif
  }
  else {
    msg("[openair][LTE_PHY][SYNC] primary_synch0_time not allocated\n");
    return(-1);
  }

  primary_synch1_time = (int *)malloc16(2*frame_parms->ofdm_symbol_size*sizeof(int));
  if (primary_synch1_time) {
    bzero(primary_synch1_time,frame_parms->ofdm_symbol_size*sizeof(int));
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][SYNC] primary_synch1_time allocated at %p\n", primary_synch1_time);
#endif
  }
  else {
    msg("[openair][LTE_PHY][SYNC] primary_synch1_time not allocated\n");
    return(-1);
  }

  primary_synch2_time = (int *)malloc16(2*frame_parms->ofdm_symbol_size*sizeof(int));
  if (primary_synch2_time) {
    bzero(primary_synch2_time,frame_parms->ofdm_symbol_size*sizeof(int));
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][SYNC] primary_synch2_time allocated at %p\n", primary_synch2_time);
#endif
  }
  else {
    msg("[openair][LTE_PHY][SYNC] primary_synch2_time not allocated\n");
    return(-1);
  }

  // generate oversampled sync_time sequences
  k=frame_parms->ofdm_symbol_size-36;
  for (i=0; i<72; i++) {
    syncF_tmp[2*k] = primary_synch0[2*i]>>2;  //we need to shift input to avoid overflow in fft
    syncF_tmp[2*k+1] = primary_synch0[2*i+1]>>2;
    k++;
    if (k >= frame_parms->ofdm_symbol_size) {
      k++;  // skip DC carrier
      k-=frame_parms->ofdm_symbol_size;
    }
  }

  fft((short*)syncF_tmp,          /// complex input
      (short*)primary_synch0_time,          /// complex output
      frame_parms->twiddle_ifft,    /// complex twiddle factors
      frame_parms->rev,             /// bit reversed permutation vector
      frame_parms->log2_symbol_size,/// log2(FFT_SIZE)
      frame_parms->log2_symbol_size/2,
      0);                            /// 0 - input is in complex Q1.15 format, 1 - input is in complex redundant Q1.15 format)
    
  k=frame_parms->ofdm_symbol_size-36;
  for (i=0; i<72; i++) {
    syncF_tmp[2*k] = primary_synch1[2*i]>>2;  //we need to shift input to avoid overflow in fft
    syncF_tmp[2*k+1] = primary_synch1[2*i+1]>>2;
    k++;
    if (k >= frame_parms->ofdm_symbol_size) {
      k++;  // skip DC carrier
      k-=frame_parms->ofdm_symbol_size;
    }
  }

  fft((short*)syncF_tmp,          /// complex input
      (short*)primary_synch1_time,          /// complex output
      frame_parms->twiddle_ifft,    /// complex twiddle factors
      frame_parms->rev,             /// bit reversed permutation vector
      frame_parms->log2_symbol_size,/// log2(FFT_SIZE)
      frame_parms->log2_symbol_size/2,
      0) ;                           /// 0 - input is in complex Q1.15 format, 1 - input is in complex redundant Q1.15 format)

  k=frame_parms->ofdm_symbol_size-36;
  for (i=0; i<72; i++) {
    syncF_tmp[2*k] = primary_synch2[2*i]>>2;  //we need to shift input to avoid overflow in fft
    syncF_tmp[2*k+1] = primary_synch2[2*i+1]>>2;
    k++;
    if (k >= frame_parms->ofdm_symbol_size) {
      k++;  // skip DC carrier
      k-=frame_parms->ofdm_symbol_size;
    }
  }

  fft((short*)syncF_tmp,          /// complex input
      (short*)primary_synch2_time,          /// complex output
      frame_parms->twiddle_ifft,    /// complex twiddle factors
      frame_parms->rev,             /// bit reversed permutation vector
      frame_parms->log2_symbol_size,/// log2(FFT_SIZE)
      frame_parms->log2_symbol_size/2,
      0);                            /// 0 - input is in complex Q1.15 format, 1 - input is in complex redundant Q1.15 format)

#ifdef USER_MODE
#ifdef DEBUG_PHY
  write_output("primary_syncF0.m","psync0",primary_synch0_time,frame_parms->ofdm_symbol_size*2,2,1);
  write_output("primary_sync0.m","psync0",primary_synch0_time,frame_parms->ofdm_symbol_size*2,2,1);
  write_output("primary_sync1.m","psync1",primary_synch1_time,frame_parms->ofdm_symbol_size*2,2,1);
  write_output("primary_sync2.m","psync2",primary_synch2_time,frame_parms->ofdm_symbol_size*2,2,1);
#endif
#endif

  return (1);
}

void lte_sync_time_free(void) {

#ifdef USER_MODE
  free(sync_corr);
  free(primary_synch0_time);
  free(primary_synch1_time);
  free(primary_synch2_time);
#endif
  sync_corr = NULL;

}

int lte_sync_time(int **rxdata, ///rx data in time domain
		    LTE_DL_FRAME_PARMS *frame_parms) {

  // perform a time domain correlation using the oversampled sync sequence

  unsigned int n, m, ar, peak_pos;
  int temp_re, temp_im, peak_val;

  if (sync_corr == NULL) {
    msg("[SYNC TIME] sync_corr not yet allocated! Exiting.\n");
    return(-1);
  }

  peak_val = 0;
  peak_pos = 0;

  for (n=0; n<LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*(frame_parms->samples_per_tti); n++) {
    sync_corr[n] = 0;
    //if (n%1000==0) printf("n=%d\n",n);
    if (n<LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*(frame_parms->samples_per_tti)-frame_parms->ofdm_symbol_size) {
      for (m=0; m<frame_parms->ofdm_symbol_size; m++) {
	for (ar=0;ar<frame_parms->nb_antennas_rx;ar++) {
	// sync_corr[n] += conj(primary_synch0_time[m])*rxdata[0][n+m];
	//if (n==0)
	//  msg("[SYNC TIME] Ant %d, sync[%d]=%d+%dj, rx[%d]=%d+%dj\n", ar, m, primary_synch0_time[2*m], primary_synch0_time[2*m+1], 
	//      n+m, ((short*)rxdata[ar])[2*(1<<ds)*(n+m)], ((short*)rxdata[ar])[2*(1<<ds)*(n+m)+1]); 
	  temp_re = (short)((int)((short*)primary_synch0_time)[4*m]  *(int)((short*)rxdata[ar])[2*(n+m)]   >> 15) +
	            (short)((int)((short*)primary_synch0_time)[4*m+1]*(int)((short*)rxdata[ar])[2*(n+m)+1] >> 15);
	  temp_im = (short)((int)((short*)primary_synch0_time)[4*m]  *(int)((short*)rxdata[ar])[2*(n+m)+1] >> 15) - 
  	            (short)((int)((short*)primary_synch0_time)[4*m+1]*(int)((short*)rxdata[ar])[2*(n+m)]   >> 15);
	//if (n==0)
	//  msg("[SYNC TIME] temp[%d] = %d+%dj\n",m,temp_re,temp_im);
	((short*)sync_corr)[2*n] += temp_re;
	((short*)sync_corr)[2*n+1] += temp_im;
	}
      }
    }
    // calculate the absolute value of sync_corr[n]
    sync_corr[n] = ((int)((short*)sync_corr)[2*n])*((int)((short*)sync_corr)[2*n])
      +((int)((short*)sync_corr)[2*n+1])*((int)((short*)sync_corr)[2*n+1]);
    if (sync_corr[n]>peak_val) {
      peak_val = sync_corr[n];
      peak_pos = n;
    }
  }

  msg("[SYNC TIME] Peak found at pos %d\n",peak_pos);

#ifdef USER_MODE
  write_output("sync_corr.m","synccorr",sync_corr,LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti,1,2);
#endif

  return(peak_pos);

}

