//#include <string.h>
#include "defs.h"
#include "PHY/extern.h"

int init_frame_parms(LTE_DL_FRAME_PARMS *frame_parms) {

  if (frame_parms->Ncp==1) {
    frame_parms->nb_prefix_samples = 512;
    frame_parms->symbols_per_tti = 12;
  }
  else {
    frame_parms->nb_prefix_samples = 256;
    frame_parms->symbols_per_tti = 14;
  }
  
  switch (frame_parms->N_RB_DL) {
  case 100:
    frame_parms->ofdm_symbol_size = 2048;
    frame_parms->log2_symbol_size = 11;
    frame_parms->samples_per_tti = 30720;
    frame_parms->first_carrier_offset = 1448;
    break;
  case 50:
    frame_parms->ofdm_symbol_size = 1024;
    frame_parms->log2_symbol_size = 10;
    frame_parms->samples_per_tti = 15360;
    frame_parms->first_carrier_offset = 724; 
    frame_parms->nb_prefix_samples>>=1;
   break;
  case 25:
    frame_parms->ofdm_symbol_size = 512;
    frame_parms->log2_symbol_size = 9;
    frame_parms->samples_per_tti = 7680;
    frame_parms->first_carrier_offset = 362;
    frame_parms->nb_prefix_samples>>=2;
    break;
  case 15:
    frame_parms->ofdm_symbol_size = 256;
    frame_parms->log2_symbol_size = 8;
    frame_parms->samples_per_tti = 3840;
    frame_parms->first_carrier_offset = 166;
    frame_parms->nb_prefix_samples>>=3;
    break;
  case 6:
    frame_parms->ofdm_symbol_size = 128;
    frame_parms->log2_symbol_size = 7;
    frame_parms->samples_per_tti = 1920;
    frame_parms->first_carrier_offset = 92;
    frame_parms->nb_prefix_samples>>=4;
    break;

  default:
    msg("init_frame_parms: Error: Number of resource blocks (N_RB_DL %d) undefined, frame_parms = %p \n",frame_parms->N_RB_DL, frame_parms);
    return(-1);
    break;
  }

  return(0);
}


void copy_lte_parms_to_phy_framing(LTE_DL_FRAME_PARMS *frame_parms, PHY_FRAMING *phy_framing) {

  //phy_framing->fc_khz;
  //phy_framing->fs_khz;
  phy_framing->Nsymb = frame_parms->symbols_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME;
  phy_framing->Nd = frame_parms->ofdm_symbol_size;     
  phy_framing->log2Nd = frame_parms->log2_symbol_size;
  phy_framing->Nc = frame_parms->nb_prefix_samples;    
  phy_framing->Nz = frame_parms->ofdm_symbol_size - frame_parms->N_RB_DL*12;    
  phy_framing->Nf = frame_parms->N_RB_DL;    
  phy_framing->Extension_type = CYCLIC_PREFIX;
} 



int phy_init_lte_ue(LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_UE_COMMON *lte_ue_common_vars,
		    LTE_UE_DLSCH *lte_ue_dlsch_vars,
		    LTE_UE_PBCH *lte_ue_pbch_vars) {

  int i,j;

  lte_ue_common_vars->rxdata = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
  if (lte_ue_common_vars->rxdata) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->rxdata allocated at %p\n",
	   lte_ue_common_vars->rxdata);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->rxdata not allocated\n");
    return(-1);
  }

  for (i=0; i<frame_parms->nb_antennas_rx; i++) {
    lte_ue_common_vars->rxdata[i] = PHY_vars->rx_vars[i].RX_DMA_BUFFER;
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->rxdata[%d] = %p\n",i,lte_ue_common_vars->rxdata[i]);
#endif
  }

  lte_ue_common_vars->rxdataF = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
  if (lte_ue_common_vars->rxdataF) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->rxdataF allocated at %p\n",
	   lte_ue_common_vars->rxdataF);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->rxdataF not allocated\n");
    return(-1);
  }

  for (i=0; i<frame_parms->nb_antennas_rx; i++) {
    //RK 2 times because of output format of FFT!  We should get rid of this
    lte_ue_common_vars->rxdataF[i] = (int *)malloc16(2*sizeof(int)*(frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti));
    if (lte_ue_common_vars->rxdataF[i]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->rxdataF[%d] allocated at %p\n",i,
	     lte_ue_common_vars->rxdataF[i]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->rxdataF[%d] not allocated\n",i);
      return(-1);
    }
  }
  
  lte_ue_common_vars->dl_ch_estimates = (int **)malloc16(frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx*sizeof(int*));
  if (lte_ue_common_vars->dl_ch_estimates) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates allocated at %p\n",
	   lte_ue_common_vars->dl_ch_estimates);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates not allocated\n");
    return(-1);
  }


  for (i=0; i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx; i++) {
    lte_ue_common_vars->dl_ch_estimates[i] = (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*(frame_parms->ofdm_symbol_size));
    if (lte_ue_common_vars->dl_ch_estimates[i]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates[%d] allocated at %p\n",i,
	     lte_ue_common_vars->dl_ch_estimates[i]);
#endif

      memset(lte_ue_common_vars->dl_ch_estimates[i],0,frame_parms->symbols_per_tti*sizeof(int)*(frame_parms->ofdm_symbol_size));
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates[%d] not allocated\n",i);
      return(-1);
    }
  }

  // DLSCH
  lte_ue_dlsch_vars->rxdataF_ext    = (int **)malloc16(2*sizeof(int*));
  for (i=0;i<frame_parms->nb_antennas_rx;i++)
    lte_ue_dlsch_vars->rxdataF_ext[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));

  lte_ue_dlsch_vars->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
  for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
    lte_ue_dlsch_vars->rxdataF_comp[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));

  lte_ue_dlsch_vars->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
  for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
    lte_ue_dlsch_vars->dl_ch_estimates_ext[i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

  lte_ue_dlsch_vars->dl_ch_mag = (int **)malloc16(4*sizeof(short*));
  for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
      lte_ue_dlsch_vars->dl_ch_mag[i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

  lte_ue_dlsch_vars->dl_ch_magb = (int **)malloc16(4*sizeof(short*));
  for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
    lte_ue_dlsch_vars->dl_ch_magb[i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

  lte_ue_dlsch_vars->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
  lte_ue_dlsch_vars->llr[1] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));


  // PBCH
  lte_ue_pbch_vars->rxdataF_ext    = (int **)malloc16(2*sizeof(int*));
  for (i=0;i<frame_parms->nb_antennas_rx;i++)
    lte_ue_pbch_vars->rxdataF_ext[i] = (int *)malloc16(sizeof(int)*(6*12*4));

  lte_ue_pbch_vars->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
  for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
    lte_ue_pbch_vars->rxdataF_comp[i] = (int *)malloc16(sizeof(int)*(6*12*4));

  lte_ue_pbch_vars->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
  for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
    lte_ue_pbch_vars->dl_ch_estimates_ext[i] = (int *)malloc16(sizeof(int)*6*12*4);
 
  lte_ue_pbch_vars->llr = (short *)malloc16(384*sizeof(short));

  lte_ue_pbch_vars->channel_output = (short *)malloc16((3*64+12)*sizeof(short));

  lte_ue_pbch_vars->decoded_output = (unsigned char *)malloc16(64*sizeof(unsigned char));

  // Initialize Gold sequence table
  lte_gold(frame_parms);

  // Initialize Sync
  lte_sync_time_init(frame_parms,  lte_ue_common_vars);



  return(1);
}
