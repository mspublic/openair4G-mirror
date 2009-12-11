/* 
   file: lte_est_freq_offset.c
   author (c): florian.kaltenberger@eurecom.fr
   date: 19.11.2009
*/

#include "PHY/defs.h"

int lte_est_freq_offset(int **dl_ch_estimates,
			LTE_DL_FRAME_PARMS *frame_parms,
			int l,
			int* freq_offset) {

  int ch_offset, omega; 
  struct complex16 *omega_cpx; 
  double phase_offset;
  int freq_offset_est;
  int i;
  unsigned char aa;
  short *dl_ch,*dl_ch_prev;
  static int first_run = 1;
  short coef = 1<<10;
  short ncoef =  32767 - coef;


  ch_offset = (l*(frame_parms->ofdm_symbol_size));
 
  if ((l!=0) && (l!=(4-frame_parms->Ncp))) {
    msg("lte_est_freq_offset: l (%d) must be 0 or %d\n",l,4-frame_parms->Ncp);
    return(-1);
  }

  phase_offset = 0.0;

  //for (aa=0;aa<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;aa++) {
  for (aa=0;aa<1;aa++) {

    dl_ch = (short *)&dl_ch_estimates[aa][12+ch_offset];
    if (ch_offset == 0)
      dl_ch_prev = (short *)&dl_ch_estimates[aa][12+(4-frame_parms->Ncp)*(frame_parms->ofdm_symbol_size)];
    else
      dl_ch_prev = (short *)&dl_ch_estimates[aa][12+0];

    // calculate omega = angle(conj(dl_ch)*dl_ch_prev))
    omega = dot_product(dl_ch,dl_ch_prev,(frame_parms->N_RB_DL/2 - 1)*12,15);
    //omega = dot_product(dl_ch,dl_ch_prev,frame_parms->ofdm_symbol_size,15);
    omega_cpx = (struct complex16*) &omega;
    
    dl_ch = (short *)&dl_ch_estimates[aa][(frame_parms->N_RB_DL/2 + 1)*12 + ch_offset];
    if (ch_offset == 0)
      dl_ch_prev = (short *)&dl_ch_estimates[aa][(frame_parms->N_RB_DL/2 + 1)*12+(4-frame_parms->Ncp)*(frame_parms->ofdm_symbol_size)];
    else
      dl_ch_prev = (short *)&dl_ch_estimates[aa][(frame_parms->N_RB_DL/2 + 1)*12];

    // calculate omega = angle(conj(dl_ch)*dl_ch_prev))
    omega = dot_product(dl_ch,dl_ch_prev,(frame_parms->N_RB_DL/2 - 1)*12,15);
    omega_cpx->r += ((struct complex16*) &omega)->r;
    omega_cpx->i += ((struct complex16*) &omega)->i;

    phase_offset = atan2((double)omega_cpx->i,(double)omega_cpx->r);
  }
  //phase_offset /= (frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx);

  freq_offset_est = (int) (phase_offset/(2*M_PI)/2.5e-4); //2.5e-4 is the time between pilot symbols

  // update freq_offset with phase_offset using a moving average filter
  if (first_run == 1) {
    *freq_offset = freq_offset_est;
    first_run = 0;
  }
  else
    *freq_offset = ((freq_offset_est * coef) + (*freq_offset * ncoef)) >> 15;

#ifdef DEBUG_PHY
    msg("l=%d, phase_offset = %f (%d,%d), freq_offset_est = %d Hz, freq_offset_filt = %d \n",l,phase_offset,omega_cpx->r,omega_cpx->i,freq_offset_est,*freq_offset);
#endif

  return(0);
}
