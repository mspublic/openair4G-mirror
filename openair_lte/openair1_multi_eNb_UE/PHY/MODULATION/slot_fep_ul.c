#include "PHY/defs.h"
#include "defs.h"
//#define DEBUG_FEP

int slot_fep_ul(LTE_DL_FRAME_PARMS *frame_parms,
		LTE_eNB_COMMON *eNb_common_vars,
		unsigned char l,
		unsigned char Ns,
		unsigned char eNb_id,
		int no_prefix) {
 
  unsigned char aa;
  unsigned char symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within sub-frame
  unsigned int nb_prefix_samples = (no_prefix ? 0 : frame_parms->nb_prefix_samples);
  unsigned int offset = (frame_parms->ofdm_symbol_size + nb_prefix_samples) * frame_parms->symbols_per_tti * (Ns>>1);

  if (l<0 || l>=7-frame_parms->Ncp) {
    msg("slot_fep: l must be between 0 and %d\n",7-frame_parms->Ncp);
    return(-1);
  }
  if (Ns<0 || Ns>=20) {
    msg("slot_fep: Ns must be between 0 and 19\n");
    return(-1);
  }

#ifdef DEBUG_FEP
  msg("slot_fep: offset %d, symbol %d, nb_prefix_samples %d\n",offset, symbol, nb_prefix_samples);
#endif
  
  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
    fft((short *)&eNb_common_vars->rxdata[eNb_id][aa][nb_prefix_samples + (frame_parms->ofdm_symbol_size+nb_prefix_samples)*symbol+offset],
	(short*)&eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
	frame_parms->twiddle_fft,
	frame_parms->rev,
	frame_parms->log2_symbol_size,
	frame_parms->log2_symbol_size>>1,
	0);
  }

  if ((l==(6-frame_parms->Ncp)) && (Ns%2==1)) {
    for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
#ifdef DEBUG_FEP
      msg("Channel estimation eNb %d, aarx %d\n",eNb_id,aa);
#endif

      mult_cpx_vector_norep((short*) &eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
			    (short*) eNb_common_vars->srs,
			    (short*) eNb_common_vars->srs_ch_estimates[eNb_id][aa],
			    frame_parms->ofdm_symbol_size,
			    15);
    }
  }

#ifdef DEBUG_FEP
  msg("slot_fep: done\n");
#endif
  return(0);
}
