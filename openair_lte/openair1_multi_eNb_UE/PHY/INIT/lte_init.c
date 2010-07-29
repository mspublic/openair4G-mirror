//#include <string.h>
#include "defs.h"
#include "PHY/extern.h"
#include "SIMULATION/TOOLS/defs.h"
#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softconfig.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_pci.h"
//#include "pci_commands.h"
#endif //CBMIMO1

#define DEBUG_PHY

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

  frame_parms->tdd_config=3;
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

void phy_init_lte_top(LTE_DL_FRAME_PARMS *lte_frame_parms) {

  lte_gold(lte_frame_parms);
  lte_sync_time_init(lte_frame_parms);
  
  generate_64qam_table();
  generate_16qam_table();
  generate_RIV_tables();
  
  generate_pcfich_reg_mapping(lte_frame_parms);
  generate_phich_reg_mapping_ext(lte_frame_parms);
  
  set_taus_seed(1328);
  
}

int phy_init_lte_ue(LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_UE_COMMON *ue_common_vars,
		    LTE_UE_DLSCH **ue_dlsch_vars,
		    LTE_UE_DLSCH **ue_dlsch_vars_cntl,
		    LTE_UE_DLSCH **ue_dlsch_vars_ra,
		    LTE_UE_DLSCH **ue_dlsch_vars_1A,
		    LTE_UE_PBCH **ue_pbch_vars,
		    LTE_UE_PDCCH **ue_pdcch_vars,
		    PHY_VARS_UE *phy_vars_ue) {

  int i,j;
  unsigned char eNb_id;

  // TX buffers
  ue_common_vars->txdataF = (mod_sym_t **)malloc16(frame_parms->nb_antennas_tx*sizeof(mod_sym_t*));
#ifdef IFFT_FPGA
#ifdef USER_MODE
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    ue_common_vars->txdataF[i] = (int *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
    bzero(ue_common_vars->txdataF[i],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
  }
#else //USER_MODE
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    ue_common_vars->txdataF[i] = PHY_vars->tx_vars[i].TX_DMA_BUFFER;
  }
  /* merge indicated the following instead
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    ue_common_vars->txdataF[i] = TX_DMA_BUFFER[0][i];
  }
  */
  ue_common_vars->txdata = NULL;
#endif
#else //IFFT_FPGA
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    ue_common_vars->txdataF[i] = (int *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
    bzero(ue_common_vars->txdataF[i],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
  }
  ue_common_vars->txdata = (mod_sym_t **)malloc16(frame_parms->nb_antennas_tx*sizeof(mod_sym_t*));
#ifdef USER_MODE
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    ue_common_vars->txdata[i] = (int *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
    bzero(ue_common_vars->txdata[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
  }
#else
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    ue_common_vars->txdata[i] = PHY_vars->tx_vars[i].TX_DMA_BUFFER;
  }
  /* merge indicated the following instead
  for (i=0; i<frame_parms->nb_antennas_tx; i++) {
    ue_common_vars->txdata[i] = TX_DMA_BUFFER[0][i];
  }
  */
#endif //USER_MODE
#endif //IFFT_FPGA

  // RX buffers
  ue_common_vars->rxdata = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
  if (ue_common_vars->rxdata) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata allocated at %p\n", ue_common_vars->rxdata);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata not allocated\n");
    return(-1);
  }

#ifndef USER_MODE
  for (i=0; i<frame_parms->nb_antennas_rx; i++) {
    ue_common_vars->rxdata[i] = RX_DMA_BUFFER[0][i];
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata[%d] = %p\n",i,ue_common_vars->rxdata[i]);
#endif
  }
#else //USER_MODE
  for (i=0; i<frame_parms->nb_antennas_rx; i++) {
    ue_common_vars->rxdata[i] = (int*) malloc16(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
    if (ue_common_vars->rxdata[i]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata[%d] allocated at %p\n",i,ue_common_vars->rxdata[i]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata[%d] not allocated\n",i);
      return(-1);
    }
  }
#endif //USER_MODE

  ue_common_vars->rxdataF = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
  if (ue_common_vars->rxdataF) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF allocated at %p\n", ue_common_vars->rxdataF);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF not allocated\n");
    return(-1);
  }

  for (i=0; i<frame_parms->nb_antennas_rx; i++) {
    //RK 2 times because of output format of FFT!  We should get rid of this
    ue_common_vars->rxdataF[i] = (int *)malloc16(2*sizeof(int)*(frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti));
    if (ue_common_vars->rxdataF[i]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF[%d] allocated at %p\n",i,ue_common_vars->rxdataF[i]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF[%d] not allocated\n",i);
      return(-1);
    }
  }

  // Channel estimates  
  for (eNb_id=0;eNb_id<3;eNb_id++) {
    ue_common_vars->dl_ch_estimates[eNb_id] = (int **)malloc16(4*sizeof(int*));
    if (ue_common_vars->dl_ch_estimates[eNb_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates (eNb %d) allocated at %p\n",
	  eNb_id,ue_common_vars->dl_ch_estimates[eNb_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates not allocated\n");
      return(-1);
    }

  

    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++) {
	ue_common_vars->dl_ch_estimates[eNb_id][(j<<1) + i] = (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*(frame_parms->ofdm_symbol_size)+LTE_CE_FILTER_LENGTH);
	if (ue_common_vars->dl_ch_estimates[eNb_id][(j<<1)+i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates[%d][%d] allocated at %p\n",eNb_id,(j<<1)+i,
	      ue_common_vars->dl_ch_estimates[eNb_id][(j<<1)+i]);
#endif
	  
	  memset(ue_common_vars->dl_ch_estimates[eNb_id][(j<<1)+i],0,frame_parms->symbols_per_tti*sizeof(int)*(frame_parms->ofdm_symbol_size));
	}
	else {
	  msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates[%d] not allocated\n",i);
	  return(-1);
	}
      }
  }


  ue_common_vars->dl_ch_estimates_time = (int **)malloc16(4*sizeof(int*));
  if (ue_common_vars->dl_ch_estimates_time) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates_time allocated at %p\n",
	   ue_common_vars->dl_ch_estimates_time);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates_time not allocated_time\n");
    return(-1);
  }

  for (i=0; i<frame_parms->nb_antennas_rx; i++)
    for (j=0; j<frame_parms->nb_antennas_tx; j++) {
      ue_common_vars->dl_ch_estimates_time[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
      if (ue_common_vars->dl_ch_estimates_time[(j<<1)+i]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates_time[%d] allocated at %p\n",i,
	    ue_common_vars->dl_ch_estimates_time[(j<<1)+i]);
#endif
	
	memset(ue_common_vars->dl_ch_estimates_time[(j<<1)+i],0,sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
      }
      else {
	msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates_time[%d] not allocated\n",i);
	return(-1);
      }
    }
  

  //  lte_ue_dlsch_vars = (LTE_UE_DLSCH **)malloc16(3*sizeof(LTE_UE_DLSCH*));
  //  lte_ue_pbch_vars = (LTE_UE_PBCH **)malloc16(3*sizeof(LTE_UE_PBCH*));

  // DLSCH
  for (eNb_id=0;eNb_id<3;eNb_id++) {

    ue_dlsch_vars[eNb_id] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
    ue_dlsch_vars_cntl[eNb_id] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
    ue_dlsch_vars_ra[eNb_id] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
    ue_dlsch_vars_1A[eNb_id] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
    ue_pdcch_vars[eNb_id] = (LTE_UE_PDCCH *)malloc16(sizeof(LTE_UE_PDCCH));
#ifdef DEBUG_PHY
    msg("[OPENAIR][LTE PHY][INIT] ue_dlsch_vars[%d] = %p\n",eNb_id,ue_dlsch_vars[eNb_id]);
    msg("[OPENAIR][LTE PHY][INIT] ue_dlsch_vars_cntl[%d] = %p\n",eNb_id,ue_dlsch_vars_cntl[eNb_id]);
    msg("[OPENAIR][LTE PHY][INIT] ue_dlsch_vars_ra[%d] = %p\n",eNb_id,ue_dlsch_vars_ra[eNb_id]);
    msg("[OPENAIR][LTE PHY][INIT] ue_dlsch_vars_1A[%d] = %p\n",eNb_id,ue_dlsch_vars_1A[eNb_id]);
    msg("[OPENAIR][LTE PHY][INIT] ue_pdcch_vars[%d] = %p\n",eNb_id,ue_pdcch_vars[eNb_id]);
#endif

    ue_dlsch_vars[eNb_id]->rxdataF_ext    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[eNb_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
    
    ue_dlsch_vars[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[eNb_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));

    ue_pdcch_vars[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_pdcch_vars[eNb_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*4));

    ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

    ue_dlsch_vars[eNb_id]->dl_ch_rho_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[eNb_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

    ue_dlsch_vars[eNb_id]->pmi_ext = (unsigned char *)malloc16(frame_parms->N_RB_DL);

    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[eNb_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    ue_dlsch_vars[eNb_id]->dl_ch_mag = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++) 
	ue_dlsch_vars[eNb_id]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    ue_dlsch_vars[eNb_id]->dl_ch_magb = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[eNb_id]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    ue_dlsch_vars[eNb_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx;i++)
      ue_dlsch_vars[eNb_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));
    
    ue_dlsch_vars[eNb_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
    ue_dlsch_vars[eNb_id]->llr[1] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));

    ue_dlsch_vars[eNb_id]->llr128 = (short **)malloc16(sizeof(short **));


    ue_dlsch_vars_cntl[eNb_id]->rxdataF_ext    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_cntl[eNb_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
    
    ue_dlsch_vars_cntl[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));

    ue_dlsch_vars_cntl[eNb_id]->pmi_ext = (unsigned char *)malloc16(frame_parms->N_RB_DL);

    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_cntl[eNb_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));

    ue_dlsch_vars_cntl[eNb_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx;i++)
      ue_dlsch_vars_cntl[eNb_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));
    
    ue_dlsch_vars_cntl[eNb_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));


    /***/

    ue_dlsch_vars_ra[eNb_id]->rxdataF_ext    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_ra[eNb_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
    
    ue_dlsch_vars_ra[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_ra[eNb_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));

    ue_dlsch_vars_ra[eNb_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));

    ue_dlsch_vars_ra[eNb_id]->pmi_ext = (unsigned char *)malloc16(frame_parms->N_RB_DL);

    for (i=0;i<frame_parms->nb_antennas_rx;i++)
      ue_dlsch_vars_ra[eNb_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));
    
    ue_dlsch_vars_ra[eNb_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));

    /***/

    ue_dlsch_vars_1A[eNb_id]->rxdataF_ext    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_1A[eNb_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
    
    ue_dlsch_vars_1A[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_1A[eNb_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));

    ue_dlsch_vars_1A[eNb_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx;i++)
      ue_dlsch_vars_1A[eNb_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));
    
    ue_dlsch_vars_1A[eNb_id]->pmi_ext = (unsigned char *)malloc16(frame_parms->N_RB_DL);

    ue_dlsch_vars_1A[eNb_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));

    /***/

    ue_pdcch_vars[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_pdcch_vars[eNb_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*4));

    /***/

    ue_dlsch_vars_cntl[eNb_id]->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_cntl[eNb_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

    ue_dlsch_vars_cntl[eNb_id]->dl_ch_rho_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_cntl[eNb_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    ue_dlsch_vars_cntl[eNb_id]->dl_ch_mag = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++) 
	ue_dlsch_vars_cntl[eNb_id]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    ue_dlsch_vars_cntl[eNb_id]->dl_ch_magb = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_cntl[eNb_id]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    ue_dlsch_vars_cntl[eNb_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));

    /***/
  
    ue_dlsch_vars_cntl[eNb_id]->llr128 = (short **)malloc16(sizeof(short **));

    ue_dlsch_vars_ra[eNb_id]->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_ra[eNb_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

    ue_dlsch_vars_ra[eNb_id]->dl_ch_rho_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_ra[eNb_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    ue_dlsch_vars_ra[eNb_id]->dl_ch_mag = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++) 
	ue_dlsch_vars_ra[eNb_id]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    ue_dlsch_vars_ra[eNb_id]->dl_ch_magb = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_ra[eNb_id]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    //    ue_dlsch_vars_ra[eNb_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));

    ue_dlsch_vars_ra[eNb_id]->llr128 = (short **)malloc16(sizeof(short **));
    /***/
  
    ue_dlsch_vars_1A[eNb_id]->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_1A[eNb_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

    ue_dlsch_vars_1A[eNb_id]->dl_ch_rho_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_1A[eNb_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    ue_dlsch_vars_1A[eNb_id]->dl_ch_mag = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++) 
	ue_dlsch_vars_1A[eNb_id]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    ue_dlsch_vars_1A[eNb_id]->dl_ch_magb = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars_1A[eNb_id]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    //    ue_dlsch_vars_1A[eNb_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));

    ue_dlsch_vars_1A[eNb_id]->llr128 = (short **)malloc16(sizeof(short **));
    /***/  

   ue_pdcch_vars[eNb_id]->rxdataF_ext    = (int **)malloc16(4*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_pdcch_vars[eNb_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));

    ue_pdcch_vars[eNb_id]->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++)
	ue_pdcch_vars[eNb_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    ue_pdcch_vars[eNb_id]->llr = (unsigned short *)malloc16(4*frame_parms->N_RB_DL*12*sizeof(unsigned short));
    ue_pdcch_vars[eNb_id]->wbar = (unsigned short *)malloc16(4*frame_parms->N_RB_DL*12*sizeof(unsigned short));

    ue_pdcch_vars[eNb_id]->e_rx = (char *)malloc16(4*2*frame_parms->N_RB_DL*12*sizeof(unsigned char));
    
    // PBCH
    ue_pbch_vars[eNb_id] = (LTE_UE_PBCH *)malloc16(sizeof(LTE_UE_PBCH));
    ue_pbch_vars[eNb_id]->rxdataF_ext    = (int **)malloc16(2*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx;i++)
      ue_pbch_vars[eNb_id]->rxdataF_ext[i] = (int *)malloc16(sizeof(int)*(6*12*4));
    
    ue_pbch_vars[eNb_id]->rxdataF_comp    = (int **)malloc16(4*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
      ue_pbch_vars[eNb_id]->rxdataF_comp[i] = (int *)malloc16(sizeof(int)*(6*12*4));
    
    ue_pbch_vars[eNb_id]->dl_ch_estimates_ext = (int **)malloc16(4*sizeof(short*));
    for (i=0;i<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;i++)
      ue_pbch_vars[eNb_id]->dl_ch_estimates_ext[i] = (int *)malloc16(sizeof(int)*6*12*4);
    
    ue_pbch_vars[eNb_id]->llr = (short *)malloc16(36*12*2*sizeof(short));
    
    ue_pbch_vars[eNb_id]->channel_output = (short *)malloc16((3*64+12)*sizeof(short));
    
    ue_pbch_vars[eNb_id]->decoded_output = (unsigned char *)malloc16(64*sizeof(unsigned char));
    
    ue_pbch_vars[eNb_id]->pdu_errors_conseq=0;
    ue_pbch_vars[eNb_id]->pdu_errors=0;
    ue_pbch_vars[eNb_id]->pdu_errors_last=0;
    ue_pbch_vars[eNb_id]->pdu_fer=0;
  } 

  // Initialize Gold sequence table
  // lte_gold(frame_parms); --> moved to cbmimo1_fileops
  
  // Initialize Sync
  // lte_sync_time_init(frame_parms); --> moved to cbmimo1_fileops
#ifndef NO_UL_REF 
  generate_ul_ref_sigs();
#endif


  if (phy_vars_ue->is_secondary_ue) {
      phy_vars_ue->ul_precoder_S_UE = (int **)malloc16(4*sizeof(int*));
      if (phy_vars_ue->ul_precoder_S_UE) {
#ifdef DEBUG_PHY
	msg("[openair][SECSYS_PHY][INIT] phy_vars_ue->ul_precoder_S_UE allocated at %p\n",phy_vars_ue->ul_precoder_S_UE);
#endif
      }
      else {
	msg("[openair][SECSYS_PHY][INIT] phy_vars_ue->ul_precoder_S_UE not allocated\n");
	return(-1);
      }
      
      for (j=0; j<phy_vars_ue->lte_frame_parms.nb_antennas_tx; j++) {
	phy_vars_ue->ul_precoder_S_UE[j] = (int *)malloc16(2*sizeof(int)*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)); // repeated format (hence the '2*')
	if (phy_vars_ue->ul_precoder_S_UE[j]) {
#ifdef DEBUG_PHY
	  msg("[openair][SECSYS_PHY][INIT] phy_vars_ue->ul_precoder_S_UE[%d] allocated at %p\n",j,
	      phy_vars_ue->ul_precoder_S_UE[j]);
#endif
	  memset(phy_vars_ue->ul_precoder_S_UE[j],0,2*sizeof(int)*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size));
	}
	else {
	  msg("[openair][SECSYS_PHY][INIT] phy_vars_ue->ul_precoder_S_UE[%d] not allocated\n",j);
	  return(-1);
	}
      } //for(j=...nb_antennas_tx
  }

  return(0);
}

int phy_init_lte_eNB(LTE_DL_FRAME_PARMS *frame_parms,
		     LTE_eNB_COMMON *eNB_common_vars,
		     LTE_eNB_ULSCH **eNB_ulsch_vars,
		     unsigned char is_secondary_eNb,
		     PHY_VARS_eNB *phy_vars_eNb){

  int i, j, eNb_id, UE_id;


  phy_vars_eNb->first_run_timing_advance = 1; ///This flag used to be static. With multiple eNBs this does no longer work, hence we put it in the structure. However it has to be initialized with 1, which is performed here.
  phy_vars_eNb->first_run_I0_measurements = 1; ///This flag used to be static. With multiple eNBs this does no longer work, hence we put it in the structure. However it has to be initialized with 1, which is performed here.

  for (eNb_id=0; eNb_id<3; eNb_id++) {

    // TX vars
#ifndef IFFT_FPGA
    eNB_common_vars->txdata[eNb_id] = (mod_sym_t **)malloc16(frame_parms->nb_antennas_tx*sizeof(mod_sym_t*));
    if (eNB_common_vars->txdata[eNb_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdata[%d] allocated at %p\n",eNb_id,
	  eNB_common_vars->txdata[eNb_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdata[%d] not allocated\n",eNb_id);
      return(-1);
    }

    for (i=0; i<frame_parms->nb_antennas_tx; i++) {
#ifndef USER_MODE
      if (eNb_id == 0)
	eNB_common_vars->txdata[eNb_id][i] = PHY_vars->tx_vars[i].TX_DMA_BUFFER;
      else
#endif
	{
	  eNB_common_vars->txdata[eNb_id][i] = (mod_sym_t *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
	  bzero(eNB_common_vars->txdata[eNb_id][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
	}
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdata[%d][%d] = %p\n",eNb_id,i,eNB_common_vars->txdata[eNb_id][i]);
#endif
    }
#else // IFFT_FPGA
    for (i=0; i<frame_parms->nb_antennas_tx; i++) {
      eNB_common_vars->txdata[eNb_id][i] = (mod_sym_t *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t));
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdata[%d][%d] = %p\n",eNb_id,i,eNB_common_vars->txdata[eNb_id][i]);
#endif
    }
#endif
  

    eNB_common_vars->txdataF[eNb_id] = (mod_sym_t **)malloc16(frame_parms->nb_antennas_tx*sizeof(mod_sym_t*));
    if (eNB_common_vars->txdataF[eNb_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdataF[%d] allocated at %p\n",eNb_id,
	  eNB_common_vars->txdataF[eNb_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdataF[%d] not allocated\n",eNb_id);
      return(-1);
    }
#ifdef IFFT_FPGA
    for (i=0; i<frame_parms->nb_antennas_tx; i++) {
#ifndef USER_MODE
      if (eNb_id == 0)
	eNB_common_vars->txdataF[eNb_id][i] = PHY_vars->tx_vars[i].TX_DMA_BUFFER;
      else
#endif
	{
	  eNB_common_vars->txdataF[eNb_id][i] = (mod_sym_t **)malloc16(NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_SYMBOLS_PER_FRAME*sizeof(mod_sym_t*));
	    bzero(eNB_common_vars->txdataF[eNb_id][i],NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_SYMBOLS_PER_FRAME*sizeof(mod_sym_t));
	}
      

#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdataF[%d][%d] = %p, length = %d\n",
	  eNb_id,i,eNB_common_vars->txdataF[eNb_id][i],
	  NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_SYMBOLS_PER_FRAME*sizeof(mod_sym_t));
#endif
    }
#else //IFFT_FPGA
    for (i=0; i<frame_parms->nb_antennas_tx; i++) {
      eNB_common_vars->txdataF[eNb_id][i] = (int *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
      bzero(eNB_common_vars->txdataF[eNb_id][i],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdataF[%d][%d] = %p, length = %d\n",eNb_id,i,eNB_common_vars->txdataF[eNb_id][i],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(int));
#endif
    }
#endif  
    
    //RX vars
    eNB_common_vars->rxdata[eNb_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    if (eNB_common_vars->rxdata[eNb_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata[%d] allocated at %p\n",eNb_id,
	  eNB_common_vars->rxdata[eNb_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata[%d] not allocated\n",eNb_id);
      return(-1);
    }
    
    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	eNB_common_vars->rxdata[eNb_id][i] = RX_DMA_BUFFER[eNb_id][i];
	//	bzero(eNB_common_vars->rxdata[eNb_id][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata[%d][%d] = %p\n",eNb_id,i,eNB_common_vars->rxdata[eNb_id][i]);
#endif
    }
    
    eNB_common_vars->rxdataF[eNb_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    if (eNB_common_vars->rxdataF[eNb_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d] allocated at %p\n",eNb_id,
	  eNB_common_vars->rxdataF[eNb_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d] not allocated\n",eNb_id);
      return(-1);
    }
    
    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
      //RK 2 times because of output format of FFT!  We should get rid of this
      eNB_common_vars->rxdataF[eNb_id][i] = (int *)malloc16(2*sizeof(int)*(frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti));
      if (eNB_common_vars->rxdataF[eNb_id][i]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d][%d] allocated at %p\n",eNb_id,i,
	    eNB_common_vars->rxdataF[eNb_id][i]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d][%d] not allocated\n",eNb_id,i);
	return(-1);
      }
    }
    
    // Channel estimates for SRS
    eNB_common_vars->srs_ch_estimates[eNb_id] = (int **)malloc16(4*sizeof(int*));
    if (eNB_common_vars->srs_ch_estimates[eNb_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->srs_ch_estimates[%d] allocated at %p\n",eNb_id,
	  eNB_common_vars->srs_ch_estimates[eNb_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->srs_ch_estimates[%d] not allocated\n",eNb_id);
      return(-1);
    }
    
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<2; j++) { //frame_parms->nb_antennas_tx
	eNB_common_vars->srs_ch_estimates[eNb_id][(j<<1) + i] = (int *)malloc16(sizeof(int)*(frame_parms->ofdm_symbol_size));
	if (eNB_common_vars->srs_ch_estimates[eNb_id][(j<<1)+i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->srs_ch_estimates[%d][%d] allocated at %p\n",eNb_id,(j<<1)+i,
	      eNB_common_vars->srs_ch_estimates[eNb_id][(j<<1)+i]);
#endif
	  
	  memset(eNB_common_vars->srs_ch_estimates[eNb_id][(j<<1)+i],0,sizeof(int)*(frame_parms->ofdm_symbol_size));
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->srs_ch_estimates[%d] not allocated\n",i);
	  return(-1);
	}
      }

    // Channel estimates for SRS (time)
    eNB_common_vars->srs_ch_estimates_time[eNb_id] = (int **)malloc16(4*sizeof(int*));
    if (eNB_common_vars->srs_ch_estimates_time[eNb_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->srs_ch_estimates_time allocated at %p\n",
	  eNB_common_vars->srs_ch_estimates_time[eNb_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->srs_ch_estimates_time not allocated\n");
      return(-1);
    }

    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<frame_parms->nb_antennas_tx; j++) {
	eNB_common_vars->srs_ch_estimates_time[eNb_id][(j<<1) + i] = (int *)malloc16(sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
	if (eNB_common_vars->srs_ch_estimates_time[eNb_id][(j<<1)+i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->srs_ch_estimates_time[%d] allocated at %p\n",(j<<1)+i,
	      eNB_common_vars->srs_ch_estimates_time[eNb_id][(j<<1)+i]);
#endif
	  
	  memset(eNB_common_vars->srs_ch_estimates_time[eNb_id][(j<<1)+i],0,sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->srs_ch_estimates_time[%d] not allocated\n",i);
	  return(-1);
	}
      }

    eNB_common_vars->sync_corr[eNb_id] = (unsigned int *)malloc16(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int)*frame_parms->samples_per_tti);
    if (eNB_common_vars->sync_corr[eNb_id]) {
      bzero(eNB_common_vars->sync_corr[eNb_id],LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int)*frame_parms->samples_per_tti);
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->sync_corr[%d] allocated at %p\n", eNb_id, eNB_common_vars->sync_corr[eNb_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->sync_corr[%d] not allocated\n", eNb_id);
      return(-1);
    }


  } //eNb_id


#ifndef NO_UL_REF 
  generate_ul_ref_sigs_rx();
  // SRS
  eNB_common_vars->srs = (int *)malloc16(2*frame_parms->ofdm_symbol_size*sizeof(int*));
  if (!eNB_common_vars->srs || 
      (generate_srs_rx(frame_parms, eNB_common_vars->srs))==-1) {
    msg("[openair][LTE_PHY][INIT] Error generating SRS\n");
    return(-1);
  }
  else 
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] SRS generated at %p\n",eNB_common_vars->srs);
#endif

#endif

  // ULSCH VARS

  for (UE_id=0; UE_id<NUMBER_OF_UE_MAX; UE_id++) {

    eNB_ulsch_vars[UE_id] = (LTE_eNB_ULSCH *)malloc16(NUMBER_OF_UE_MAX*sizeof(LTE_eNB_ULSCH));
    if (eNB_ulsch_vars[UE_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d] allocated at %p\n",UE_id,eNB_ulsch_vars[UE_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d] not allocated\n",UE_id);
      return(-1);
    }

    for (eNb_id=0; eNb_id<3; eNb_id++) {

      eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      if (eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d] allocated at %p\n",UE_id,eNb_id,
	    eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d] not allocated\n",UE_id,eNb_id);
	return(-1);
      }
      
      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	//RK 2 times because of output format of FFT!  We should get rid of this
	eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id][i] = 
	  (int *)malloc16(2*sizeof(int)*(frame_parms->N_RB_UL*12*frame_parms->symbols_per_tti));
	if (eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id][i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d][%d] allocated at %p\n",UE_id,eNb_id,i,
	      eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id][i]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d][%d] not allocated\n",UE_id,eNb_id,i);
	  return(-1);
	}
      }

      eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNb_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      if (eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNb_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext2[%d] allocated at %p\n",UE_id,eNb_id,
	    eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNb_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d] not allocated\n",UE_id,eNb_id);
	return(-1);
      }

      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNb_id][i] = 
	  (int *)malloc16(sizeof(int)*(frame_parms->N_RB_UL*12*frame_parms->symbols_per_tti));
	if (eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNb_id][i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext2[%d][%d] allocated at %p\n",UE_id,eNb_id,i,
	      eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNb_id][i]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d][%d] not allocated\n",UE_id,eNb_id,i);
	  return(-1);
	}
      }
      
      // Channel estimates for DRS
      eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id] = (int **)malloc16(4*sizeof(int*));
      if (eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates[%d] allocated at %p\n",UE_id,eNb_id,
	    eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates[%d] not allocated\n",UE_id,eNb_id);
	return(-1);
      }
      
      
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<2; j++) { //frame_parms->nb_antennas_tx
	  eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id][(j<<1) + i] = 
	    (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  if (eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id][(j<<1)+i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates[%d][%d] allocated at %p\n",UE_id,eNb_id,(j<<1)+i,
		eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id][(j<<1)+i]);
#endif
	    
	    memset(eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id][(j<<1)+i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates[%d][%d] not allocated\n",UE_id,eNb_id,(j<<1)+i);
	    return(-1);
	  }
	}
      
      eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
      if (eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp[%d] allocated at %p\n",UE_id,eNb_id,
	    eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp[%d] not allocated\n",UE_id,eNb_id);
	return(-1);
      }

      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id][i] = 
	  (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	if (eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id][i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp[%d][%d] allocated at %p\n",UE_id,eNb_id,i,
	      eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id][i]);
#endif
	  
	  memset(eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp[%d][%d] not allocated\n",UE_id,eNb_id,i);
	  return(-1);
	}
      }
      
      eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
      if (eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag[%d] allocated at %p\n",UE_id,eNb_id,
	    eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag[%d] not allocated\n",UE_id,eNb_id);
	return(-1);
      }

      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id][i] = 
	  (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	if (eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id][i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag[%d][%d] allocated at %p\n",UE_id,eNb_id,i,
	      eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id][i]);
#endif
	  
	  memset(eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag[%d][%d] not allocated\n",UE_id,eNb_id,i);
	  return(-1);
	}
      }

      eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
      if (eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb[%d] allocated at %p\n",UE_id,eNb_id,
	    eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb[%d] not allocated\n",UE_id,eNb_id);
	return(-1);
      }

      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id][i] = 
	  (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	if (eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id][i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb[%d][%d] allocated at %p\n",UE_id,eNb_id,i,
	      eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id][i]);
#endif
	  
	  memset(eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb[%d][%d] not allocated\n",UE_id,eNb_id,i);
	  return(-1);
	}
      }

    } //eNb_id

    eNB_ulsch_vars[UE_id]->llr = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
    if (! eNB_ulsch_vars[UE_id]->llr) {
      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->llr not allocated\n",UE_id);
      return(-1);
    }
    
  } //UE_id 

  if (is_secondary_eNb) {
    for (eNb_id=0; eNb_id<3; eNb_id++) {
      phy_vars_eNb->dl_precoder_SeNb[eNb_id] = (int **)malloc16(4*sizeof(int*));
      if (phy_vars_eNb->dl_precoder_SeNb[eNb_id]) {
#ifdef DEBUG_PHY
	msg("[openair][SECSYS_PHY][INIT] phy_vars_eNb->dl_precoder_SeNb[%d] allocated at %p\n",eNb_id,
	    phy_vars_eNb->dl_precoder_SeNb[eNb_id]);
#endif
      }
      else {
	msg("[openair][SECSYS_PHY][INIT] phy_vars_eNb->dl_precoder_SeNb[%d] not allocated\n",eNb_id);
	return(-1);
      }
      
      for (j=0; j<2; j++) { //phy_vars_eNb->lte_frame_parms.nb_antennas_tx
	phy_vars_eNb->dl_precoder_SeNb[eNb_id][j] = (int *)malloc16(2*sizeof(int)*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)); // repeated format (hence the '2*')
	if (phy_vars_eNb->dl_precoder_SeNb[eNb_id][j]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] phy_vars_eNb->dl_precoder_SeNb[%d][%d] allocated at %p\n",eNb_id,j,
	      phy_vars_eNb->dl_precoder_SeNb[eNb_id][j]);
#endif
	  memset(phy_vars_eNb->dl_precoder_SeNb[eNb_id][j],0,2*sizeof(int)*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size));
	}
	else {
	  msg("[openair][LTE_PHY][INIT] phy_vars_eNb->dl_precoder_SeNb[%d][%d] not allocated\n",eNb_id,j);
	  return(-1);
	}
      } //for(j=...nb_antennas_tx
      
    } //for(eNb_id...
  }
  return (0);  
}
    
