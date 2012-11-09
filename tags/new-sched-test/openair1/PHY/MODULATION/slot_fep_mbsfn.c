#include "PHY/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#define DEBUG_FEP

#define SOFFSET 0

int slot_fep_mbsfn(PHY_VARS_UE *phy_vars_ue,
	     unsigned char l,
	     int subframe,
	     int sample_offset,
	     int no_prefix) {

  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_ue->lte_frame_parms;
  LTE_UE_COMMON *ue_common_vars   = &phy_vars_ue->lte_ue_common_vars;
  u8 eNB_id = 0;//ue_common_vars->eNb_id;
  
  unsigned char aa;
  unsigned char frame_type = 0; // Frame Type: 0 - FDD, 1 - TDD;
  unsigned int nb_prefix_samples = (no_prefix ? 0 : frame_parms->nb_prefix_samples);
  unsigned int nb_prefix_samples0 = (no_prefix ? 0 : frame_parms->nb_prefix_samples0);
  unsigned int subframe_offset,subframe_offset_F;
 
  int i;
    
  if (no_prefix) {
    subframe_offset = frame_parms->ofdm_symbol_size * frame_parms->symbols_per_tti * subframe;

  }
  else {
    subframe_offset = frame_parms->samples_per_tti * subframe;

  }
  subframe_offset_F = frame_parms->ofdm_symbol_size * frame_parms->symbols_per_tti * subframe;


  if (l<0 || l>=12) {
    msg("slot_fep_mbsfn: l must be between 0 and 11\n");
    return(-1);
  }
  
  if ((subframe == 0) || (subframe == 5) ||    // SFn 0,4,5,9;
      (subframe == 4) || (subframe == 9))	  {   //check for valid MBSFN subframe
    msg("slot_fep_mbsfn: Subframe must be 1,2,3,6,7,8 for FDD (Frame type=0) \n");  //and 3,4,7,8,9 for TDD(Frame type=1)
    return(-1);
  }

#ifdef DEBUG_FEP
  msg("slot_fep_mbsfn: subframe %d, symbol %d, nb_prefix_samples %d, nb_prefix_samples0 %d, subframe_offset %d, sample_offset %d\n", subframe, l, nb_prefix_samples,nb_prefix_samples0,subframe_offset,sample_offset);
#endif
  

  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
    memset(&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*l],0,2*frame_parms->ofdm_symbol_size*sizeof(int));

    if (l==0) {
      fft((short *)&ue_common_vars->rxdata[aa][sample_offset +
					       nb_prefix_samples0 + 
					       subframe_offset -
					       SOFFSET],
	  (short*)&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*l],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }
    else {

      fft((short *)&ue_common_vars->rxdata[aa][sample_offset +
					       (frame_parms->ofdm_symbol_size+nb_prefix_samples0+nb_prefix_samples) + 
					       (frame_parms->ofdm_symbol_size+nb_prefix_samples)*(l-1) +
					       subframe_offset-
					       SOFFSET],
	  (short*)&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*l],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }

    memcpy(&ue_common_vars->rxdataF2[aa][2*subframe_offset_F+2*frame_parms->ofdm_symbol_size*l],
	   &ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*l],
	   2*frame_parms->ofdm_symbol_size*sizeof(int));

  }
  //if ((l==0) || (l==(4-frame_parms->Ncp))) {
// changed to invoke MBSFN channel estimation in symbols 2,6,10    
    if ((l==2)||(l==6)||(l==10)) {
    for (aa=0;aa<frame_parms->nb_antennas_tx;aa++) {
#ifndef PERFECT_CE
#ifdef DEBUG_FEP
      msg("Channel estimation eNB %d, aatx %d, subframe %d, symbol %d\n",eNB_id,aa,subframe,l);
#endif

     lte_dl_mbsfn_channel_estimation(phy_vars_ue,
				    eNB_id,
					0,
				    subframe,
				    l);					
	for (i=0;i<phy_vars_ue->PHY_measurements.n_adj_cells;i++) {		
		lte_dl_mbsfn_channel_estimation(phy_vars_ue,
				    eNB_id,
					i+1,
				    subframe,
				    l);		
    /*  lte_dl_channel_estimation(phy_vars_ue,eNB_id,0,
				Ns,
				aa,
				l,
				symbol);
      for (i=0;i<phy_vars_ue->PHY_measurements.n_adj_cells;i++) {
	lte_dl_channel_estimation(phy_vars_ue,eNB_id,i+1,
				  Ns,
				  aa,
				  l,
				  symbol);*/
      }
#endif

      // do frequency offset estimation here!
      // use channel estimates from current symbol (=ch_t) and last symbol (ch_{t-1}) 
#ifdef DEBUG_FEP
      msg("Frequency offset estimation\n");
#endif   
     // if ((l == 0) || (l==(4-frame_parms->Ncp))) 
/*	  if ((l==2)||(l==6)||(l==10)) 
	lte_mbsfn_est_freq_offset(ue_common_vars->dl_ch_estimates[0],
			    frame_parms,
			    l,
			    &ue_common_vars->freq_offset); */
    }
  }
#ifdef DEBUG_FEP
  msg("slot_fep_mbsfn: done\n");
#endif
  return(0);
}
