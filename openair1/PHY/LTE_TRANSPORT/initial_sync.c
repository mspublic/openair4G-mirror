#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"

int pbch_detection(PHY_VARS_UE *phy_vars_ue) {

  u8 l,pbch_decoded,frame_mod4,pbch_tx_ant,dummy;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;

  msg("[PHY][UE%d] Initial sync: starting PBCH detection (rx_offset %d)\n",phy_vars_ue->Mod_id,
      phy_vars_ue->rx_offset);

  for (l=0;l<frame_parms->symbols_per_tti/2;l++) {
    
    slot_fep(phy_vars_ue,
	     l,
	     1,
	     phy_vars_ue->rx_offset,
	     0);
  }
  
  lte_ue_measurements(phy_vars_ue,
		      phy_vars_ue->rx_offset,
		      0,
		      0);

	msg("[PHY][UE %d][initial sync] RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), avg rx power %d dB (%d lin), RX gain %d dB\n",
		  phy_vars_ue->Mod_id,
		  phy_vars_ue->PHY_measurements.rx_rssi_dBm[0] - ((phy_vars_ue->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0), 
		  phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][0],
		  phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][1],
		  phy_vars_ue->PHY_measurements.wideband_cqi[0][0],
		  phy_vars_ue->PHY_measurements.wideband_cqi[0][1],		  
		  phy_vars_ue->PHY_measurements.rx_power_avg_dB[0],
		  phy_vars_ue->PHY_measurements.rx_power_avg[0],
		  phy_vars_ue->rx_total_gain_dB);
      
	msg("[PHY][UE %d][initial sync] N0 %d dBm digital (%d, %d) dB, linear (%d, %d), avg noise power %d dB (%d lin)\n",
		  phy_vars_ue->Mod_id,
		  phy_vars_ue->PHY_measurements.n0_power_tot_dBm,
		  phy_vars_ue->PHY_measurements.n0_power_dB[0],
		  phy_vars_ue->PHY_measurements.n0_power_dB[1],
		  phy_vars_ue->PHY_measurements.n0_power[0],
		  phy_vars_ue->PHY_measurements.n0_power[1],
		  phy_vars_ue->PHY_measurements.n0_power_avg_dB,
		  phy_vars_ue->PHY_measurements.n0_power_avg);
  
  pbch_decoded = 0;
  for (frame_mod4=0;frame_mod4<4;frame_mod4++) {
    pbch_tx_ant = rx_pbch(&phy_vars_ue->lte_ue_common_vars,
			  phy_vars_ue->lte_ue_pbch_vars[0],
			  &phy_vars_ue->lte_frame_parms,
			  0,
			  SISO,
			  frame_mod4);
    if ((pbch_tx_ant>0) && (pbch_tx_ant<=4)) {
      pbch_decoded = 1;
      break;
    }
    
    pbch_tx_ant = rx_pbch(&phy_vars_ue->lte_ue_common_vars,
			  phy_vars_ue->lte_ue_pbch_vars[0],
			  &phy_vars_ue->lte_frame_parms,
			  0,
			  ALAMOUTI,
			  frame_mod4);
    if ((pbch_tx_ant>0) && (pbch_tx_ant<=4)) {
      pbch_decoded = 1;
      break;
    }
  }
  
  
  if (pbch_decoded) {
    
    frame_parms->nb_antennas_tx = pbch_tx_ant;
    
    // set initial transmission mode to 1 or 2 depending on number of detected TX antennas
    frame_parms->mode1_flag = (pbch_tx_ant==1);
    // openair_daq_vars.dlsch_transmission_mode = (pbch_tx_ant>1) ? 2 : 1;
    
    // now check for Bandwidth of Cell
    dummy = (phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[0]>>5)&7;
    switch (dummy) {
      
    case 0 : 
      frame_parms->N_RB_DL = 6;
      break;
    case 1 : 
      frame_parms->N_RB_DL = 15;
      break;
    case 2 : 
      frame_parms->N_RB_DL = 25;
      break;
    case 3 : 
      frame_parms->N_RB_DL = 50;
      break;
    case 4 : 
      frame_parms->N_RB_DL = 100;
      break;
    default:
      msg("[PHY][UE%d] Initial sync: PBCH decoding: Unknown N_RB_DL\n",phy_vars_ue->Mod_id);
      return -1;
      break;
    }
#ifndef USER_MODE
    if (frame_parms->N_RB_DL != 25) {
      msg("[PHY][UE%d] Initial sync: PBCH decoding: Detected NB_RB %d, but CBMIMO1 can only handle NB_RB=25\n",phy_vars_ue->Mod_id,frame_parms->N_RB_DL);
      return -1;
    }
#endif
    
    // now check for PHICH parameters
    frame_parms->phich_config_common.phich_duration = (PHICH_DURATION_t)((phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[0]>>4)&1);
    dummy = (phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[0]>>2)&3;
    switch (dummy) {
    case 0:
      frame_parms->phich_config_common.phich_resource = oneSixth;
      break;
    case 1:
      frame_parms->phich_config_common.phich_resource = half;
      break;
    case 2:
      frame_parms->phich_config_common.phich_resource = one;
      break;
    case 3:
      frame_parms->phich_config_common.phich_resource = two;
      break;
    default:
      msg("[PHY][UE%d] Initial sync: Unknown PHICH_DURATION\n",phy_vars_ue->Mod_id);
      return -1;
      break;
    }
    
    mac_xface->frame = 	(((phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[0]&3)<<6) + (phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[1]>>2))<<2;
    mac_xface->frame += frame_mod4;
    // one frame delay
    mac_xface->frame ++;
    msg("[PHY][UE%d] Initial sync: pbch decoded sucessfully mode1_flag %d, tx_ant %d, frame %d, N_RB_DL %d, phich_duration %d, phich_resource %d!\n",
	phy_vars_ue->Mod_id,
	frame_parms->mode1_flag,
	pbch_tx_ant,
	mac_xface->frame,
	frame_parms->N_RB_DL,
	frame_parms->phich_config_common.phich_duration,
	frame_parms->phich_config_common.phich_resource);
    return(0);
  }
  else {
    msg("[PHY][UE%d] Initial sync: No PBCH found\n",phy_vars_ue->Mod_id);
    return(-1);
  }
  
}

int initial_sync(PHY_VARS_UE *phy_vars_ue) {
 
  u32 sync_pos,sync_pos_slot;
  u32 metric_fdd_ncp=0,metric_fdd_ecp=0,metric_tdd_ncp=0,metric_tdd_ecp=0,max_metric;
  u8 phase_fdd_ncp,phase_fdd_ecp,phase_tdd_ncp,phase_tdd_ecp;
  u8 flip_fdd_ncp,flip_fdd_ecp,flip_tdd_ncp,flip_tdd_ecp;
  u16 Nid_cell_fdd_ncp,Nid_cell_fdd_ecp,Nid_cell_tdd_ncp,Nid_cell_tdd_ecp;
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_ue->lte_frame_parms;
  u8 i;
  int ret;

  sync_pos = lte_sync_time(phy_vars_ue->lte_ue_common_vars.rxdata, 
			   &phy_vars_ue->lte_frame_parms, 
			   (int *)&phy_vars_ue->lte_ue_common_vars.eNb_id);
  sync_pos -= phy_vars_ue->lte_frame_parms.nb_prefix_samples;

  msg("[PHY][UE%d] Initial sync : Estimated PSS position %d, Nid2 %d\n",phy_vars_ue->Mod_id,sync_pos,phy_vars_ue->lte_ue_common_vars.eNb_id);

  // SSS detection
  // First try FDD normal prefix (one symbol before PSS)
  phy_vars_ue->lte_frame_parms.Ncp=0;
  phy_vars_ue->lte_frame_parms.frame_type=0;
  init_frame_parms(&phy_vars_ue->lte_frame_parms,1);

  // PSS is hypothesized in last symbol of first slot in Frame
  sync_pos_slot = (frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - frame_parms->nb_prefix_samples;

  if (sync_pos >= sync_pos_slot)
    phy_vars_ue->rx_offset = sync_pos - sync_pos_slot;  
  else
    phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos - sync_pos_slot;
  
  if (((sync_pos - sync_pos_slot) >=0 ) && 
      ((sync_pos - sync_pos_slot) < (FRAME_LENGTH_COMPLEX_SAMPLES/2 - frame_parms->samples_per_tti)) ) {
    
    
    // now do SSS detection
    // FDD normal prefix
    rx_sss(phy_vars_ue,&metric_fdd_ncp,&flip_fdd_ncp,&phase_fdd_ncp);
    Nid_cell_fdd_ncp = phy_vars_ue->lte_frame_parms.Nid_cell; 
    //    printf("FDD Normal prefix CellId %d metric %d, phase %d, flip %d\n",
    //   	   Nid_cell_fdd_ncp,metric_fdd_ncp,phase_fdd_ncp,flip_fdd_ncp);
  }
  
  // Now FDD extended prefix
  phy_vars_ue->lte_frame_parms.Ncp=1;
  phy_vars_ue->lte_frame_parms.frame_type=0;
  init_frame_parms(&phy_vars_ue->lte_frame_parms,1);

  // PSS is hypothesized in last symbol of first slot in Frame
  sync_pos_slot = (frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - frame_parms->nb_prefix_samples;
  
  if (sync_pos >= sync_pos_slot)
    phy_vars_ue->rx_offset = sync_pos - sync_pos_slot;  
  else
    phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos - sync_pos_slot;
  
  if (((sync_pos - sync_pos_slot) >=0 ) && 
      ((sync_pos - sync_pos_slot) < (FRAME_LENGTH_COMPLEX_SAMPLES/2 - frame_parms->samples_per_tti)) ) {
   
    rx_sss(phy_vars_ue,&metric_fdd_ecp,&flip_fdd_ecp,&phase_fdd_ecp);
    Nid_cell_fdd_ecp = phy_vars_ue->lte_frame_parms.Nid_cell; 
    //    printf("FDD Extended prefix CellId %d metric %d, phase %d, flip %d\n",
    //       	   Nid_cell_fdd_ecp,metric_fdd_ecp,phase_fdd_ecp,flip_fdd_ecp);
  }


    // Now TDD normal prefix
  phy_vars_ue->lte_frame_parms.Ncp=0;
  phy_vars_ue->lte_frame_parms.frame_type=1;
  init_frame_parms(&phy_vars_ue->lte_frame_parms,1);

  // PSS is hypothesized in 2nd symbol of third slot in Frame (S-subframe)
  sync_pos_slot = frame_parms->samples_per_tti + (frame_parms->ofdm_symbol_size<<1) + frame_parms->nb_prefix_samples0 + frame_parms->nb_prefix_samples;
  
  if (sync_pos >= sync_pos_slot)
    phy_vars_ue->rx_offset = sync_pos - sync_pos_slot;  
  else
    phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos - sync_pos_slot;
  
  if (((sync_pos - sync_pos_slot) >=0 ) && 
      ((sync_pos - sync_pos_slot) < (FRAME_LENGTH_COMPLEX_SAMPLES/2 - frame_parms->samples_per_tti)) ) {
   
    rx_sss(phy_vars_ue,&metric_tdd_ncp,&flip_tdd_ncp,&phase_tdd_ncp);
    Nid_cell_tdd_ncp = phy_vars_ue->lte_frame_parms.Nid_cell; 
    //    printf("TDD Normal prefix CellId %d metric %d, phase %d, flip %d\n",
    //	   Nid_cell_tdd_ncp,metric_tdd_ncp,phase_tdd_ncp,flip_tdd_ncp);
  }

    // Nod TDD extended prefix
  phy_vars_ue->lte_frame_parms.Ncp=1;
  phy_vars_ue->lte_frame_parms.frame_type=1;
  init_frame_parms(&phy_vars_ue->lte_frame_parms,1);

  // PSS is hypothesized in 2nd symbol of third slot in Frame (S-subframe)
  sync_pos_slot = frame_parms->samples_per_tti + (frame_parms->ofdm_symbol_size<<1) + frame_parms->nb_prefix_samples0 + frame_parms->nb_prefix_samples;
  
  if (sync_pos >= sync_pos_slot)
    phy_vars_ue->rx_offset = sync_pos - sync_pos_slot;  
  else
    phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos - sync_pos_slot;
  
  if (((sync_pos - sync_pos_slot) >=0 ) && 
      ((sync_pos - sync_pos_slot) < (FRAME_LENGTH_COMPLEX_SAMPLES/2 - frame_parms->samples_per_tti)) ) {
   
    rx_sss(phy_vars_ue,&metric_tdd_ecp,&flip_tdd_ecp,&phase_tdd_ecp);
    Nid_cell_tdd_ecp = phy_vars_ue->lte_frame_parms.Nid_cell; 
    //    printf("TDD Extended prefix CellId %d metric %d, phase %d, flip %d\n",
    //    	   Nid_cell_tdd_ecp,metric_tdd_ecp,phase_tdd_ecp,flip_tdd_ecp);
  }
  

  max_metric = (metric_fdd_ncp>metric_fdd_ecp) ? metric_fdd_ncp : metric_fdd_ecp;
  max_metric = (metric_tdd_ncp>max_metric) ? metric_tdd_ncp : max_metric;
  max_metric = (metric_tdd_ecp>max_metric) ? metric_tdd_ecp : max_metric;

  // frameware does not support sss, therefore, the frame params are hard coded to NCP=1 and TDD=1
#ifdef IFFT_FPGA
  max_metric = metric_tdd_ecp;
  Nid_cell_tdd_ecp = 0;
#endif

  if (max_metric == metric_fdd_ncp) {
    phy_vars_ue->lte_frame_parms.Ncp=0;
    phy_vars_ue->lte_frame_parms.frame_type=0;
    phy_vars_ue->lte_frame_parms.Nid_cell = Nid_cell_fdd_ncp;
    phy_vars_ue->lte_frame_parms.nushift  = Nid_cell_fdd_ncp%6;
    init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
    for (i=0;i<3;i++)
      lte_gold(frame_parms,phy_vars_ue->lte_gold_table[i],i);    

    sync_pos_slot = (frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - frame_parms->nb_prefix_samples;
    
    if (sync_pos >= sync_pos_slot)
      phy_vars_ue->rx_offset = sync_pos - sync_pos_slot;  
    else
      phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos - sync_pos_slot;
    msg("[PHY][UE%d] Initial sync : Found Cell ID %d for FDD Normal Prefix, rx_offset %d,sync_pos %d, sync_pos_slot %d\n",phy_vars_ue->Mod_id,Nid_cell_fdd_ncp,phy_vars_ue->rx_offset,sync_pos,sync_pos_slot);
  }
  else if (max_metric == metric_fdd_ecp) {
    phy_vars_ue->lte_frame_parms.Ncp=1;
    phy_vars_ue->lte_frame_parms.frame_type=0;
    phy_vars_ue->lte_frame_parms.Nid_cell = Nid_cell_fdd_ecp;
    phy_vars_ue->lte_frame_parms.nushift  = Nid_cell_fdd_ecp%6;
    init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
    for (i=0;i<3;i++)
      lte_gold(frame_parms,phy_vars_ue->lte_gold_table[i],i);    


    sync_pos_slot = (frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - frame_parms->nb_prefix_samples;
  
    if (sync_pos >= sync_pos_slot)
      phy_vars_ue->rx_offset = sync_pos - sync_pos_slot;  
    else
      phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos - sync_pos_slot;
    msg("[PHY][UE%d] Initial synch: Found Cell ID %d for FDD Extended Prefix, rx_offset %d,sync_pos %d, sync_pos_slot %d\n",phy_vars_ue->Mod_id,Nid_cell_fdd_ecp,phy_vars_ue->rx_offset,sync_pos,sync_pos_slot);
  }
  else if (max_metric == metric_tdd_ncp) {
    phy_vars_ue->lte_frame_parms.Ncp=0;
    phy_vars_ue->lte_frame_parms.frame_type=1;
    phy_vars_ue->lte_frame_parms.Nid_cell = Nid_cell_tdd_ncp;
    phy_vars_ue->lte_frame_parms.nushift  = Nid_cell_tdd_ncp%6;
    init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
    for (i=0;i<3;i++)
      lte_gold(frame_parms,phy_vars_ue->lte_gold_table[i],i);    



    sync_pos_slot = frame_parms->samples_per_tti + (frame_parms->ofdm_symbol_size<<1) + frame_parms->nb_prefix_samples0 + frame_parms->nb_prefix_samples;
  
    if (sync_pos >= sync_pos_slot)
      phy_vars_ue->rx_offset = sync_pos - sync_pos_slot;  
    else
      phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos - sync_pos_slot;
    msg("[PHY][UE%d] Initial sync : Found Cell ID %d for TDD Normal Prefix, rx_offset %d,sync_pos %d, sync_pos_slot %d\n",phy_vars_ue->Mod_id,Nid_cell_tdd_ncp,phy_vars_ue->rx_offset,sync_pos,sync_pos_slot);

  }
  else if (max_metric == metric_tdd_ecp) {
    phy_vars_ue->lte_frame_parms.Ncp=1;
    phy_vars_ue->lte_frame_parms.frame_type=1;
    phy_vars_ue->lte_frame_parms.Nid_cell = Nid_cell_tdd_ecp;
    phy_vars_ue->lte_frame_parms.nushift  = Nid_cell_tdd_ecp%6;
    init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
    for (i=0;i<3;i++)
      lte_gold(frame_parms,phy_vars_ue->lte_gold_table[i],i);    

    sync_pos_slot = frame_parms->samples_per_tti + (frame_parms->ofdm_symbol_size<<1) + frame_parms->nb_prefix_samples0 + frame_parms->nb_prefix_samples;
  
    if (sync_pos >= sync_pos_slot)
      phy_vars_ue->rx_offset = sync_pos - sync_pos_slot;  
    else
      phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos - sync_pos_slot;

    msg("[PHY][UE%d] Initial sync: Found Cell ID %d for TDD Extended Prefix, rx_offset %d,sync_pos %d, sync_pos_slot %d\n",phy_vars_ue->Mod_id,Nid_cell_tdd_ecp,phy_vars_ue->rx_offset,sync_pos,sync_pos_slot);

  }

  msg("[PHY][UE%d] Initial sync: (max_metric %d, metric_tdd_ecp %d)\n", phy_vars_ue->Mod_id, max_metric, metric_tdd_ecp);
  // Now do PBCH detection
  ret = pbch_detection(phy_vars_ue);
  if (ret==0) {
#ifdef OPENAIR2
    msg("[openair][SCHED][SYNCH] Sending synch status to higher layers\n");
    //mac_resynch();
    mac_xface->chbch_phy_sync_success(phy_vars_ue->Mod_id,0);//phy_vars_ue->lte_ue_common_vars.eNb_id);
#endif //OPENAIR2

    generate_pcfich_reg_mapping(frame_parms);
    generate_phich_reg_mapping(frame_parms);
    
    phy_vars_ue->UE_mode[0] = PRACH;
  }

  phy_adjust_gain(phy_vars_ue,0);
  
  return ret;
}
