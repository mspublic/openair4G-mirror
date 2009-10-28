/* file: lte_sync_time.c
   purpose: coarse timing synchronization for LTE (using PSS)
   author: florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 22.10.2009 
*/

#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

int* sync_corr = NULL;


int lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned short ds = frame_parms->log2_symbol_size - 7;
  sync_corr = (int *)malloc16(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int)*frame_parms->samples_per_tti>>ds);
    if (sync_corr) {
#ifdef DEBUG_PHY
      printk("[openair][LTE_PHY][SYNC] sync_corr allocated at %p\n",
	     sync_corr);
#endif
    }
    else {
      printk("[openair][LTE_PHY][SYNC] sync_corr not allocated\n");
      return(-1);
    }
    return (1);
}

void lte_sync_time_free(void) {

#ifdef USER_MODE
  //free(sync_corr);
#endif
  sync_corr = NULL;

}

short lte_sync_time(int **rxdata, ///rx data in time domain
		    LTE_DL_FRAME_PARMS *frame_parms) {

  // perform a time domain correlation using the downsampled rx signal
  // we can downsample by a factor of frame_parms->log2_symbol_size - 7;

  unsigned short ds, n, m, ar, peak_pos;
  int temp_re, temp_im, peak_val;

  if (sync_corr == NULL) {
    msg("[SYNC TIME] sync_corr not yet allocated! Exiting.\n");
    return(-1);
  }

  ds = frame_parms->log2_symbol_size - 7;
  msg("[SYNC TIME] ds = %d\n",ds);

  peak_val = 0;
  peak_pos = 0;

  for (n=0; n<LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*(frame_parms->samples_per_tti>>ds); n++) {
    sync_corr[n] = 0;
    if (n<LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*(frame_parms->samples_per_tti>>ds)-128) {
      for (m=0; m<128; m++) {
	for (ar=0;ar<frame_parms->nb_antennas_rx;ar++) {
	// sync_corr[n] += conj(primary_synch0_time[m])*rxdata[0][n+m];
	//if (n==0)
	//  msg("[SYNC TIME] Ant %d, sync[%d]=%d+%dj, rx[%d]=%d+%dj\n", ar, m, primary_synch0_time[2*m], primary_synch0_time[2*m+1], 
	//      n+m, ((short*)rxdata[ar])[2*(1<<ds)*(n+m)], ((short*)rxdata[ar])[2*(1<<ds)*(n+m)+1]); 
	temp_re = (short)((int)primary_synch0_time[2*m]*(int)((short*)rxdata[ar])[2*(1<<ds)*(n+m)] >> 15) +
	  (short)((int)primary_synch0_time[2*m+1]*(int)((short*)rxdata[ar])[2*(1<<ds)*(n+m)+1] >> 15);
	temp_im = (short)((int)primary_synch0_time[2*m]*(int)((short*)rxdata[ar])[2*(1<<ds)*(n+m)+1] >> 15) - 
	  (short)((int)primary_synch0_time[2*m+1]*(int)((short*)rxdata[ar])[2*(1<<ds)*(n+m)] >> 15);
	//if (n==0)
	//  msg("[SYNC TIME] temp[%d] = %d+%dj\n",m,temp_re,temp_im);
	((short*)sync_corr)[2*n] += temp_re;
	((short*)sync_corr)[2*n+1] += temp_im;
	}
      }
    }
    // calculate the absolute value of sync_corr[n]
    sync_corr[n] = (int)((short*)sync_corr)[2*n]*(int)((short*)sync_corr)[2*n]+(int)((short*)sync_corr)[2*n+1]*(int)((short*)sync_corr)[2*n+1];
    if (sync_corr[n]>peak_val) {
      peak_val = sync_corr[n];
      peak_pos = n<<ds;
    }
  }

  msg("[SYNC TIME] Peak found at pos %d\n",peak_pos);

#ifdef USER_MODE
  write_output("sync_corr.m","synccorr",sync_corr,LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti>>ds,1,2);
#endif

  return(peak_pos);

}

