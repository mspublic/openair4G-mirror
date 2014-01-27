#include "PHY/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
//#define DEBUG_FEP

int slot_fep(LTE_DL_FRAME_PARMS *frame_parms,
	     LTE_UE_COMMON *ue_common_vars,
	     unsigned char l,
	     unsigned char Ns,
	     int sample_offset,
	     int no_prefix) {
 
  unsigned char aa;
  unsigned char eNb_id;
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
  debug_msg("slot_fep: slot %d, symbol %d, nb_prefix_samples %d, nb_prefix_samples0 %d, slot_offset %d, subframe_offset %d, sample_offset %d\n", Ns, symbol, nb_prefix_samples,nb_prefix_samples0,slot_offset,subframe_offset,sample_offset);
#endif
  
  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
    if (l==0) {
      fft((short *)&ue_common_vars->rxdata[aa][sample_offset +
					       slot_offset +
					       nb_prefix_samples0 + 
					       subframe_offset],
	  (short*)&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }
    else {

      fft((short *)&ue_common_vars->rxdata[aa][sample_offset +
					       slot_offset +
					       (frame_parms->ofdm_symbol_size+nb_prefix_samples0+nb_prefix_samples) + 
					       (frame_parms->ofdm_symbol_size+nb_prefix_samples)*(l-1) +
					       subframe_offset],
	  (short*)&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }

    memcpy(&ue_common_vars->rxdataF2[aa][2*subframe_offset+2*frame_parms->ofdm_symbol_size*symbol],
	   &ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	   2*frame_parms->ofdm_symbol_size*sizeof(int));

  }
    

  if ((l==0) || (l==(4-frame_parms->Ncp))) {
    for (aa=0;aa<frame_parms->nb_antennas_tx;aa++)
      for (eNb_id=0;eNb_id<1;eNb_id++){ //3;eNb_id++){
#ifdef DEBUG_FEP
	debug_msg("Channel estimation eNb %d, aatx %d, slot %d, symbol %d\n",eNb_id,aa,Ns,l);
#endif
	lte_dl_channel_estimation(ue_common_vars->dl_ch_estimates[eNb_id],
				  ue_common_vars->rxdataF,
				  eNb_id,
				  frame_parms,
				  Ns,
				  aa,
				  l,
				  symbol);
      }
    // do frequency offset estimation here!
    // use channel estimates from current symbol (=ch_t) and last symbol (ch_{t-1}) 
#ifdef DEBUG_FEP
    msg("Frequency offset estimation\n");
#endif   
    if ((Ns == 0) & (l==(4-frame_parms->Ncp))) 
      lte_est_freq_offset(ue_common_vars->dl_ch_estimates[0],
			  frame_parms,
			  l,
			  &ue_common_vars->freq_offset);
  }

#ifdef DEBUG_FEP
  msg("slot_fep: done\n");
#endif
  return(0);
}