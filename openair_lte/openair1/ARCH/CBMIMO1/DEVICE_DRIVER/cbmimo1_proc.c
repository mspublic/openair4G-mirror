#ifndef USER_MODE
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#endif //USER_MODE

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/extern.h"

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
    len += sprintf(&buffer[len], "Frame count: %d\neNB0 RSSI %d dBm (%d dB, %d dB)\neNB1 RSSI %d dBm (%d dB, %d dB)\neNB2 RSSI %d dBm (%d dB, %d dB)\nN0 %d dBm (%d dB, %d dB)\n",
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
		   PHY_vars->PHY_measurements.n0_power_tot_dB-PHY_vars->rx_total_gain_dB,
		   PHY_vars->PHY_measurements.n0_power_dB[0],
		   PHY_vars->PHY_measurements.n0_power_dB[1]);
    len += sprintf(&buffer[len], "RX Gain %d dB\n",PHY_vars->rx_total_gain_dB);
    
    for (eNB=0;eNB<NUMBER_OF_eNB_MAX;eNB++) {
      len += sprintf(&buffer[len], "RX spatial power eNB%d: [%d %d; %d %d] dB\n",
		     eNB,
		     PHY_vars->PHY_measurements.rx_spatial_power_dB[eNB][0][0],
		     PHY_vars->PHY_measurements.rx_spatial_power_dB[eNB][0][1],
		     PHY_vars->PHY_measurements.rx_spatial_power_dB[eNB][1][0],
		     PHY_vars->PHY_measurements.rx_spatial_power_dB[eNB][1][1]);
      
      len += sprintf(&buffer[len], "Subband CQI eNB%d (Ant 0): [%d %d %d %d %d %d %d] dB\n",
		     eNB,
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][0],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][1],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][2],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][3],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][4],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][5],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][0][6]);
      
      len += sprintf(&buffer[len], "Subband CQI eNB%d (Ant 1): [%d %d %d %d %d %d %d] dB\n",
		     eNB,
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][0],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][1],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][2],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][3],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][4],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][5],
		     PHY_vars->PHY_measurements.subband_cqi_dB[eNB][1][6]);

      
      len += sprintf(&buffer[len], "Subband PMI eNB%d (Ant 0): [(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)] dB\n",
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
      
      len += sprintf(&buffer[len], "Subband PMI eNB%d (Ant 1): [(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)] dB\n",
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
      
      len += sprintf(&buffer[len], "PMI Antenna selection eNB%d : [%d %d %d %d %d %d %d]\n",
		     eNB,
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][0],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][1],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][2],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][3],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][4],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][5],
		     PHY_vars->PHY_measurements.selected_rx_antennas[eNB][6]);
      
      len += sprintf(&buffer[len], "Wideband CQI eNB %d : %d dB\n",eNB,PHY_vars->PHY_measurements.wideband_cqi_tot[eNB]);
      len += sprintf(&buffer[len], "Quantized PMI eNB %d : %x\n",eNB,pmi2hex_2Ar1(quantize_subband_pmi(&PHY_vars->PHY_measurements,eNB)));
      
      
    }
  } // is_clusterhead

  else {
    len += sprintf(&buffer[len],"\n\neNB 0 Frame %d : RX Gain %d dB\n",mac_xface->frame,PHY_vars->rx_total_gain_dB);
    len += sprintf(&buffer[len],"\n\neNB 0 Frame %d : UE 0 (%x) rssi (%d,%d) dBm\n",
		   mac_xface->frame,
		   eNB_UE_stats[0].UE_id[0],
		   eNB_UE_stats[0].UL_rssi[0][0],
		   eNB_UE_stats[0].UL_rssi[0][1]);
    len += sprintf(&buffer[len],"eNB 0 UE 0 (%x) DL_cqi %d, DL_pmi_single %x\n",
		   eNB_UE_stats[0].UE_id[0],
		   eNB_UE_stats[0].DL_cqi[0][0],
		   pmi2hex_2Ar1(eNB_UE_stats[0].DL_pmi_single[0]));
    len += sprintf(&buffer[len],"eNB 0 UE 0 (%x) Timing advance %d samples (%d 16Ts)\n",
		   eNB_UE_stats[0].UE_id[0],
		   eNB_UE_stats[0].UE_timing_offset[0],
		   eNB_UE_stats[0].UE_timing_offset[0]>>2);

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
