/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/print_stats.c
* \brief PHY statstic logging function
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
* \note
* \warning
*/

#include "PHY/LTE_TRANSPORT/proto.h"

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/extern.h"



extern int mac_get_rrc_status(uint8_t Mod_id,uint8_t eNB_flag,uint8_t index);

#ifdef EXMIMO
#ifdef DRIVER2013
#include "openair0_lib.h"
extern int card;

#else
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#endif
#endif

int dump_ue_stats(PHY_VARS_UE *phy_vars_ue, char* buffer, int length, runmode_t mode, int input_level_dBm) {

  uint8_t eNB=0;
  uint32_t RRC_status;
  int len=length;
#ifdef EXMIMO
#ifdef DRIVER2013
  exmimo_config_t *p_exmimo_config = openair0_exmimo_pci[card].exmimo_config_ptr;
#endif
#endif

  if (phy_vars_ue==NULL)
    return 0;

  if ((mode == normal_txrx) || (mode == no_L2_connect)) {
  len += sprintf(&buffer[len], "[UE_PROC] UE %d, RNTI %x\n",phy_vars_ue->Mod_id, phy_vars_ue->lte_ue_pdcch_vars[0]->crnti);
  len += sprintf(&buffer[len], "[UE PROC] Frame count: %d\neNB0 RSSI %d dBm (%d dB, %d dB)\neNB1 RSSI %d dBm (%d dB, %d dB)\neNB2 RSSI %d dBm (%d dB, %d dB)\nN0 %d dBm (%d dB, %d dB)\n",
		 phy_vars_ue->frame,
		 phy_vars_ue->PHY_measurements.rx_rssi_dBm[0],
		 phy_vars_ue->PHY_measurements.rx_power_dB[0][0],
		 phy_vars_ue->PHY_measurements.rx_power_dB[0][1],
		 phy_vars_ue->PHY_measurements.rx_rssi_dBm[1],
		 phy_vars_ue->PHY_measurements.rx_power_dB[1][0],
		 phy_vars_ue->PHY_measurements.rx_power_dB[1][1],
		 phy_vars_ue->PHY_measurements.rx_rssi_dBm[2],
		 phy_vars_ue->PHY_measurements.rx_power_dB[2][0],
		 phy_vars_ue->PHY_measurements.rx_power_dB[2][1],
		 phy_vars_ue->PHY_measurements.n0_power_tot_dBm,
		 phy_vars_ue->PHY_measurements.n0_power_dB[0],
		 phy_vars_ue->PHY_measurements.n0_power_dB[1]);
#ifdef CBMIMO1
    len += sprintf(&buffer[len], "[UE PROC] RX Gain %d dB (rf_mode %d)\n",phy_vars_ue->rx_total_gain_dB, openair_daq_vars.rx_rf_mode);
#else
#ifdef EXMIMO
#ifdef DRIVER2013
    len += sprintf(&buffer[len], "[UE PROC] RX Gain %d dB (rf_mode %d, vga %d dB)\n",phy_vars_ue->rx_total_gain_dB, phy_vars_ue->rx_gain_mode[0],p_exmimo_config->rf.rx_gain[0][0]);
#else
    len += sprintf(&buffer[len], "[UE PROC] RX Gain %d dB (rf_mode %d, vga %d dB)\n",phy_vars_ue->rx_total_gain_dB, phy_vars_ue->rx_gain_mode[0],exmimo_pci_interface->rf.rx_gain00);
#endif
#else
    len += sprintf(&buffer[len], "[UE PROC] RX Gain %d dB\n",phy_vars_ue->rx_total_gain_dB);
#endif
#endif
  len += sprintf(&buffer[len], "[UE_PROC] Frequency offset %d Hz (%d)\n",phy_vars_ue->lte_ue_common_vars.freq_offset,openair_daq_vars.freq_offset);
  len += sprintf(&buffer[len], "[UE PROC] UE mode = %s (%d)\n",mode_string[phy_vars_ue->UE_mode[0]],phy_vars_ue->UE_mode[0]);
  len += sprintf(&buffer[len], "[UE PROC] timing_advance = %d\n",phy_vars_ue->timing_advance);
  len += sprintf(&buffer[len], "[UE PROC] UE tx power = %d\n", PHY_vars_UE_g[0]->tx_power_dBm);  
  
  //for (eNB=0;eNB<NUMBER_OF_eNB_MAX;eNB++) {
  for (eNB=0;eNB<1;eNB++) {
    len += sprintf(&buffer[len], "[UE PROC] RX spatial power eNB%d: [%d %d; %d %d] dB\n",
		   eNB,
		   phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB][0][0],
		   phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB][0][1],
		   phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB][1][0],
		   phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB][1][1]);

    len += sprintf(&buffer[len], "[UE PROC] RX total power eNB%d: %d dB, avg: %d dB\n",eNB,phy_vars_ue->PHY_measurements.rx_power_tot_dB[eNB],phy_vars_ue->PHY_measurements.rx_power_avg_dB[eNB]);
    len += sprintf(&buffer[len], "[UE PROC] RX total power lin: %d, avg: %d, RX total noise lin: %d, avg: %d\n",phy_vars_ue->PHY_measurements.rx_power_tot[eNB], phy_vars_ue->PHY_measurements.rx_power_avg[eNB], phy_vars_ue->PHY_measurements.n0_power_tot, phy_vars_ue->PHY_measurements.n0_power_avg);
    len += sprintf(&buffer[len], "[UE PROC] effective SINR %.2f dB\n",phy_vars_ue->sinr_eff);
    len += sprintf(&buffer[len], "[UE PROC] Wideband CQI eNB %d: %d dB, avg: %d dB\n",eNB,phy_vars_ue->PHY_measurements.wideband_cqi_tot[eNB],phy_vars_ue->PHY_measurements.wideband_cqi_avg[eNB]);

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
    
    len += sprintf(&buffer[len], "[UE PROC] Quantized PMI eNB %d (max): %x\n",eNB,pmi2hex_2Ar1(quantize_subband_pmi(&phy_vars_ue->PHY_measurements,eNB)));
    len += sprintf(&buffer[len], "[UE PROC] Quantized PMI eNB %d (both): %x,%x\n",eNB,
		   pmi2hex_2Ar1(quantize_subband_pmi2(&phy_vars_ue->PHY_measurements,eNB,0)),
		   pmi2hex_2Ar1(quantize_subband_pmi2(&phy_vars_ue->PHY_measurements,eNB,1)));

#ifdef OPENAIR2    
      RRC_status = mac_get_rrc_status(phy_vars_ue->Mod_id,0,0);
      len += sprintf(&buffer[len],"[UE PROC] RRC status = %d\n",RRC_status);
#endif
      len += sprintf(&buffer[len],"[UE PROC] RSRP[0] %d, RSSI %d, RSRQ[0] %d\n",phy_vars_ue->PHY_measurements.rsrp[0], phy_vars_ue->PHY_measurements.rssi, phy_vars_ue->PHY_measurements.rsrq[0]);
    
    len += sprintf(&buffer[len], "[UE PROC] Transmission Mode %d (mode1_flag %d)\n",phy_vars_ue->transmission_mode[eNB],phy_vars_ue->lte_frame_parms.mode1_flag);
    if (phy_vars_ue->transmission_mode[eNB] == 6)
      len += sprintf(&buffer[len], "[UE PROC] Mode 6 Wideband CQI eNB %d : %d dB\n",eNB,phy_vars_ue->PHY_measurements.precoded_cqi_dB[eNB][0]);
    if (phy_vars_ue->dlsch_ue[0] && phy_vars_ue->dlsch_ue[0][0] && phy_vars_ue->dlsch_ue[0][1]) {
      len += sprintf(&buffer[len], "[UE PROC] Saved PMI for DLSCH eNB %d : %x (%p)\n",eNB,pmi2hex_2Ar1(phy_vars_ue->dlsch_ue[0][0]->pmi_alloc),phy_vars_ue->dlsch_ue[0][0]);

      len += sprintf(&buffer[len], "[UE PROC] eNB %d: dl_power_off = %d\n",eNB,phy_vars_ue->dlsch_ue[0][0]->harq_processes[0]->dl_power_off);

      len += sprintf(&buffer[len], "[UE PROC] DL mcs1 (dlsch cw1) %d\n",phy_vars_ue->dlsch_ue[0][0]->harq_processes[0]->mcs);
      len += sprintf(&buffer[len], "[UE PROC] DL mcs2 (dlsch cw2) %d\n",phy_vars_ue->dlsch_ue[0][1]->harq_processes[0]->mcs);
    }
    len += sprintf(&buffer[len], "[UE PROC] DLSCH Total %d, Error %d, FER %d\n",phy_vars_ue->dlsch_received[0],phy_vars_ue->dlsch_errors[0],phy_vars_ue->dlsch_fer[0]);
    len += sprintf(&buffer[len], "[UE PROC] DLSCH (SI) Total %d, Error %d\n",phy_vars_ue->dlsch_SI_received[0],phy_vars_ue->dlsch_SI_errors[0]);
    len += sprintf(&buffer[len], "[UE PROC] DLSCH (RA) Total %d, Error %d\n",phy_vars_ue->dlsch_ra_received[0],phy_vars_ue->dlsch_ra_errors[0]);
#ifdef Rel10
    int i=0;
    //len += sprintf(&buffer[len], "[UE PROC] MCH  Total %d\n", phy_vars_ue->dlsch_mch_received[0]);
     for(i=0; i <phy_vars_ue->lte_frame_parms.num_MBSFN_config; i++ ){
      len += sprintf(&buffer[len], "[UE PROC] MCH (MCCH MBSFN %d) Total %d, Error %d, Trials %d\n",
		     i, phy_vars_ue->dlsch_mcch_received[i][0],phy_vars_ue->dlsch_mcch_errors[i][0],phy_vars_ue->dlsch_mcch_trials[i][0]);   
      len += sprintf(&buffer[len], "[UE PROC] MCH (MTCH MBSFN %d) Total %d, Error %d, Trials %d\n",
		     i, phy_vars_ue->dlsch_mtch_received[i][0],phy_vars_ue->dlsch_mtch_errors[i][0],phy_vars_ue->dlsch_mtch_trials[i][0]);  
    }
#endif 
    len += sprintf(&buffer[len], "[UE PROC] DLSCH Bitrate %dkbps\n",(phy_vars_ue->bitrate[0]/1000));
    len += sprintf(&buffer[len], "[UE PROC] Total Received Bits %dkbits\n",(phy_vars_ue->total_received_bits[0]/1000));
    len += sprintf(&buffer[len], "[UE PROC] IA receiver %d\n",openair_daq_vars.use_ia_receiver);

  }

  }
  else {
    len += sprintf(&buffer[len], "[UE PROC] Frame count: %d, RSSI %3.2f dB (%d dB, %d dB), N0 %3.2f dB (%d dB, %d dB)\n",
		   phy_vars_ue->frame,
		   10*log10(phy_vars_ue->PHY_measurements.rssi),
		   phy_vars_ue->PHY_measurements.rx_power_dB[0][0],
		   phy_vars_ue->PHY_measurements.rx_power_dB[0][1],
		   10*log10(phy_vars_ue->PHY_measurements.n0_power_tot),
		   phy_vars_ue->PHY_measurements.n0_power_dB[0],
		   phy_vars_ue->PHY_measurements.n0_power_dB[1]);
#ifdef EXMIMO
    phy_vars_ue->rx_total_gain_dB = ((int)(10*log10(phy_vars_ue->PHY_measurements.rssi)))-input_level_dBm;
    len += sprintf(&buffer[len], "[UE PROC] rf_mode %d, input level (set by user) %d dBm, VGA gain %d dB ==> total gain %3.2f dB, noise figure %3.2f dB\n",
		   phy_vars_ue->rx_gain_mode[0],
		   input_level_dBm, 
#ifdef DRIVER2013
		   p_exmimo_config->rf.rx_gain[0][0],
#else
		   exmimo_pci_interface->rf.rx_gain00,
#endif
		   10*log10(phy_vars_ue->PHY_measurements.rssi)-input_level_dBm,
		   10*log10(phy_vars_ue->PHY_measurements.n0_power_tot)-phy_vars_ue->rx_total_gain_dB+105);
#endif
  }

  len += sprintf(&buffer[len],"EOF\n");
  len += sprintf(&buffer[len],"\0");

  return len;
} // is_clusterhead

int dump_eNB_stats(PHY_VARS_eNB *phy_vars_eNB, char* buffer, int length) {

  unsigned int success=0;
  uint8_t eNB,UE_id,i,j,number_of_cards_l=1;
  uint32_t ulsch_errors=0;
  uint32_t ulsch_round_attempts[4]={0,0,0,0},ulsch_round_errors[4]={0,0,0,0};
  uint32_t harq_pid_ul, harq_pid_dl;
  uint32_t UE_id_mac, RRC_status;
  if (phy_vars_eNB==NULL)
    return 0;

  int len = length;

  //  if(phy_vars_eNB->frame==0){
  phy_vars_eNB->total_dlsch_bitrate = 0;//phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_bitrate + phy_vars_eNB->total_dlsch_bitrate;
  phy_vars_eNB->total_transmitted_bits = 0;// phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits +  phy_vars_eNB->total_transmitted_bits;
  phy_vars_eNB->total_system_throughput = 0;//phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits + phy_vars_eNB->total_system_throughput;
  // }

  for (eNB=0;eNB<number_of_cards_l;eNB++) {
    len += sprintf(&buffer[len],"[eNB PROC] eNB %d/%d Frame %d: RX Gain %d dB, I0 %d dBm (%d,%d) dB \n",
		   eNB,number_of_cards_l,
		   phy_vars_eNB->frame,
		   phy_vars_eNB->rx_total_gain_eNB_dB,
		   phy_vars_eNB->PHY_measurements_eNB[eNB].n0_power_tot_dBm,
		   phy_vars_eNB->PHY_measurements_eNB[eNB].n0_power_dB[0],
		   phy_vars_eNB->PHY_measurements_eNB[eNB].n0_power_dB[1]);
    
    len += sprintf(&buffer[len],"[eNB PROC] Subband I0: ");
    for (i=0;i<25;i++)
      len += sprintf(&buffer[len],"%2d ",
		     phy_vars_eNB->PHY_measurements_eNB[eNB].n0_subband_power_tot_dB[i]);
    len += sprintf(&buffer[len],"\n");
    len += sprintf(&buffer[len],"\n[eNB PROC] PERFORMANCE PARAMETERS\n");
    /*
    len += sprintf(&buffer[len],"[eNB PROC] Total DLSCH Bitrate for the System %dkbps\n",((phy_vars_eNB->eNB_UE_stats[0].dlsch_bitrate + phy_vars_eNB->eNB_UE_stats[1].dlsch_bitrate)/1000));
    len += sprintf(&buffer[len],"[eNB PROC] Total Bits successfully transitted %dKbits in %dframe(s)\n",((phy_vars_eNB->eNB_UE_stats[0].total_transmitted_bits + phy_vars_eNB->eNB_UE_stats[1].total_transmitted_bits)/1000),phy_vars_eNB->frame+1);
    len += sprintf(&buffer[len],"[eNB PROC] Average System Throughput %dKbps\n",(phy_vars_eNB->eNB_UE_stats[0].total_transmitted_bits + phy_vars_eNB->eNB_UE_stats[1].total_transmitted_bits)/((phy_vars_eNB->frame+1)*10));
    */

    for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
#ifdef OPENAIR2
      if (phy_vars_eNB->dlsch_eNB[(uint8_t)UE_id][0]->rnti>0) {
	phy_vars_eNB->total_dlsch_bitrate = phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_bitrate + phy_vars_eNB->total_dlsch_bitrate;
	phy_vars_eNB->total_transmitted_bits = phy_vars_eNB->eNB_UE_stats[UE_id].total_TBS + phy_vars_eNB->total_transmitted_bits;
	//phy_vars_eNB->total_system_throughput = phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits + phy_vars_eNB->total_system_throughput;
	if (phy_vars_eNB->eNB_UE_stats[UE_id].mode == PUSCH) 
	  success = success + ((phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[1]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[2]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[3]) - (phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[0]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[1]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[2]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[3]));
      }
#else
      phy_vars_eNB->total_dlsch_bitrate = phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_bitrate + phy_vars_eNB->total_dlsch_bitrate;
      phy_vars_eNB->total_transmitted_bits = phy_vars_eNB->eNB_UE_stats[UE_id].total_TBS +  phy_vars_eNB->total_transmitted_bits;
      //phy_vars_eNB->total_system_throughput = phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits + phy_vars_eNB->total_system_throughput;
      success = success + ((phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[1]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[2]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[3]) - (phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[0]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[1]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[2]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[3]));
#endif
    }

    len += sprintf(&buffer[len],"[eNB PROC] Total DLSCH bits successfully transmitted %d kbits in %d frame(s)\n",(phy_vars_eNB->total_transmitted_bits/1000),phy_vars_eNB->frame+1);
    len += sprintf(&buffer[len],"[eNB PROC] Total DLSCH average system throughput %d kbps\n",(phy_vars_eNB->total_dlsch_bitrate/1000));
    len += sprintf(&buffer[len],"[eNB PROC] Total DLSCH successful transmissions %d in %d frame(s)\n",success,phy_vars_eNB->frame+1);
    //len += sprintf(&buffer[len],"[eNB PROC] FULL MU-MIMO Transmissions/Total Transmissions = %d/%d\n",phy_vars_eNB->FULL_MUMIMO_transmissions,phy_vars_eNB->check_for_total_transmissions);
    //len += sprintf(&buffer[len],"[eNB PROC] MU-MIMO Transmissions/Total Transmissions = %d/%d\n",phy_vars_eNB->check_for_MUMIMO_transmissions,phy_vars_eNB->check_for_total_transmissions);
    //len += sprintf(&buffer[len],"[eNB PROC] SU-MIMO Transmissions/Total Transmissions = %d/%d\n",phy_vars_eNB->check_for_SUMIMO_transmissions,phy_vars_eNB->check_for_total_transmissions);
     
  }
  
  len += sprintf(&buffer[len],"\n");

  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
#ifdef OPENAIR2
    if (phy_vars_eNB->dlsch_eNB[(uint8_t)UE_id][0]->rnti>0) {
#endif
      harq_pid_dl = phy_vars_eNB->dlsch_eNB[(uint8_t)UE_id][0]->current_harq_pid;
      harq_pid_ul = 2;
      len += sprintf(&buffer[len],"[eNB PROC] UE %d (%x) RSSI: (%d,%d) dBm, Sector %d, DLSCH Mode %d, UE_DL_mcs %d, UE_UL_MCS %d\n",
		     UE_id,
		     phy_vars_eNB->eNB_UE_stats[UE_id].crnti,
		     phy_vars_eNB->eNB_UE_stats[UE_id].UL_rssi[0],
		     phy_vars_eNB->eNB_UE_stats[UE_id].UL_rssi[1],
		     phy_vars_eNB->eNB_UE_stats[UE_id].sector,
		     phy_vars_eNB->transmission_mode[UE_id],
		     phy_vars_eNB->dlsch_eNB[(uint8_t)UE_id][0]->harq_processes[harq_pid_dl]->mcs,
		     phy_vars_eNB->ulsch_eNB[(uint8_t)UE_id]->harq_processes[harq_pid_ul]->mcs);
      
      len += sprintf(&buffer[len],"[eNB PROC] Wideband CQI: (%d,%d) dB\n",
		     phy_vars_eNB->PHY_measurements_eNB[eNB].wideband_cqi_dB[UE_id][0],
		     phy_vars_eNB->PHY_measurements_eNB[eNB].wideband_cqi_dB[UE_id][1]);
      
      len += sprintf(&buffer[len],"[eNB PROC] Subband CQI: ");
      for (i=0;i<25;i++)
	len += sprintf(&buffer[len],"%2d ",
		       phy_vars_eNB->PHY_measurements_eNB[eNB].subband_cqi_tot_dB[UE_id][i]);
      len += sprintf(&buffer[len],"\n");
      
      len += sprintf(&buffer[len],"[eNB PROC] DL_cqi %d, DL_pmi_single %x\n",
		     phy_vars_eNB->eNB_UE_stats[UE_id].DL_cqi[0],
		     pmi2hex_2Ar1(phy_vars_eNB->eNB_UE_stats[UE_id].DL_pmi_single));
      
      len += sprintf(&buffer[len],"[eNB PROC] Timing advance %d samples (%d 16Ts), update %d\n",
		     phy_vars_eNB->eNB_UE_stats[UE_id].UE_timing_offset,
		     phy_vars_eNB->eNB_UE_stats[UE_id].UE_timing_offset>>2,
		     phy_vars_eNB->eNB_UE_stats[UE_id].timing_advance_update);
      
      len += sprintf(&buffer[len],"[eNB PROC] Mode = %s(%d)\n",
		     mode_string[phy_vars_eNB->eNB_UE_stats[UE_id].mode],
		     phy_vars_eNB->eNB_UE_stats[UE_id].mode);
#ifdef OPENAIR2
      UE_id_mac = find_UE_id(phy_vars_eNB->Mod_id,phy_vars_eNB->dlsch_eNB[(uint8_t)UE_id][0]->rnti);
      RRC_status = mac_get_rrc_status(phy_vars_eNB->Mod_id,1,UE_id_mac);
	
      len += sprintf(&buffer[len],"[eNB PROC] UE_id_mac = %d, RRC status = %d\n",UE_id_mac,RRC_status);
#endif
      
#ifdef OPENAIR2
      if (phy_vars_eNB->eNB_UE_stats[UE_id].mode == PUSCH) {
#endif
	for (i=0;i<3;i++) {
	  ulsch_errors += phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_errors[i];
	  for (j=0;j<4;j++) {
	    ulsch_round_attempts[j] += phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[i][j];
	    ulsch_round_errors[j] += phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[i][j];
	  }
	}

	len += sprintf(&buffer[len],"[eNB PROC] SR received/total: %d/%d (diff %d)\n",
		       phy_vars_eNB->eNB_UE_stats[UE_id].sr_received,
		       phy_vars_eNB->eNB_UE_stats[UE_id].sr_total,
		       phy_vars_eNB->eNB_UE_stats[UE_id].sr_total-phy_vars_eNB->eNB_UE_stats[UE_id].sr_received);
	len += sprintf(&buffer[len],"[eNB PROC] ULSCH FER per round (%d, %d, %d, %d) : (%d, %d, %d, %d) : (%d, %d, %d, %d)  \n",
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[0][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[0][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[0][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[0][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[1][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[1][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[1][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[1][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[2][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[2][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[2][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_fer[2][3]);

		       

	len += sprintf(&buffer[len],"[eNB PROC] ULSCH errors %d/%d (%d/%d,%d/%d,%d/%d,%d/%d) : %d/%d (%d/%d,%d/%d,%d/%d,%d/%d) : %d/%d (%d/%d,%d/%d,%d/%d,%d/%d) \n",
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_errors[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[0][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[0][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[0][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[0][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[0][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[0][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[0][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[0][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[0][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_errors[1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[1][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[1][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[1][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[1][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[1][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[1][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[1][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[1][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[1][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_errors[2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[2][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[2][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[2][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[2][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[2][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[2][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[2][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[2][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[2][3]);
	len += sprintf(&buffer[len],"[eNB PROC] DLSCH errors %d/%d (%d/%d/%d,%d/%d/%d,%d/%d/%d,%d/%d/%d)\n",
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_l2_errors,
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_ACK[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_ACK[1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_ACK[2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_ACK[3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[3]);

	len += sprintf(&buffer[len],"[eNB PROC] DLSCH total bits from MAC: %dkbit\n",(phy_vars_eNB->eNB_UE_stats[UE_id].total_TBS_MAC)/1000);
	len += sprintf(&buffer[len],"[eNB PROC] DLSCH total bits ack'ed: %dkbit\n",(phy_vars_eNB->eNB_UE_stats[UE_id].total_TBS)/1000);
	len += sprintf(&buffer[len],"[eNB PROC] DLSCH Average throughput (10 frames): %dkbps\n",(phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_bitrate/1000));
	len += sprintf(&buffer[len],"[eNB PROC] Transmission Mode %d\n",phy_vars_eNB->transmission_mode[UE_id]);
 
	if(phy_vars_eNB->transmission_mode[UE_id] == 5){
	  if(phy_vars_eNB->mu_mimo_mode[UE_id].dl_pow_off == 0)
	    len += sprintf(&buffer[len],"[eNB PROC] ****UE %d is in MU-MIMO mode****\n",UE_id);
	  else if(phy_vars_eNB->mu_mimo_mode[UE_id].dl_pow_off == 1)
	    len += sprintf(&buffer[len],"[eNB PROC] ****UE %d is in SU-MIMO mode****\n",UE_id);
	  else
	    len += sprintf(&buffer[len],"[eNB PROC] ****UE %d is not scheduled****\n",UE_id);
	}
	len += sprintf(&buffer[len],"[eNB PROC] RB Allocation on Sub-bands: ");
	
    //	for (j=0;j< mac_xface->lte_frame_parms->N_RBGS;j++)
	for (j=0;j<7;j++)
	  len += sprintf(&buffer[len],"%d ",
			 phy_vars_eNB->mu_mimo_mode[UE_id].rballoc_sub[j]);
	len += sprintf(&buffer[len],"\n");
	len += sprintf(&buffer[len],"[eNB PROC] Total Number of Allocated PRBs = %d\n",phy_vars_eNB->mu_mimo_mode[UE_id].pre_nb_available_rbs);
	
	
#ifdef OPENAIR2
      }
    }
#endif
    len += sprintf(&buffer[len],"\n");
  }
  len += sprintf(&buffer[len],"EOF\n");
  len += sprintf(&buffer[len],"\0");
  
  return len;
}
