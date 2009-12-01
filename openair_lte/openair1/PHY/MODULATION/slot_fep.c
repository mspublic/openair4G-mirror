#include "PHY/defs.h"
#include "defs.h"
//#define DEBUG_FEP

int slot_fep(LTE_DL_FRAME_PARMS *frame_parms,
	      LTE_UE_COMMON *ue_common_vars,
	      unsigned char l,
	      unsigned char Ns,
	      int offset,
	      int no_prefix) {
 
  unsigned char aa;
  unsigned char symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within frame
  unsigned int nb_prefix_samples = (no_prefix ? 0 : frame_parms->nb_prefix_samples);

#ifdef DEBUG_FEP
  if (l<0 || l>=7-frame_parms->Ncp) {
    msg("slot_fep: l must be between 0 and %d\n",7-frame_parms->Ncp);
    return(-1);
  }
  if (Ns<0 || Ns>=20) {
    msg("slot_fep: Ns must be between 0 and 19\n");
    return(-1);
  }
#endif

#ifdef DEBUG_PHY
    msg("slot_fep: offset %d, symbol %d, nb_prefix_samples %d\n",offset, symbol, nb_prefix_samples);
#endif
  
  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
    fft((short *)&ue_common_vars->rxdata[aa][nb_prefix_samples + (frame_parms->ofdm_symbol_size+nb_prefix_samples)*symbol+offset],
	(short*)&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	frame_parms->twiddle_fft,
	frame_parms->rev,
	frame_parms->log2_symbol_size,
	frame_parms->log2_symbol_size>>1,
	0);
  }

  if ((l==0) || (l==(4-frame_parms->Ncp))) {
    for (aa=0;aa<frame_parms->nb_antennas_tx;aa++)
      lte_dl_channel_estimation(ue_common_vars->dl_ch_estimates,
				ue_common_vars->rxdataF,
				frame_parms,
				Ns,
				aa,
				l,
				symbol);
    
    // do frequency offset estimation here!
    // use channel estimates from current symbol (=ch_t) and last symbol (ch_{t-1}) 
    /*
    lte_est_freq_offset(ue_common_vars->dl_ch_estimates,
			frame_parms,
			aa,
			l,
			ue_common_vars->freq_offset);
    */
  }

  return(0);
}
