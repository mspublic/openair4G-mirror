#include "PHY/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
//#define DEBUG_FEP

#define SOFFSET 0

int slot_fep(PHY_VARS_UE *phy_vars_ue,
	     unsigned char l,
	     unsigned char Ns,
	     int sample_offset,
	     int no_prefix) {

  LTE_DL_FRAME_PARMS *frame_parms = phy_vars_ue->lte_frame_parms[0];
  LTE_UE_BUFFER *ue_buffer_vars   = phy_vars_ue->lte_ue_buffer_vars;
  unsigned char aa;
  unsigned char symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within sub-frame
  unsigned int nb_prefix_samples = (no_prefix ? 0 : frame_parms->nb_prefix_samples);
  unsigned int nb_prefix_samples0 = (no_prefix ? 0 : frame_parms->nb_prefix_samples0);
  unsigned int subframe_offset,subframe_offset_F;
  unsigned int slot_offset;
  int i;
  unsigned int frame_length_samples = frame_parms->samples_per_tti * 10;
  u8 eNB_id;

  if (no_prefix) {
    subframe_offset = frame_parms->ofdm_symbol_size * frame_parms->symbols_per_tti * (Ns>>1);
    slot_offset = frame_parms->ofdm_symbol_size * (frame_parms->symbols_per_tti>>1) * (Ns%2);
  }
  else {
    subframe_offset = frame_parms->samples_per_tti * (Ns>>1);
    slot_offset = (frame_parms->samples_per_tti>>1) * (Ns%2);
  }
  subframe_offset_F = frame_parms->ofdm_symbol_size * frame_parms->symbols_per_tti * (Ns>>1);


  if (l<0 || l>=7-frame_parms->Ncp) {
    msg("slot_fep: l must be between 0 and %d\n",7-frame_parms->Ncp);
    return(-1);
  }
  if (Ns<0 || Ns>=20) {
    msg("slot_fep: Ns must be between 0 and 19\n");
    return(-1);
  }

#ifdef DEBUG_FEP
    msg("slot_fep: frame %d: slot %d, symbol %d, nb_prefix_samples %d, nb_prefix_samples0 %d, slot_offset %d, subframe_offset %d, sample_offset %d\n", phy_vars_ue->frame,Ns, symbol, nb_prefix_samples,nb_prefix_samples0,slot_offset,subframe_offset,sample_offset);
#endif
  

  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
    memset(&ue_buffer_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],0,2*frame_parms->ofdm_symbol_size*sizeof(int));

    if (l==0) {
      if ((sample_offset +
	  slot_offset +
	  nb_prefix_samples0 + 
	  subframe_offset -
	   SOFFSET) > (frame_length_samples - frame_parms->ofdm_symbol_size))
	memcpy((short *)&ue_buffer_vars->rxdata[aa][frame_length_samples],
	       (short *)&ue_buffer_vars->rxdata[aa][0],
	       frame_parms->ofdm_symbol_size*sizeof(int));

      fft((short *)&ue_buffer_vars->rxdata[aa][(sample_offset +
                                                slot_offset +
                                                nb_prefix_samples0 + 
                                                subframe_offset -
                                                SOFFSET) % frame_length_samples],
          (short*)&ue_buffer_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
          frame_parms->twiddle_fft,
          frame_parms->rev,
          frame_parms->log2_symbol_size,
          frame_parms->log2_symbol_size>>1,
          0);
    }
    else {
      if ((sample_offset +
	   slot_offset +
	   (frame_parms->ofdm_symbol_size+nb_prefix_samples0+nb_prefix_samples) + 
	   (frame_parms->ofdm_symbol_size+nb_prefix_samples)*(l-1) +
	   subframe_offset-
	   SOFFSET) > (frame_length_samples - frame_parms->ofdm_symbol_size))
	memcpy((short *)&ue_buffer_vars->rxdata[aa][frame_length_samples],
	       (short *)&ue_buffer_vars->rxdata[aa][0],
	       frame_parms->ofdm_symbol_size*sizeof(int));
 
      fft((short *)&ue_buffer_vars->rxdata[aa][(sample_offset +
					       slot_offset +
					       (frame_parms->ofdm_symbol_size+nb_prefix_samples0+nb_prefix_samples) + 
					       (frame_parms->ofdm_symbol_size+nb_prefix_samples)*(l-1) +
					       subframe_offset-
						SOFFSET) % frame_length_samples],
	  (short*)&ue_buffer_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }
  }

  for(eNB_id = 0; eNB_id < phy_vars_ue->n_connected_eNB; eNB_id++) {
    frame_parms = phy_vars_ue->lte_frame_parms[eNB_id];
    if((eNB_id == 0) || (phy_vars_ue->UE_mode[eNB_id] != NOT_SYNCHED)) {
      if ((l==0) || (l==(4-frame_parms->Ncp))) {
        for (aa=0;aa<frame_parms->nb_antennas_tx_eNB;aa++) {
#ifndef PERFECT_CE
#ifdef DEBUG_FEP
          msg("Channel estimation eNB %d, aatx %d, slot %d, symbol %d\n",eNB_id,aa,Ns,l);
#endif

          lte_dl_channel_estimation(phy_vars_ue,eNB_id,0,
                                    Ns,
                                    aa,
                                    l,
                                    symbol);

          for (i=0;i<phy_vars_ue->PHY_measurements.n_adj_cells[eNB_id];i++) {
            lte_dl_channel_estimation(phy_vars_ue,eNB_id,i+1,
                                      Ns,
                                      aa,
                                      l,
                                      symbol);
          }

#endif

          // do frequency offset estimation here!
          // use channel estimates from current symbol (=ch_t) and last symbol (ch_{t-1}) 
          lte_est_freq_offset(phy_vars_ue->lte_ue_common_vars[eNB_id]->dl_ch_estimates[0],
              frame_parms, l, &phy_vars_ue->lte_ue_common_vars[eNB_id]->freq_offset);
#ifdef DEBUG_FEP
          msg("[UE %d]Frequency offset estimation for eNB %d: %d\n", phy_vars_ue->Mod_id, eNB_id, 
              phy_vars_ue->lte_ue_common_vars[eNB_id]->freq_offset);
#endif   
        } // aa
      } // l
    } // UE_mode
  } // eNB_id
#ifdef DEBUG_FEP
  msg("slot_fep: done\n");
#endif
  return(0);
}

