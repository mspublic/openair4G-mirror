#include "defs.h"

int init_frame_parms(LTE_DL_FRAME_PARMS *frame_parms,u8 osf) {

  u8 log2_osf;

  if (frame_parms->Ncp==1) {
    frame_parms->nb_prefix_samples0=512;
    frame_parms->nb_prefix_samples = 512;
    frame_parms->symbols_per_tti = 12;
  }
  else {
    frame_parms->nb_prefix_samples0 = 160;
    frame_parms->nb_prefix_samples = 144;
    frame_parms->symbols_per_tti = 14;
  }

  switch(osf) {
  case 1:
    log2_osf = 0;
    break;
  case 2:
    log2_osf = 1;
    break;
  case 4:
    log2_osf = 2;
    break;
  case 8:
    log2_osf = 3;
    break;
  case 16:
    log2_osf = 4;
    break;
  default:
    msg("Illegal oversampling %d\n",osf);
    return(-1);
  }

  switch (frame_parms->N_RB_DL) {
  case 100:
    if (osf>1) {
      msg("Illegal oversampling %d for N_RB_DL %d\n",osf,frame_parms->N_RB_DL);
      return(-1);
    }
    frame_parms->ofdm_symbol_size = 2048;
    frame_parms->log2_symbol_size = 11;
    frame_parms->samples_per_tti = 30720;
    frame_parms->first_carrier_offset = 2048-600;
    break;
  case 50:
    if (osf>1) {
      msg("Illegal oversampling %d for N_RB_DL %d\n",osf,frame_parms->N_RB_DL);
      return(-1);
    }
    frame_parms->ofdm_symbol_size = 1024*osf;
    frame_parms->log2_symbol_size = 10+log2_osf;
    frame_parms->samples_per_tti = 15360*osf;
    frame_parms->first_carrier_offset = frame_parms->ofdm_symbol_size - 300; 
    frame_parms->nb_prefix_samples>>=(1-log2_osf);
    frame_parms->nb_prefix_samples0>>=(1-log2_osf);
   break;
  case 25:
    if (osf>2) {
      msg("Illegal oversampling %d for N_RB_DL %d\n",osf,frame_parms->N_RB_DL);
      return(-1);
    }
    frame_parms->ofdm_symbol_size = 512*osf;
    
    frame_parms->log2_symbol_size = 9+log2_osf;
    frame_parms->samples_per_tti = 7680*osf;
    frame_parms->first_carrier_offset = frame_parms->ofdm_symbol_size - 150; 
    frame_parms->nb_prefix_samples>>=(2-log2_osf);
    frame_parms->nb_prefix_samples0>>=(2-log2_osf);
    break;
  case 15:
    frame_parms->ofdm_symbol_size = 256*osf;
    frame_parms->log2_symbol_size = 8+log2_osf;
    frame_parms->samples_per_tti = 3840*osf;
    frame_parms->first_carrier_offset = frame_parms->ofdm_symbol_size - 90;
    frame_parms->nb_prefix_samples>>=(3-log2_osf);
    frame_parms->nb_prefix_samples0>>=(3-log2_osf);
    break;
  case 6:
    frame_parms->ofdm_symbol_size = 128*osf;
    frame_parms->log2_symbol_size = 7+log2_osf;
    frame_parms->samples_per_tti = 1920*osf;
    frame_parms->first_carrier_offset = frame_parms->ofdm_symbol_size - 36;
    frame_parms->nb_prefix_samples>>=(4-log2_osf);
    frame_parms->nb_prefix_samples0>>=(4-log2_osf);
    break;

  default:
    msg("init_frame_parms: Error: Number of resource blocks (N_RB_DL %d) undefined, frame_parms = %p \n",frame_parms->N_RB_DL, frame_parms);
    return(-1);
    break;
  }

  //  frame_parms->tdd_config=3;
  return(0);
}


void dump_frame_parms(LTE_DL_FRAME_PARMS *frame_parms)
{
  msg("frame_parms->N_RB_DL=%d\n",frame_parms->N_RB_DL);
  msg("frame_parms->N_RB_UL=%d\n",frame_parms->N_RB_UL);
  msg("frame_parms->Nid_cell=%d\n",frame_parms->Nid_cell);
  msg("frame_parms->Ncp=%d\n",frame_parms->Ncp);
  msg("frame_parms->Ncp_UL=%d\n",frame_parms->Ncp_UL);
  msg("frame_parms->nushift=%d\n",frame_parms->nushift);
  msg("frame_parms->frame_type=%d\n",frame_parms->frame_type);
  msg("frame_parms->tdd_config=%d\n",frame_parms->tdd_config);
  msg("frame_parms->tdd_config_S=%d\n",frame_parms->tdd_config_S);
  msg("frame_parms->freq_idx=%d\n",frame_parms->freq_idx);
  msg("frame_parms->dual_tx=%d\n",frame_parms->dual_tx);
  msg("frame_parms->mode1_flag=%d\n",frame_parms->mode1_flag);
  msg("frame_parms->nb_antennas_tx=%d\n",frame_parms->nb_antennas_tx);
  msg("frame_parms->nb_antennas_rx=%d\n",frame_parms->nb_antennas_rx);
  msg("frame_parms->ofdm_symbol_size=%d\n",frame_parms->ofdm_symbol_size);
  msg("frame_parms->log2_symbol_size=%d\n",frame_parms->log2_symbol_size);
  msg("frame_parms->nb_prefix_samples=%d\n",frame_parms->nb_prefix_samples);
  msg("frame_parms->nb_prefix_samples0=%d\n",frame_parms->nb_prefix_samples0);
  msg("frame_parms->first_carrier_offset=%d\n",frame_parms->first_carrier_offset);
  msg("frame_parms->samples_per_tti=%d\n",frame_parms->samples_per_tti);
  msg("frame_parms->symbols_per_tti=%d\n",frame_parms->symbols_per_tti);
 
}
