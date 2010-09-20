#include "PHY/defs.h"
#include "PHY/extern.h"

extern u8 number_of_cards;

void dump_ue_stats(PHY_VARS_UE *phy_vars_ue) {
  char buffer[2048];
  u8 eNB;
  u16 len;

  len += sprintf(&buffer[len], "[UE PROC] Frame count: %d\neNB0 RSSI %d dBm (%d dB, %d dB)\neNB1 RSSI %d dBm (%d dB, %d dB)\neNB2 RSSI %d dBm (%d dB, %d dB)\nN0 %d dBm (%d dB, %d dB)\n",
		 mac_xface->frame,
		 phy_vars_ue->PHY_measurements.rx_rssi_dBm[0],
		 phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][0],
		 phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][1],
		 phy_vars_ue->PHY_measurements.rx_rssi_dBm[1],
		 phy_vars_ue->PHY_measurements.wideband_cqi_dB[1][0],
		 phy_vars_ue->PHY_measurements.wideband_cqi_dB[1][1],
		 phy_vars_ue->PHY_measurements.rx_rssi_dBm[2],
		 phy_vars_ue->PHY_measurements.wideband_cqi_dB[2][0],
		 phy_vars_ue->PHY_measurements.wideband_cqi_dB[2][1],
		 phy_vars_ue->PHY_measurements.n0_power_tot_dBm,
		 phy_vars_ue->PHY_measurements.n0_power_dB[0],
		 phy_vars_ue->PHY_measurements.n0_power_dB[1]);
  len += sprintf(&buffer[len], "[UE PROC] RX Gain %d dB (rf_mode %d)\n",phy_vars_ue->rx_total_gain_dB, openair_daq_vars.rx_rf_mode);
  if (phy_vars_ue->dlsch_ue[0] && phy_vars_ue->dlsch_ue[0][0] && phy_vars_ue->dlsch_ue[0][1]) {
    len += sprintf(&buffer[len], "[UE_PROC] Frequency offset %d Hz (%d)\n",phy_vars_ue->lte_ue_common_vars.freq_offset,openair_daq_vars.freq_offset);
    len += sprintf(&buffer[len], "[UE PROC] UE mode = %s (%d)\n",mode_string[phy_vars_ue->UE_mode[0]],phy_vars_ue->UE_mode);
    if (phy_vars_ue->UE_mode == PUSCH)
      len += sprintf(&buffer[len], "[UE PROC] DLSCH FER %d, cqi = %d\n",phy_vars_ue->dlsch_fer,phy_vars_ue->current_dlsch_cqi);
    len += sprintf(&buffer[len], "[UE PROC] DL mcs1 (dlsch cw1) %d\n",phy_vars_ue->dlsch_ue[0][0]->harq_processes[0]->mcs);
    len += sprintf(&buffer[len], "[UE PROC] DL mcs2 (dlsch cw2) %d\n",phy_vars_ue->dlsch_ue[0][1]->harq_processes[0]->mcs);
  }
  len += sprintf(&buffer[len], "[UE PROC] timing_advance = %d\n",openair_daq_vars.timing_advance);
  
  
  //for (eNB=0;eNB<NUMBER_OF_eNB_MAX;eNB++) {
  for (eNB=0;eNB<1;eNB++) {
    len += sprintf(&buffer[len], "[UE PROC] RX spatial power eNB%d: [%d %d; %d %d] dB\n",
		   eNB,
		   phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB][0][0],
		   phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB][0][1],
		   phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB][1][0],
		   phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB][1][1]);
    
    len += sprintf(&buffer[len], "[UE PROC] Subband CQI eNB%d (Ant 0): [%d %d %d %d %d %d %d] dB\n",
		   eNB,
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][0][0],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][0][1],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][0][2],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][0][3],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][0][4],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][0][5],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][0][6]);
    
    len += sprintf(&buffer[len], "[UE PROC] Subband CQI eNB%d (Ant 1): [%d %d %d %d %d %d %d] dB\n",
		   eNB,
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][1][0],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][1][1],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][1][2],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][1][3],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][1][4],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][1][5],
		   phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB][1][6]);
    
    
    len += sprintf(&buffer[len], "[UE PROC] Subband PMI eNB%d (Ant 0): [(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)]\n",
		   eNB,
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][0][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][0][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][1][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][1][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][2][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][2][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][3][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][3][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][4][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][4][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][5][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][5][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][6][0],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][6][0]);
    
    len += sprintf(&buffer[len], "[UE PROC] Subband PMI eNB%d (Ant 1): [(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)]\n",
		   eNB,
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][0][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][0][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][1][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][1][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][2][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][2][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][3][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][3][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][4][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][4][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][5][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][5][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_re[eNB][6][1],
		   phy_vars_ue->PHY_measurements.subband_pmi_im[eNB][6][1]);
    
    len += sprintf(&buffer[len], "[UE PROC] PMI Antenna selection eNB%d : [%d %d %d %d %d %d %d]\n",
		   eNB,
		   phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB][0],
		   phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB][1],
		   phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB][2],
		   phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB][3],
		   phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB][4],
		   phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB][5],
		   phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB][6]);
    
    len += sprintf(&buffer[len], "[UE PROC] Wideband CQI eNB %d : %d dB\n",eNB,phy_vars_ue->PHY_measurements.wideband_cqi_tot[eNB]);
    len += sprintf(&buffer[len], "[UE PROC] Quantized PMI eNB %d (max): %x\n",eNB,pmi2hex_2Ar1(quantize_subband_pmi(&phy_vars_ue->PHY_measurements,eNB)));
    len += sprintf(&buffer[len], "[UE PROC] Quantized PMI eNB %d (both): %x,%x\n",eNB,
		   pmi2hex_2Ar1(quantize_subband_pmi2(&phy_vars_ue->PHY_measurements,eNB,0)),
		   pmi2hex_2Ar1(quantize_subband_pmi2(&phy_vars_ue->PHY_measurements,eNB,1)));
    
    
    len += sprintf(&buffer[len], "[UE PROC] Transmission Mode %d (mode1_flag %d)\n",openair_daq_vars.dlsch_transmission_mode,phy_vars_ue->lte_frame_parms.mode1_flag);
    if (openair_daq_vars.dlsch_transmission_mode == 6)
      len += sprintf(&buffer[len], "[UE PROC] Mode 6 Wideband CQI eNB %d : %d dB\n",eNB,phy_vars_ue->PHY_measurements.precoded_cqi_dB[eNB][0]);
    if (phy_vars_ue->dlsch_ue[0] && phy_vars_ue->dlsch_ue[0][0] && phy_vars_ue->dlsch_ue[0][1]) 
      len += sprintf(&buffer[len], "[UE PROC] Saved PMI for DLSCH eNB %d : %x (%p)\n",eNB,pmi2hex_2Ar1(phy_vars_ue->dlsch_ue[0][0]->pmi_alloc),phy_vars_ue->dlsch_ue[0][0]);
    
    len += sprintf(&buffer[len], "[UE PROC] DLSCH FER : %d\n",phy_vars_ue->dlsch_fer);
    
  }
  buffer[len]='\0';
  puts(buffer);
} // is_clusterhead

void dump_eNB_stats(PHY_VARS_eNB *phy_vars_eNb) {

  char buffer[2048];  
  u8 eNB,i;
  u16 len;

  for (eNB=0;eNB<number_of_cards;eNB++) {
    len += sprintf(&buffer[len],"[eNB PROC] eNB %d/%d Frame %d: RX Gain %d dB, I0 %d dBm (%d,%d) dB \n",
		   eNB,number_of_cards,
		   mac_xface->frame,
		   phy_vars_eNb->rx_total_gain_eNB_dB,
		   phy_vars_eNb->PHY_measurements_eNB[eNB].n0_power_tot_dBm,
		   phy_vars_eNb->PHY_measurements_eNB[eNB].n0_power_dB[0],
		   phy_vars_eNb->PHY_measurements_eNB[eNB].n0_power_dB[1]);
    
    len += sprintf(&buffer[len],"[eNB PROC] Subband I0: ");
    
    
    for (i=0;i<25;i++)
      len += sprintf(&buffer[len],"%2d ",
		     phy_vars_eNb->PHY_measurements_eNB[eNB].n0_subband_power_tot_dB[i]);
    len += sprintf(&buffer[len],"\n");
    
    len += sprintf(&buffer[len],"[eNB PROC] Wideband CQI: (%d,%d) dB\n",
		   phy_vars_eNb->PHY_measurements_eNB[eNB].wideband_cqi_dB[0][0],
		   phy_vars_eNb->PHY_measurements_eNB[eNB].wideband_cqi_dB[0][1]);
    
    len += sprintf(&buffer[len],"[eNB PROC] Subband CQI: ");
    
    
    for (i=0;i<25;i++)
      len += sprintf(&buffer[len],"%2d ",
		     phy_vars_eNb->PHY_measurements_eNB[eNB].subband_cqi_tot_dB[0][i]);
    len += sprintf(&buffer[len],"\n");
    
  }
  
  len += sprintf(&buffer[len],"[eNB PROC] UE 0 (%x) RSSI: (%d,%d) dBm, Sector %d, DLSCH Mode %d, DLSCH Rate adaptation %d, ULSCH Allocation mode %d, UE_DL_mcs %d, UE_UL_MCS %d, UE_UL_NB_RB %d\n",
		 phy_vars_eNb->eNB_UE_stats[0].crnti,
		 phy_vars_eNb->eNB_UE_stats[0].UL_rssi[0],
		 phy_vars_eNb->eNB_UE_stats[0].UL_rssi[1],
		 phy_vars_eNb->eNB_UE_stats[0].sector,
		 openair_daq_vars.dlsch_transmission_mode,
		 openair_daq_vars.dlsch_rate_adaptation,
		 openair_daq_vars.ulsch_allocation_mode,
		 (openair_daq_vars.dlsch_rate_adaptation == 0) ? openair_daq_vars.target_ue_dl_mcs : ((phy_vars_eNb->eNB_UE_stats[0].DL_cqi[0]<<1)),
		 openair_daq_vars.target_ue_ul_mcs,
		 openair_daq_vars.ue_ul_nb_rb
		 );
  
  len += sprintf(&buffer[len],"[eNB PROC] DL_cqi %d, DL_pmi_single %x\n",
		 phy_vars_eNb->eNB_UE_stats[0].DL_cqi[0],
		 pmi2hex_2Ar1(phy_vars_eNb->eNB_UE_stats[0].DL_pmi_single));
  len += sprintf(&buffer[len],"[eNB PROC] Timing advance %d samples (%d 16Ts)\n",
		 phy_vars_eNb->eNB_UE_stats[0].UE_timing_offset,
		 phy_vars_eNb->eNB_UE_stats[0].UE_timing_offset>>2);
  len += sprintf(&buffer[len],"[eNB PROC] Mode = %s(%d)\n",
		 mode_string[phy_vars_eNb->eNB_UE_stats[0].mode],
		 phy_vars_eNb->eNB_UE_stats[0].mode);
  if (phy_vars_eNb->eNB_UE_stats[0].mode == PUSCH) {
    len += sprintf(&buffer[len],"[eNB PROC] ULSCH errors (%d/%d (%d,%d,%d,%d) : %d/%d (%d,%d,%d,%d) : %d/%d(%d,%d,%d,%d) \n",
		   phy_vars_eNb->eNB_UE_stats[0].ulsch_errors[0],phy_vars_eNb->eNB_UE_stats[0].ulsch_decoding_attempts[0][0],
		   phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[0][0],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[0][1],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[0][2],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[0][3],
		   phy_vars_eNb->eNB_UE_stats[0].ulsch_errors[1],phy_vars_eNb->eNB_UE_stats[0].ulsch_decoding_attempts[1][0],
		   phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[1][0],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[1][1],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[1][2],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[1][3],
		   phy_vars_eNb->eNB_UE_stats[0].ulsch_errors[2],phy_vars_eNb->eNB_UE_stats[0].ulsch_decoding_attempts[2][0],
		   phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[2][0],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[2][1],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[2][2],phy_vars_eNb->eNB_UE_stats[0].ulsch_round_errors[2][3]);
    len += sprintf(&buffer[len],"[eNB PROC] DLSCH errors %d/%d (%d/%d,%d/%d,%d/%d,%d/%d)\n",
		   phy_vars_eNb->eNB_UE_stats[0].dlsch_l2_errors,phy_vars_eNb->eNB_UE_stats[0].dlsch_trials[0],
		   phy_vars_eNb->eNB_UE_stats[0].dlsch_NAK[0],phy_vars_eNb->eNB_UE_stats[0].dlsch_trials[0],
		   phy_vars_eNb->eNB_UE_stats[0].dlsch_NAK[1],phy_vars_eNb->eNB_UE_stats[0].dlsch_trials[1],
		   phy_vars_eNb->eNB_UE_stats[0].dlsch_NAK[2],phy_vars_eNb->eNB_UE_stats[0].dlsch_trials[2],
		   phy_vars_eNb->eNB_UE_stats[0].dlsch_NAK[3],phy_vars_eNb->eNB_UE_stats[0].dlsch_trials[3]);
    
  }
  buffer[len]='\0';
  puts(buffer);
}
