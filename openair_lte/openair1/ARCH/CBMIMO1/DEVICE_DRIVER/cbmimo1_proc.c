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

  int len = 0,i,fg,eNb;
  /*
   * Get the current time and format it.
   */
  if (mac_xface->is_cluster_head == 0) {
    
#ifndef OPENAIR_LTE
    len += sprintf(&buffer[len], "Frame count: %d\nCHSCH0 RSSI (%d dBm/ %d dB,%d dBm/ %d dB)\nCHSCH1 RSSI (%d dBm/ %d dB,%d dBm/ %d dB)\nCHSCH2 RSSI (%d dBm/ %d dB,%d dBm/ %d dB)\nCHSCH3 RSSI (%d dBm/ %d dB,%d dBm/ %d dB)\nN0 (%d dBm/ %d dB,%d dBm/ %d dB)\n",
		   mac_xface->frame,
		   PHY_vars->PHY_measurements.rx_rssi_dBm[0][0],
		   PHY_vars->PHY_measurements.rx_power_dB[0][0],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[0][1],
		   PHY_vars->PHY_measurements.rx_power_dB[0][1],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[1][0],
		   PHY_vars->PHY_measurements.rx_power_dB[1][0],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[1][1],
		   PHY_vars->PHY_measurements.rx_power_dB[1][1],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[2][0],
		   PHY_vars->PHY_measurements.rx_power_dB[2][0],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[2][1],
		   PHY_vars->PHY_measurements.rx_power_dB[2][1],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[3][0],
		   PHY_vars->PHY_measurements.rx_power_dB[3][0],
		   PHY_vars->PHY_measurements.rx_rssi_dBm[3][1],
		   PHY_vars->PHY_measurements.rx_power_dB[3][1],
		   PHY_vars->PHY_measurements.n0_power_dB[1][0]-PHY_vars->rx_vars[0].rx_total_gain_dB,
		   PHY_vars->PHY_measurements.n0_power_dB[1][0],
		   PHY_vars->PHY_measurements.n0_power_dB[1][1]-PHY_vars->rx_vars[0].rx_total_gain_dB,
		   PHY_vars->PHY_measurements.n0_power_dB[1][1]);
    len += sprintf(&buffer[len], "RX Gain %d dB\n",PHY_vars->rx_vars[0].rx_total_gain_dB);
#ifndef USER_MODE
    len += sprintf(&buffer[len], "Frequency band = %d\n",openair_daq_vars.freq);
#endif //USER_MODE
    for (i=1;i<4;i++) {
      len += sprintf(&buffer[len], "CHSCH %d Aggregate SINR per subband (dB) :",i);
      for (fg=0;fg<NUMBER_OF_FREQUENCY_GROUPS;fg++) {
	len += sprintf(&buffer[len],"%3d",PHY_vars->chsch_data[i].subband_aggregate_sinr[fg]);
      }
      len += sprintf(&buffer[len],"\n");
    } 
#ifndef USER_MODE
       if (openair_daq_vars.node_running == 1) {
	 if (dual_stream_flag==1)
	   len += sprintf(&buffer[len], "RX Mode: Dual stream receiver\n");
	 else
	   len += sprintf(&buffer[len], "RX Mode: Single stream receiver\n");
	 
	 len += sprintf(&buffer[len], "CHBCH1 errors: %d (%d %%)\nCHBCH2 errors: %d (%d %%)\nCHBCH3 errors: %d (%d %%)\n",
			PHY_vars->chbch_data[1].pdu_errors,
			PHY_vars->chbch_data[1].pdu_fer,
			PHY_vars->chbch_data[2].pdu_errors,
			PHY_vars->chbch_data[2].pdu_fer,
			PHY_vars->chbch_data[3].pdu_errors,
			PHY_vars->chbch_data[3].pdu_fer);
       }
       else {
	 len += sprintf(&buffer[len], "CHBCH1 detection count: (%d/%d)\nCHBCH2 detection count: (%d/%d)\nCHBCH3 detection count: (%d/%d)\n",
			PHY_vars->PHY_measurements.chbch_detection_count[1],
			PHY_vars->PHY_measurements.chbch_search_count,
			PHY_vars->PHY_measurements.chbch_detection_count[2],
			PHY_vars->PHY_measurements.chbch_search_count,
			PHY_vars->PHY_measurements.chbch_detection_count[3],
			PHY_vars->PHY_measurements.chbch_search_count);

	 len += sprintf(&buffer[len], "TTI: %d\nMRSCH RSSI (%d dBm,%d dBm)\n",
			mac_xface->frame,
			PHY_vars->PHY_measurements.rx_rssi_dBm[MRSCH_INDEX][0],
			PHY_vars->PHY_measurements.rx_rssi_dBm[MRSCH_INDEX][1]);
	 
	 len += sprintf(&buffer[len], "MRBCH detection count: (%d/%d)\n",
			PHY_vars->PHY_measurements.mrbch_detection_count,
			PHY_vars->PHY_measurements.mrbch_search_count);
       }
#endif //USER_MODE

#else //OPENAIR_LTE
       len += sprintf(&buffer[len], "Frame count: %d\neNb0 RSSI %d dBm (%d dB, %d dB)\neNb1 RSSI %d dBm (%d dB, %d dB)\neNb2 RSSI %d dBm (%d dB, %d dB)\nN0 %d dBm (%d dB, %d dB)\n",
		      mac_xface->frame,
		      PHY_vars->PHY_measurements.rx_rssi_dBm[0],
		      PHY_vars->PHY_measurements.rx_power_dB[0][0],
		      PHY_vars->PHY_measurements.rx_power_dB[0][1],
		      PHY_vars->PHY_measurements.rx_rssi_dBm[1],
		      PHY_vars->PHY_measurements.rx_power_dB[1][0],
		      PHY_vars->PHY_measurements.rx_power_dB[1][1],
		      PHY_vars->PHY_measurements.rx_rssi_dBm[2],
		      PHY_vars->PHY_measurements.rx_power_dB[2][0],
		      PHY_vars->PHY_measurements.rx_power_dB[2][1],
		      PHY_vars->PHY_measurements.n0_avg_power_dB-PHY_vars->rx_vars[0].rx_total_gain_dB,
		      PHY_vars->PHY_measurements.n0_power_dB[0],
		      PHY_vars->PHY_measurements.n0_power_dB[1]);
       len += sprintf(&buffer[len], "RX Gain %d dB\n",PHY_vars->rx_vars[0].rx_total_gain_dB);

       for (eNb=0;eNb<NUMBER_OF_eNB_MAX;eNb++) {
	 len += sprintf(&buffer[len], "RX spatial power eNb%d: [%d %d; %d %d] dB\n",
			eNb,
			PHY_vars->PHY_measurements.rx_spatial_power_dB[eNb][0][0],
			PHY_vars->PHY_measurements.rx_spatial_power_dB[eNb][0][1],
			PHY_vars->PHY_measurements.rx_spatial_power_dB[eNb][1][0],
			PHY_vars->PHY_measurements.rx_spatial_power_dB[eNb][1][1]);
	 
	 len += sprintf(&buffer[len], "RX correlation eNb%d: [%d %d] dB\n",
			eNb,
			PHY_vars->PHY_measurements.rx_correlation_dB[eNb][0],
			PHY_vars->PHY_measurements.rx_correlation_dB[eNb][1]);
       }
       /*
       
#ifndef USER_MODE
       len += sprintf(&buffer[len], "Frequency band = %d\n",openair_daq_vars.freq);
#endif //USER_MODE
       
       */

#endif //OPENAIR_LTE	 
      
    } // is_clusterhead
#ifndef USER_MODE
    else {

#ifndef OPENAIR_LTE
      if (openair_daq_vars.node_running == 1)
         len += sprintf(&buffer[len], "\n\nCH TTI: %d  MRSCH RSSI (%d dBm,%d dBm), RX Gain %d dB\n",
                        mac_xface->frame,
                        PHY_vars->PHY_measurements.rx_rssi_dBm[MRSCH_INDEX][0],
                        PHY_vars->PHY_measurements.rx_rssi_dBm[MRSCH_INDEX][1],
			PHY_vars->rx_vars[0].rx_total_gain_dB);
	 len += sprintf(&buffer[len], "MRBCH errors: %d (%d %%)\n",
			PHY_vars->mrbch_data[0].pdu_errors,
			PHY_vars->mrbch_data[0].pdu_fer);
#endif //OPENAIR_LTE      
    }
#endif //USER_MODE


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
