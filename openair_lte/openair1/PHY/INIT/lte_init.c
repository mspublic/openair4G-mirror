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
  msg("openair_lte: Copying to PHY Framing\n");
  phy_framing->Nsymb = frame_parms->symbols_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME;
  msg("openair_lte: Nsymb %d\n",phy_framing->Nsymb);
  phy_framing->Nd = frame_parms->ofdm_symbol_size;     
  msg("openair_lte: Nd %d\n",phy_framing->Nd);

  phy_framing->Nc = frame_parms->nb_prefix_samples;    
  phy_framing->Nz = frame_parms->ofdm_symbol_size - frame_parms->N_RB_DL*12;    
  phy_framing->Nf = frame_parms->N_RB_DL;    
  phy_framing->Extension_type = CYCLIC_PREFIX;
  phy_framing->log2Nd = frame_parms->log2_symbol_size;
} 



int phy_init_lte_ue(LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_UE_COMMON *lte_ue_common_vars,
		    LTE_UE_DLSCH **lte_ue_dlsch_vars,
		    LTE_UE_PBCH **lte_ue_pbch_vars) {

  int i,j;
  unsigned char eNb_id;

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
  
  for (eNb_id=0;eNb_id<3;eNb_id++) {
    lte_ue_common_vars->dl_ch_estimates[eNb_id] = (int **)malloc16(4*sizeof(int*));
    if (lte_ue_common_vars->dl_ch_estimates[eNb_id]) {
      //#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates (eNb %d) allocated at %p\n",
	  eNb_id,lte_ue_common_vars->dl_ch_estimates[eNb_id]);
      //#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates not allocated\n");
      return(-1);
    }
  

    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++) {
	lte_ue_common_vars->dl_ch_estimates[eNb_id][(j<<1) + i] = (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*(frame_parms->ofdm_symbol_size)+LTE_CE_FILTER_LENGTH);
	if (lte_ue_common_vars->dl_ch_estimates[eNb_id][(j<<1)+i]) {
	  //#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates[%d][%d] allocated at %p\n",eNb_id,(j<<1)+i,
	      lte_ue_common_vars->dl_ch_estimates[eNb_id][(j<<1)+i]);
	  //#endif
	  
	  memset(lte_ue_common_vars->dl_ch_estimates[eNb_id][(j<<1)+i],0,frame_parms->symbols_per_tti*sizeof(int)*(frame_parms->ofdm_symbol_size));
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates[%d] not allocated\n",i);
	  return(-1);
	}
      }
  }


  lte_ue_common_vars->dl_ch_estimates_time = (int **)malloc16(4*sizeof(int*));
  if (lte_ue_common_vars->dl_ch_estimates_time) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates_time allocated at %p\n",
	   lte_ue_common_vars->dl_ch_estimates_time);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates_time not allocated_time\n");
    return(-1);
  }

  for (i=0; i<frame_parms->nb_antennas_rx; i++)
    for (j=0; j<frame_parms->nb_antennas_tx; j++) {
      lte_ue_common_vars->dl_ch_estimates_time[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
      if (lte_ue_common_vars->dl_ch_estimates_time[(j<<1)+i]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates_time[%d] allocated at %p\n",i,
	  lte_ue_common_vars->dl_ch_estimates_time[(j<<1)+i]);
#endif

      memset(lte_ue_common_vars->dl_ch_estimates_time[(j<<1)+i],0,sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_ue_common_vars->dl_ch_estimates_time[%d] not allocated\n",i);
      return(-1);
    }
  }
  

  //  lte_ue_dlsch_vars = (LTE_UE_DLSCH **)malloc16(3*sizeof(LTE_UE_DLSCH*));
  //  lte_ue_pbch_vars = (LTE_UE_PBCH **)malloc16(3*sizeof(LTE_UE_PBCH*));

  // DLSCH
  for (eNb_id=0;eNb_id<3;eNb_id++) {

    lte_ue_dlsch_vars[eNb_id] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
    msg("[OPENAIR][LTE PHY][INIT] lte_ue_dlsch_vars[%d] = %p\n",eNb_id,lte_ue_dlsch_vars[eNb_id]);

    lte_ue_dlsch_vars[eNb_id]->rxdataF_ext    = (int **)malloc16(2*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	lte_ue_dlsch_vars[eNb_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
    
    lte_ue_dlsch_vars[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	lte_ue_dlsch_vars[eNb_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
    
    lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    lte_ue_dlsch_vars[eNb_id]->dl_ch_mag = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++) {
	lte_ue_dlsch_vars[eNb_id]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
        msg("[OPENAIR][LTE PHY][INIT] lte_ue_dlsch_vars[%d]->dl_ch_mag[%d] = %p\n",eNb_id,(j<<1)+i,lte_ue_dlsch_vars[eNb_id]->dl_ch_mag[(j<<1)+i]);
      }
    lte_ue_dlsch_vars[eNb_id]->dl_ch_magb = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	lte_ue_dlsch_vars[eNb_id]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    lte_ue_dlsch_vars[eNb_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx;i++)
      lte_ue_dlsch_vars[eNb_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12));
    
    lte_ue_dlsch_vars[eNb_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
    lte_ue_dlsch_vars[eNb_id]->llr[1] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));

    
    
    // PBCH
    lte_ue_pbch_vars[eNb_id] = (LTE_UE_PBCH *)malloc16(sizeof(LTE_UE_PBCH));
    lte_ue_pbch_vars[eNb_id]->rxdataF_ext    = (int **)malloc16(2*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx;i++)
      lte_ue_pbch_vars[eNb_id]->rxdataF_ext[i] = (int *)malloc16(sizeof(int)*(6*12*4));
    
    lte_ue_pbch_vars[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
      lte_ue_pbch_vars[eNb_id]->rxdataF_comp[i] = (int *)malloc16(sizeof(int)*(6*12*4));
    
    lte_ue_pbch_vars[eNb_id]->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
      lte_ue_pbch_vars[eNb_id]->dl_ch_estimates_ext[i] = (int *)malloc16(sizeof(int)*6*12*4);
    
    lte_ue_pbch_vars[eNb_id]->llr = (short *)malloc16(384*sizeof(short));
    
    lte_ue_pbch_vars[eNb_id]->channel_output = (short *)malloc16((3*64+12)*sizeof(short));
    
    lte_ue_pbch_vars[eNb_id]->decoded_output = (unsigned char *)malloc16(64*sizeof(unsigned char));
    
    lte_ue_pbch_vars[eNb_id]->pdu_errors_conseq=0;
    lte_ue_pbch_vars[eNb_id]->pdu_errors=0;
    lte_ue_pbch_vars[eNb_id]->pdu_errors_last=0;
    lte_ue_pbch_vars[eNb_id]->pdu_fer=0;
  } 

    // Initialize Gold sequence table
    // lte_gold(frame_parms); --> moved to cbmimo1_fileops
    
    // Initialize Sync
    lte_sync_time_init(frame_parms,  lte_ue_common_vars);
    
  

  return(0);
}

int phy_init_lte_eNB(LTE_DL_FRAME_PARMS *frame_parms,
		     LTE_eNB_COMMON *eNB_common_vars) {

  int i;

  eNB_common_vars->txdataF = (mod_sym_t **)malloc16(frame_parms->nb_antennas_tx*sizeof(mod_sym_t*));
#ifdef IFFT_FPGA
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    eNB_common_vars->txdataF[i] = PHY_vars->tx_vars[i].TX_DMA_BUFFER;
  }
  eNB_common_vars->txdata = NULL;
#else
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    eNB_common_vars->txdataF[i] = (int *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
    bzero(eNB_common_vars->txdataF[i],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
  }
  eNB_common_vars->txdata = (mod_sym_t **)malloc16(frame_parms->nb_antennas_tx*sizeof(mod_sym_t*));
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    eNB_common_vars->txdata[i] = PHY_vars->tx_vars[i].TX_DMA_BUFFER;
  }
#endif  

  eNB_common_vars->rxdata = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
  if (eNB_common_vars->rxdata) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata allocated at %p\n",
	   eNB_common_vars->rxdata);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata not allocated\n");
    return(-1);
  }

  for (i=0; i<frame_parms->nb_antennas_rx; i++) {
    eNB_common_vars->rxdata[i] = PHY_vars->rx_vars[i].RX_DMA_BUFFER;
    //#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata[%d] = %p\n",i,eNB_common_vars->rxdata[i]);
    //#endif
  }

  eNB_common_vars->rxdataF = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
  if (eNB_common_vars->rxdataF) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF allocated at %p\n",
	   eNB_common_vars->rxdataF);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF not allocated\n");
    return(-1);
  }

  for (i=0; i<frame_parms->nb_antennas_rx; i++) {
    //RK 2 times because of output format of FFT!  We should get rid of this
    eNB_common_vars->rxdataF[i] = (int *)malloc16(2*sizeof(int)*(frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti));
    if (eNB_common_vars->rxdataF[i]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d] allocated at %p\n",i,
	     eNB_common_vars->rxdataF[i]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d] not allocated\n",i);
      return(-1);
    }
  }


  return (0);  
}
