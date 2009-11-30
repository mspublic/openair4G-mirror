/* 
   file: lte_est_freq_offset.c
   author (c): florian.kaltenberger@eurecom.fr
   date: 19.11.2009
*/

#include <math.h>
#include <stdlib.h>
#include "PHY/defs.h"
extern double atan2(double, double);

int lte_est_freq_offset(int **dl_ch_estimates,
			LTE_DL_FRAME_PARMS *frame_parms,
			int l,
			int* freq_offset) {

  int ch_offset, omega; 
  struct complex16 *omega_cpx; 
  double phase_offset;
  int i;
  unsigned char aa;
  short *dl_ch,*dl_ch_prev;

  ch_offset = (l*(frame_parms->ofdm_symbol_size));
 
  if ((l!=0) && (l!=(4-frame_parms->Ncp))) {
    msg("lte_est_freq_offset: l must be 0 or %d\n",4-frame_parms->Ncp);
    return(-1);
  }

  phase_offset = 0.0;
  msg("phase_offset = %f\n",phase_offset);

  //for (aa=0;aa<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;aa++) {
  for (aa=0;aa<1;aa++) {

    dl_ch = (short *)&dl_ch_estimates[aa][ch_offset];
    if (ch_offset == 0)
      dl_ch_prev = (short *)&dl_ch_estimates[aa][(4-frame_parms->Ncp)*(frame_parms->ofdm_symbol_size)];
    else
      dl_ch_prev = (short *)&dl_ch_estimates[aa][0];

    // calculate omega = angle(conj(dl_ch)*dl_ch_prev))
    omega = dot_product(dl_ch,dl_ch_prev,frame_parms->ofdm_symbol_size,15);
    omega_cpx = (struct complex16*) &omega;
    
    msg("phase_offset = %f\n",phase_offset);
    
    phase_offset += atan2((double)omega_cpx->i,(double)omega_cpx->r);
  }
  phase_offset /= frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;

  msg("phase_offset = %f (%d,%d)\n",phase_offset,omega_cpx->r,omega_cpx->i);

  // update freq_offset with phase_offset using a moving average filter

  return(0);
}
