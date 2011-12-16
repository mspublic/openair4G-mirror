#include "PHY/defs.h"
#include "defs.h"
//#define DEBUG_FEP

int slot_fep_ul(LTE_DL_FRAME_PARMS *frame_parms,
		LTE_eNB_COMMON *eNB_common_vars,
		unsigned char l,
		unsigned char Ns,
		unsigned char eNB_id,
		int no_prefix) {
#ifdef DEBUG_FEP
  char fname[40], vname[40];
#endif
  unsigned char aa;
  unsigned char symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within sub-frame
  unsigned int nb_prefix_samples = (no_prefix ? 0 : frame_parms->nb_prefix_samples);
  unsigned int nb_prefix_samples0 = (no_prefix ? 0 : frame_parms->nb_prefix_samples0);
  unsigned int subframe_offset; 
  unsigned int slot_offset;

  if (no_prefix) {
    subframe_offset = frame_parms->ofdm_symbol_size * frame_parms->symbols_per_tti * (Ns>>1);
    slot_offset = frame_parms->ofdm_symbol_size * (frame_parms->symbols_per_tti>>1) * (Ns%2);
  }
  else {
    subframe_offset = frame_parms->samples_per_tti * (Ns>>1);
    slot_offset = (frame_parms->samples_per_tti>>1) * (Ns%2);
  }

  if (l<0 || l>=7-frame_parms->Ncp) {
    msg("slot_fep: l must be between 0 and %d\n",7-frame_parms->Ncp);
    return(-1);
  }
  if (Ns<0 || Ns>=20) {
    msg("slot_fep: Ns must be between 0 and 19\n");
    return(-1);
  }

#ifdef DEBUG_FEP
  msg("slot_fep: offset %d, symbol %d, nb_prefix_samples %d\n",subframe_offset, symbol, nb_prefix_samples);
#endif

  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {

#ifdef DEBUG_FEP
    if ((Ns==5) && (eNB_id==0) && (aa==0)) 
      write_output("eNB_rx.m","rxs",&eNB_common_vars->rxdata[0][0][nb_prefix_samples + (frame_parms->ofdm_symbol_size+nb_prefix_samples)*symbol+subframe_offset],((frame_parms->ofdm_symbol_size+nb_prefix_samples)*6),1,1);
    if ((Ns==7) && (eNB_id==0) && (symbol==11)) 
      write_output("eNB_rx.m","rxs",&eNB_common_vars->rxdata[0][aa][nb_prefix_samples + (frame_parms->ofdm_symbol_size+nb_prefix_samples)*symbol+subframe_offset],(frame_parms->ofdm_symbol_size),1,1);
#endif
    if (l==0) {
	     
      fft((short *)&eNB_common_vars->rxdata[eNB_id][aa][slot_offset +
							nb_prefix_samples0 + 
							subframe_offset],
	  (short*)&eNB_common_vars->rxdataF[eNB_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }
    else {
      fft((short *)&eNB_common_vars->rxdata[eNB_id][aa][slot_offset +
							(frame_parms->ofdm_symbol_size+nb_prefix_samples0+nb_prefix_samples) + 
							(frame_parms->ofdm_symbol_size+nb_prefix_samples)*(l-1) +
							subframe_offset],
	  (short*)&eNB_common_vars->rxdataF[eNB_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }
  }

#ifdef DEBUG_FEP
  msg("slot_fep: done\n");
#endif
  return(0);
}
