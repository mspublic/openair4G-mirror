#ifndef USER_MODE
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#endif //USER_MODE

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "MAC_INTERFACE/extern.h"

extern int ulsch_errors[3],ulsch_decoding_attempts[3][4],dlsch_NAK[8],dlsch_trials[4];
extern int ulsch_round_errors[3][4];
extern int dlsch_l2_errors;

#ifndef USER_MODE
static struct proc_dir_entry *proc_openair1_root;
#endif 


#ifndef USER_MODE
static int openair1_state_read(char *buffer, char **my_buffer, off_t off, int length) {

  int len = 0;

  switch (openair_daq_vars.mode) {

  case openair_NOT_SYNCHED:
    len += sprintf(&buffer[len], "NOT IN SYNCH\n");
    break;
#ifdef OPENAIR_LTE
  case openair_SYNCHED:
    len += sprintf(&buffer[len], "SYNCHED\n");
    break;
#else
  case openair_SYNCHED_TO_CHSCH:
    len += sprintf(&buffer[len], "SYNCHED TO CH %d\n",openair_daq_vars.synch_source);
    break;
  case openair_SYNCHED_TO_MRSCH:
    len += sprintf(&buffer[len], "SYNCHED TO MR\n");
    break;
#endif
  case openair_SCHED_EXIT:
    len += sprintf(&buffer[len], "EXITED\n");
    break;
  }

}
#endif //USER_MODE

extern unsigned int current_dlsch_cqi;
extern unsigned int dlsch_fer;

#ifndef USER_MODE
static int chbch_stats_read(char *buffer, char **my_buffer, off_t off, int length)
#else
int chbch_stats_read(char *buffer, char **my_buffer, off_t off, int length)
#endif
{

  int len = 0,i,fg,eNB;
  /*
   * Get the current time and format it.
   */

  if (mac_xface->is_cluster_head == 0) {
    len += sprintf(&buffer[len], "[UE PROC] Frame count: %d\neNB0 RSSI %d dBm (%d dB, %d dB)\neNB1 RSSI %d dBm (%d dB, %d dB)\neNB2 RSSI %d dBm (%d dB, %d dB)\nN0 %d dBm (%d dB, %d dB)\n",
		   mac_xface->frame,
		   PHY_vars->PHY_measurements.rx_rssi_dBm[0],
		   PHY_vars->PHY_measurements.wideband_cqi_dB[0][0],
		   PHY_vars->PHY_measurements.wideband_cqi_dB[0][1],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[1],
		   PHY_vars->PHY_measurements.wideband_cqi_dB[1][0],
		   PHY_vars->PHY_measurements.wideband_cqi_dB[1][1],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[2],
		   PHY_vars->PHY_measurements.wideband_cqi_dB[2][0],
		   PHY_vars->PHY_measurements.wideband_cqi_dB[2][1],
		   PHY_vars->PHY_measurements.n0_power_tot_dBm,
		   PHY_vars->PHY_measurements.n0_power_dB[0],
		   PHY_vars->PHY_measurements.n0_power_dB[1]);
    len += sprintf(&buffer[len], "[UE PROC] RX Gain %d dB (rf_mode %d)\n",PHY_vars->rx_total_gain_dB, openair_daq_vars.rx_rf_mode);
    if (lte_ue_common_vars && dlsch_ue && dlsch_ue[0] && dlsch_ue[1]) {
      len += sprintf(&buffer[len], "[UE_PROC] Frequency offset %d Hz (%d)\n",lte_ue_common_vars->freq_offset,openair_daq_vars.freq_offset);
      len += sprintf(&buffer[len], "[UE PROC] UE mode = %s (%d)\n",mode_string[UE_mode],UE_mode);
      if (UE_mode == PUSCH)
	len += sprintf(&buffer[len], "[UE PROC] DLSCH FER %d, cqi = %d\n",dlsch_fer,current_dlsch_cqi);
      len += sprintf(&buffer[len], "[UE PROC] DL mcs1 (dlsch cw1) %d\n",dlsch_ue[0]->harq_processes[0]->mcs);
      len += sprintf(&buffer[len], "[UE PROC] DL mcs2 (dlsch cw2) %d\n",dlsch_ue[1]->harq_processes[0]->mcs);
    }
    len += sprintf(&buffer[len], "[UE PROC] timing_advance = %d\n",openair_daq_vars.timing_advance);


    //for (eNB=0;eNB<NUMBER_OF_eNB_MAX;eNB++) {
    for (eNB=0;eNB<1;eNB++) {
      len += sprintf(&buffer[len], "[UE PROC] RX spatial power eNB%d: [%d %d; %d %d] dB\n",
		     eNB,
		     PHY_vars->PHY_measurements.rx_spatial_power_dB[eNB][0][0],
		     PHY_vars->PHY_measurements.rx_spatial_power_dB[eNB][0][1],
		     PHY_vars->PHY_measurements.rx_spatial_power_dB[eNB][1][0],
		     PHY_vars->PHY_measurements.rx_spatial_power_dB[eNB][1][1]);
      
      len += sprintf(&buffer[len], "[UE PROC] Subband CQI eNB%d (Ant 0): [%d %d %d %d %d %d %d] dB\n",
		     eNB,
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][0],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][1],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][2],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][3],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][4],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][5],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][6]);
      
      len += sprintf(&buffer[len], "[UE PROC] Subband CQI eNB%d (Ant 1): [%d %d %d %d %d %d %d] dB\n",
		     eNB,
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][0],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][1],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][2],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][3],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][4],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][5],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][6]);

      
      len += sprintf(&buffer[len], "[UE PROC] Subband PMI eNB%d (Ant 0): [(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)]\n",
		     eNB,
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][0][0],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][0][0],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][1][0],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][1][0],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][2][0],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][2][0],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][3][0],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][3][0],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][4][0],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][4][0],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][5][0],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][5][0],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][6][0],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][6][0]);
      
      len += sprintf(&buffer[len], "[UE PROC] Subband PMI eNB%d (Ant 1): [(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)]\n",
		     eNB,
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][0][1],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][0][1],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][1][1],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][1][1],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][2][1],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][2][1],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][3][1],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][3][1],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][4][1],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][4][1],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][5][1],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][5][1],
		     PHY_vars->PHY_measurements.subband_pmi_re[eNB][6][1],
		     PHY_vars->PHY_measurements.subband_pmi_im[eNB][6][1]);
      
      len += sprintf(&buffer[len], "[UE PROC] PMI Antenna selection eNB%d : [%d %d %d %d %d %d %d]\n",
		     eNB,
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][0],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][1],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][2],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][3],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][4],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][5],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][6]);
      
      len += sprintf(&buffer[len], "[UE PROC] Wideband CQI eNB %d : %d dB\n",eNB,PHY_vars->PHY_measurements.wideband_cqi_tot[eNB]);
      len += sprintf(&buffer[len], "[UE PROC] Quantized PMI eNB %d (max): %x\n",eNB,pmi2hex_2Ar1(quantize_subband_pmi(&PHY_vars->PHY_measurements,eNB)));
      len += sprintf(&buffer[len], "[UE PROC] Quantized PMI eNB %d (both): %x,%x\n",eNB,
		     pmi2hex_2Ar1(quantize_subband_pmi2(&PHY_vars->PHY_measurements,eNB,0)),
		     pmi2hex_2Ar1(quantize_subband_pmi2(&PHY_vars->PHY_measurements,eNB,1)));


      len += sprintf(&buffer[len], "[UE PROC] Transmission Mode %d (mode1_flag %d)\n",openair_daq_vars.dlsch_transmission_mode,lte_frame_parms->mode1_flag);
      if (openair_daq_vars.dlsch_transmission_mode == 6)
	len += sprintf(&buffer[len], "[UE PROC] Mode 6 Wideband CQI eNB %d : %d dB\n",eNB,PHY_vars->PHY_measurements.precoded_cqi_dB[eNB][0]);
      if (dlsch_ue && dlsch_ue[0] && dlsch_ue[1]) 
	len += sprintf(&buffer[len], "[UE PROC] Saved PMI for DLSCH eNB %d : %x (%p)\n",eNB,pmi2hex_2Ar1(dlsch_ue[0]->pmi_alloc),dlsch_ue[0]);

      len += sprintf(&buffer[len], "[UE PROC] DLSCH FER : %d\n",dlsch_fer);
      
    }
  } // is_clusterhead

  else {
    for (eNB=0;eNB<number_of_cards;eNB++) {
      len += sprintf(&buffer[len],"[eNB PROC] eNB %d/%d Frame %d: RX Gain %d dB, I0 %d dBm (%d,%d) dB \n",
		     eNB,number_of_cards,
		     mac_xface->frame,
		     PHY_vars->rx_total_gain_eNB_dB,
		     PHY_vars->PHY_measurements_eNB[eNB].n0_power_tot_dBm,
		     PHY_vars->PHY_measurements_eNB[eNB].n0_power_dB[0],
		     PHY_vars->PHY_measurements_eNB[eNB].n0_power_dB[1]);

      len += sprintf(&buffer[len],"[eNB PROC] Subband I0: ");


      for (i=0;i<25;i++)
	len += sprintf(&buffer[len],"%2d ",
		       PHY_vars->PHY_measurements_eNB[eNB].n0_subband_power_tot_dB[i]);
      len += sprintf(&buffer[len],"\n");

      len += sprintf(&buffer[len],"[eNB PROC] Wideband CQI: (%d,%d) dB\n",
		     PHY_vars->PHY_measurements_eNB[eNB].wideband_cqi_dB[0][0],
		     PHY_vars->PHY_measurements_eNB[eNB].wideband_cqi_dB[0][1]);
      
      len += sprintf(&buffer[len],"[eNB PROC] Subband CQI: ");


      for (i=0;i<25;i++)
	len += sprintf(&buffer[len],"%2d ",
		       PHY_vars->PHY_measurements_eNB[eNB].subband_cqi_tot_dB[0][i]);
      len += sprintf(&buffer[len],"\n");
      
    }

    len += sprintf(&buffer[len],"[eNB PROC] UE 0 (%x) RSSI: (%d,%d) dBm, Sector %d, DLSCH Mode %d, DLSCH Rate adaptation %d, ULSCH Allocation mode %d, UE_DL_mcs %d, UE_UL_MCS %d, UE_UL_NB_RB %d\n",
		   eNB_UE_stats[0].UE_id[0],
		   eNB_UE_stats[0].UL_rssi[0][0],
		   eNB_UE_stats[0].UL_rssi[0][1],
		   eNB_UE_stats[0].sector[0],
		   openair_daq_vars.dlsch_transmission_mode,
		   openair_daq_vars.dlsch_rate_adaptation,
		   openair_daq_vars.ulsch_allocation_mode,
		   (openair_daq_vars.dlsch_rate_adaptation == 0) ? openair_daq_vars.target_ue_dl_mcs : ((eNB_UE_stats[0].DL_cqi[0][0]<<1)),
		   openair_daq_vars.target_ue_ul_mcs,
		   openair_daq_vars.ue_ul_nb_rb
		   );
      
    len += sprintf(&buffer[len],"[eNB PROC] DL_cqi %d, DL_pmi_single %x\n",
		   eNB_UE_stats[0].DL_cqi[0][0],
		   pmi2hex_2Ar1(eNB_UE_stats[0].DL_pmi_single[0]));
    len += sprintf(&buffer[len],"[eNB PROC] Timing advance %d samples (%d 16Ts)\n",
		   eNB_UE_stats[0].UE_timing_offset[0],
		   eNB_UE_stats[0].UE_timing_offset[0]>>2);
    len += sprintf(&buffer[len],"[eNB PROC] Mode = %s(%d)\n",
		   mode_string[eNB_UE_stats[0].mode[0]],
		   eNB_UE_stats[0].mode[0]);
    if (eNB_UE_stats[0].mode[0] == PUSCH) {
      len += sprintf(&buffer[len],"[eNB PROC] ULSCH errors (%d/%d (%d,%d,%d,%d) : %d/%d (%d,%d,%d,%d) : %d/%d(%d,%d,%d,%d) \n",
		     ulsch_errors[0],ulsch_decoding_attempts[0][0],
		     ulsch_round_errors[0][0],ulsch_round_errors[0][1],ulsch_round_errors[0][2],ulsch_round_errors[0][3],
		     ulsch_errors[1],ulsch_decoding_attempts[1][0],
		     ulsch_round_errors[1][0],ulsch_round_errors[1][1],ulsch_round_errors[1][2],ulsch_round_errors[1][3],
		     ulsch_errors[2],ulsch_decoding_attempts[2][0],
		     ulsch_round_errors[2][0],ulsch_round_errors[2][1],ulsch_round_errors[2][2],ulsch_round_errors[2][3]);
      len += sprintf(&buffer[len],"[eNB PROC] DLSCH errors %d/%d (%d/%d,%d/%d,%d/%d,%d/%d)\n",
		     dlsch_l2_errors,dlsch_trials[0],
		     dlsch_NAK[0],dlsch_trials[0],
		     dlsch_NAK[1],dlsch_trials[1],
		     dlsch_NAK[2],dlsch_trials[2],
		     dlsch_NAK[3],dlsch_trials[3]);

    }
  }

  return len;
}
/*
 * Initialize the module and add the /proc file.
 */
#ifndef USER_MODE
int add_openair1_stats(void)
{
 
  msg("Creating openair1 proc entry\n"); 
  proc_openair1_root = proc_mkdir("openair1",0);
  
  //  create_proc_info_entry("bch_stats", S_IFREG | S_IRUGO, proc_openair1_root, chbch_stats_read);
  //  create_proc_info_entry("openair1_state", S_IFREG | S_IRUGO, proc_openair1_root, openair1_state_read);
  create_proc_read_entry("bch_stats", S_IFREG | S_IRUGO, proc_openair1_root, (read_proc_t*)&chbch_stats_read,NULL);
  create_proc_read_entry("openair1_state", S_IFREG | S_IRUGO, proc_openair1_root, (read_proc_t*)&openair1_state_read,NULL);
  return 0;
}
/*
 * Unregister the file when the module is closed.
 */
void remove_openair_stats(void)
{

  if (proc_openair1_root) {
    printk("[OPENAIR][CLEANUP] Removing openair proc entry\n");
    remove_proc_entry("bch_stats", proc_openair1_root);
    remove_proc_entry("openair1_state", proc_openair1_root);
    remove_proc_entry("openair1",NULL);
  }
}
#endif