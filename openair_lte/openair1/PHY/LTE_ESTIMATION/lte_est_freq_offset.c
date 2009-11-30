/* 
   file: lte_est_freq_offset.c
   author (c): florian.kaltenberger@eurecom.fr
   date: 19.11.2009
*/

#include "defs.h"

int lte_est_freq_offset(int **dl_ch_estimates,
			LTE_DL_FRAME_PARMS *frame_parms,
			int l,
			int* freq_offset) {

  int ch_offset, omega; 
  float phase_offset;
  int i;
  unsigned char aa;
  short *dl_ch,*dl_ch_prev;

  ch_offset     = (l*(frame_parms->ofdm_symbol_size));
 
  if ((l!=0) && (l!=(4-frame_parms->Ncp))) {
    msg("lte_est_freq_offset: l must be 0 or %d\n",4-frame_parms->Ncp);
    return(-1);
  }

  phase_offset = 0;
  for (aa=0;aa<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;aa++) {

    dl_ch = (short *)&dl_ch_estimates[aa][ch_offset];
    if (ch_offset == 0)
      dl_ch_prev = (short *)&dl_ch_estimates[aa][(4-frame_parms->Ncp)*(frame_parms->ofdm_symbol_size)];
    else
      dl_ch_prev = (short *)&dl_ch_estimates[aa][0];

    // calculate omega = angle(conj(dl_ch)*dl_ch_prev))
    omega = dot_product(dl_ch,dl_ch_prev,frame_parms->ofdm_symbol_size,15);

    phase_offset += atan2((float)((short*)&omega)[1],(float)((short*)&omega)[0]);
  }
  phase_offset/=frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;

  msg("phase_offset = %f (%d,%d)\n",phase_offset,((short*)&omega)[0],((short*)&omega)[1]);

  // update freq_offset with phase_offset using a moving average filter

  return(0);
}
