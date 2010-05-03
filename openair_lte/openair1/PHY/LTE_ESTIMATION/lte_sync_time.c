/* file: lte_sync_time.c
   purpose: coarse timing synchronization for LTE (using PSS)
   author: florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 22.10.2009 
*/

//#include <string.h>
#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"


#define DEBUG_PHY

int* sync_corr = NULL;
int sync_tmp[2048*4] __attribute__((aligned(16)));
short syncF_tmp[2048*2] __attribute__((aligned(16)));
//short sync1F_tmp[256*2] __attribute__((aligned(16)));
//short sync2F_tmp[256*2] __attribute__((aligned(16)));

int lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms ) { // LTE_UE_COMMON *common_vars

  int i,k;
  //unsigned short ds = frame_parms->log2_symbol_size - 7;
  
  sync_corr = (int *)malloc16(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int)*frame_parms->samples_per_tti);
  if (sync_corr) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][SYNC] sync_corr allocated at %p\n", sync_corr);
#endif
    //common_vars->sync_corr = sync_corr;
  }
  else {
    msg("[openair][LTE_PHY][SYNC] sync_corr not allocated\n");
    return(-1);
  }

  primary_synch0_time = (int *)malloc16((frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)*sizeof(int));
  if (primary_synch0_time) {
    bzero(primary_synch0_time,(frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)*sizeof(int));
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][SYNC] primary_synch0_time allocated at %p\n", primary_synch0_time);
#endif
  }
  else {
    msg("[openair][LTE_PHY][SYNC] primary_synch0_time not allocated\n");
    return(-1);
  }

  primary_synch1_time = (int *)malloc16((frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)*sizeof(int));
  if (primary_synch1_time) {
    bzero(primary_synch1_time,(frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)*sizeof(int));
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][SYNC] primary_synch1_time allocated at %p\n", primary_synch1_time);
#endif
  }
  else {
    msg("[openair][LTE_PHY][SYNC] primary_synch1_time not allocated\n");
    return(-1);
  }

  primary_synch2_time = (int *)malloc16((frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)*sizeof(int));
  if (primary_synch2_time) {
    bzero(primary_synch2_time,(frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)*sizeof(int));
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
      (short*)sync_tmp,          /// complex output
      frame_parms->twiddle_ifft,    /// complex twiddle factors
      frame_parms->rev,             /// bit reversed permutation vector
      frame_parms->log2_symbol_size,/// log2(FFT_SIZE)
      frame_parms->log2_symbol_size/2,
      0);                            /// 0 - input is in complex Q1.15 format, 1 - input is in complex redundant Q1.15 format)


  for (i=0; i<frame_parms->ofdm_symbol_size; i++)
    primary_synch0_time[i+frame_parms->nb_prefix_samples] = sync_tmp[2*i];
  // this is the CP
  for (i=0; i<frame_parms->nb_prefix_samples; i++)
    primary_synch0_time[i] = sync_tmp[2*(frame_parms->ofdm_symbol_size-frame_parms->nb_prefix_samples+i)];

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
      (short*)sync_tmp,          /// complex output
      frame_parms->twiddle_ifft,    /// complex twiddle factors
      frame_parms->rev,             /// bit reversed permutation vector
      frame_parms->log2_symbol_size,/// log2(FFT_SIZE)
      frame_parms->log2_symbol_size/2,
      0) ;                           /// 0 - input is in complex Q1.15 format, 1 - input is in complex redundant Q1.15 format)

  for (i=0; i<frame_parms->ofdm_symbol_size; i++)
    primary_synch1_time[i+frame_parms->nb_prefix_samples] = sync_tmp[2*i];
  // this is the CP
  for (i=0; i<frame_parms->nb_prefix_samples; i++)
    primary_synch1_time[i] = sync_tmp[2*(frame_parms->ofdm_symbol_size-frame_parms->nb_prefix_samples+i)];

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
      (short*)sync_tmp,          /// complex output
      frame_parms->twiddle_ifft,    /// complex twiddle factors
      frame_parms->rev,             /// bit reversed permutation vector
      frame_parms->log2_symbol_size,/// log2(FFT_SIZE)
      frame_parms->log2_symbol_size/2,
      0);                            /// 0 - input is in complex Q1.15 format, 1 - input is in complex redundant Q1.15 format)

  for (i=0; i<frame_parms->ofdm_symbol_size; i++)
    primary_synch2_time[i+frame_parms->nb_prefix_samples] = sync_tmp[2*i];
  // this is the CP
  for (i=0; i<frame_parms->nb_prefix_samples; i++)
    primary_synch2_time[i] = sync_tmp[2*(frame_parms->ofdm_symbol_size-frame_parms->nb_prefix_samples+i)];

#ifdef USER_MODE
#ifdef DEBUG_PHY
  write_output("primary_sync0.m","psync0",primary_synch0_time,frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples,1,1);
  write_output("primary_sync1.m","psync1",primary_synch1_time,frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples,1,1);
  write_output("primary_sync2.m","psync2",primary_synch2_time,frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples,1,1);
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

inline int abs32(int x) { 
  return (((int)((short*)&x)[0])*((int)((short*)&x)[0]) + ((int)((short*)&x)[1])*((int)((short*)&x)[1]));
}

int lte_sync_time(int **rxdata, ///rx data in time domain
		  LTE_DL_FRAME_PARMS *frame_parms,
		  int length,
		  int *eNb_id) {

  // perform a time domain correlation using the oversampled sync sequence

  unsigned int n, m, ar, s, peak_pos, peak_val, sync_source;
  int temp_re, temp_im, result;
  int sync_out[3] = {0,0,0};

  //msg("[SYNC TIME] Calling sync_time.\n");
  if (sync_corr == NULL) {
    msg("[SYNC TIME] sync_corr not yet allocated! Exiting.\n");
    return(-1);
  }

  peak_val = 0;
  peak_pos = 0;
  sync_source = 0;

  for (n=0; n<length; n+=4) {

#ifdef RTAI_ENABLED
    // This is necessary since the sync takes a long time and it seems to block all other threads thus screwing up RTAI. If we pause it for a little while during its execution we give RTAI a chance to catch up with its other tasks.
    if ((n%frame_parms->samples_per_tti == 0) && (n>0) && (openair_daq_vars.sync_state==0)) {
#ifdef DEBUG_PHY
      msg("[SYNC TIME] pausing for 1000ns, n=%d\n",n);
#endif
      rt_sleep(nano2count(1000));
    }
#endif

    sync_corr[n] = 0;
    for (s=0;s<3;s++)
      sync_out[s]=0;

    if (n<(length-frame_parms->ofdm_symbol_size-frame_parms->nb_prefix_samples)) {

      //calculate dot product of primary_synch0_time and rxdata[ar][n] (ar=0..nb_ant_rx) and store the sum in temp[n];
      for (ar=0;ar<frame_parms->nb_antennas_rx;ar++) {
	result = dot_product((short*)primary_synch0_time, (short*) &(rxdata[ar][n]), frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples, 15);
	((short*)sync_out)[0] += ((short*) &result)[0];
	((short*)sync_out)[1] += ((short*) &result)[1];
      }

      for (ar=0;ar<frame_parms->nb_antennas_rx;ar++) {
	result = dot_product((short*)primary_synch1_time, (short*) &(rxdata[ar][n]), frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples, 15);
	((short*)sync_out)[2] += ((short*) &result)[0];
	((short*)sync_out)[3] += ((short*) &result)[1];
      }

      for (ar=0;ar<frame_parms->nb_antennas_rx;ar++) {
	result = dot_product((short*)primary_synch2_time, (short*) &(rxdata[ar][n]), frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples, 15);
	((short*)sync_corr)[2*n] += ((short*) &result)[0];
	((short*)sync_corr)[2*n+1] += ((short*) &result)[1];
	((short*)sync_out)[4] += ((short*) &result)[0];
	((short*)sync_out)[5] += ((short*) &result)[1];
      }

      //      if (n==0)
      //	msg("[SYNC TIME] sync_corr[%d] = %d+%dj\n",n,((short*) sync_corr)[2*n],((short*) sync_corr)[2*n+1]);

      /*
      sync_corr[n] = 0;
      for (m=0; m<frame_parms->ofdm_symbol_size; m++) {
	for (ar=0;ar<frame_parms->nb_antennas_rx;ar++) {
	// sync_corr[n] += conj(primary_synch0_time[m])*rxdata[0][n+m];
	//if (n==0)
	//  msg("[SYNC TIME] Ant %d, sync[%d]=%d+%dj, rx[%d]=%d+%dj\n", ar, m, primary_synch0_time[2*m], primary_synch0_time[2*m+1], 
	//      n+m, ((short*)rxdata[ar])[2*(1<<ds)*(n+m)], ((short*)rxdata[ar])[2*(1<<ds)*(n+m)+1]); 
	  temp_re = (short)(((int)((short*)primary_synch0_time)[2*m] *(int)((short*)rxdata[ar])[2*(n+m)] +
			    (int)((short*)primary_synch0_time)[2*m+1]*(int)((short*)rxdata[ar])[2*(n+m)+1]) >> 15);
	  temp_im = (short)(((int)((short*)primary_synch0_time)[2*m] *(int)((short*)rxdata[ar])[2*(n+m)+1] - 
			    (int)((short*)primary_synch0_time)[2*m+1]*(int)((short*)rxdata[ar])[2*(n+m)]) >> 15);
	((short*)sync_corr)[2*n] += temp_re;
	((short*)sync_corr)[2*n+1] += temp_im;
	}
      }
      */

      //      if (n==0)
      //	msg("[SYNC TIME] sync_corr[%d] = %d+%dj\n",n,((short*) sync_corr)[2*n],((short*) sync_corr)[2*n+1]);

    }
    // calculate the absolute value of sync_corr[n]
    //    sync_corr[n] = ((int)((short*)sync_corr)[2*n])*((int)((short*)sync_corr)[2*n])
    //      +((int)((short*)sync_corr)[2*n+1])*((int)((short*)sync_corr)[2*n+1]);
    sync_corr[n] = abs32(sync_corr[n]);

    for (s=0;s<3;s++) {
      sync_out[s] = abs32(sync_out[s]);
      if (sync_out[s]>peak_val) {
	peak_val = sync_out[s];
	peak_pos = n;
	sync_source = s;
      }
    }
  }

  *eNb_id = sync_source;

#ifdef DEBUG_PHY
  msg("[SYNC TIME] Sync source = %d, Peak found at pos %d, val = %d\n",sync_source,peak_pos,peak_val);

#ifdef USER_MODE
  write_output("sync_corr_ue.m","synccorr",sync_corr,length,1,2);
#endif
#endif

  return(peak_pos);

}

int lte_sync_time_eNb(int **rxdata, ///rx data in time domain
		      LTE_DL_FRAME_PARMS *frame_parms,
		      int eNb_id,
		      int length,
		      int *peak_val) {

  // perform a time domain correlation using the oversampled sync sequence

  unsigned int n, ar, peak_pos, mean_val;
  int result;
  short *primary_synch_time;

  //msg("[SYNC TIME] Calling sync_time_eNb(%p,%p,%d,%d)\n",rxdata,frame_parms,eNb_id,length);
  if (sync_corr == NULL) {
    msg("[SYNC TIME] sync_corr not yet allocated! Exiting.\n");
    return(-1);
  }

  switch (eNb_id) {
  case 0:
    primary_synch_time = (short*)primary_synch0_time;
    break;
  case 1:
    primary_synch_time = (short*)primary_synch1_time;
    break;
  case 2:
    primary_synch_time = (short*)primary_synch2_time;
    break;
  default:
    msg("[SYNC TIME] Illegal eNb_id!\n");
    return (-1);
  }

  *peak_val = 0;
  peak_pos = 0;
  mean_val = 0;

  for (n=0; n<length; n+=4) {

    sync_corr[n] = 0;

    if (n<(length-frame_parms->ofdm_symbol_size-frame_parms->nb_prefix_samples)) {

      //calculate dot product of primary_synch0_time and rxdata[ar][n] (ar=0..nb_ant_rx) and store the sum in temp[n];
      for (ar=0;ar<frame_parms->nb_antennas_rx;ar++)  {
      	result = dot_product((short*)primary_synch_time, (short*) &(rxdata[ar][n]), frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples, 15);
	((short*)sync_corr)[2*n]   += ((short*) &result)[0];
	((short*)sync_corr)[2*n+1] += ((short*) &result)[1];
      }

    }
    // calculate the absolute value of sync_corr[n]
    //    sync_corr[n] = ((int)((short*)sync_corr)[2*n])*((int)((short*)sync_corr)[2*n])
    //      +((int)((short*)sync_corr)[2*n+1])*((int)((short*)sync_corr)[2*n+1]);
    sync_corr[n] = abs32(sync_corr[n]);
    mean_val += sync_corr[n]>>10;

    if (sync_corr[n]>*peak_val) {
      *peak_val = sync_corr[n];
      peak_pos = n;
    }
  }

#ifdef DEBUG_PHY
#ifdef USER_MODE
    if (eNb_id==0)
      write_output("sync_corr_eNb.m","synccorr",sync_corr,length,1,2);
#endif
#endif

  if ((*peak_val>>10 * length) < 50*mean_val) {
#ifdef DEBUG_PHY
     debug_msg("[SYNC TIME] No peak found (%d,%d,%d)\n",peak_pos,*peak_val,mean_val);
#endif
    return(-1);
  }
  else {
#ifdef DEBUG_PHY
    debug_msg("[SYNC TIME] Peak found at pos %d, val = %d, mean_val = %d\n",peak_pos,*peak_val,mean_val);
#endif
    return(peak_pos);
  }

}
