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

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/extern.h"

extern u8 number_of_cards;

int dump_ue_stats(PHY_VARS_UE *phy_vars_ue, char* buffer, int len) {

  u8 eNB=0;

  if (phy_vars_ue==NULL)
    return 0;

  len += sprintf(&buffer[len], "[UE_PROC] UE %d, RNTI %x\n",phy_vars_ue->Mod_id, phy_vars_ue->lte_ue_pdcch_vars[0]->crnti);
  len += sprintf(&buffer[len], "[UE PROC] Frame count: %d\neNB0 RSSI %d dBm (%d dB, %d dB)\neNB1 RSSI %d dBm (%d dB, %d dB)\neNB2 RSSI %d dBm (%d dB, %d dB)\nN0 %d dBm (%d dB, %d dB)\n",
		 phy_vars_ue->frame,
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
    len += sprintf(&buffer[len], "[UE PROC] UE mode = %s (%d)\n",mode_string[phy_vars_ue->UE_mode[0]],phy_vars_ue->UE_mode[0]);
    if (phy_vars_ue->UE_mode[0] == PUSCH)
      len += sprintf(&buffer[len], "[UE PROC] DLSCH FER %d, cqi = %d\n",phy_vars_ue->dlsch_fer[0],phy_vars_ue->current_dlsch_cqi[0]);
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
    
    
    len += sprintf(&buffer[len], "[UE PROC] Transmission Mode %d (mode1_flag %d)\n",phy_vars_ue->transmission_mode[eNB],phy_vars_ue->lte_frame_parms.mode1_flag);
    if (openair_daq_vars.dlsch_transmission_mode == 6)
      len += sprintf(&buffer[len], "[UE PROC] Mode 6 Wideband CQI eNB %d : %d dB\n",eNB,phy_vars_ue->PHY_measurements.precoded_cqi_dB[eNB][0]);
    if (phy_vars_ue->dlsch_ue[0] && phy_vars_ue->dlsch_ue[0][0] && phy_vars_ue->dlsch_ue[0][1]) 
      len += sprintf(&buffer[len], "[UE PROC] Saved PMI for DLSCH eNB %d : %x (%p)\n",eNB,pmi2hex_2Ar1(phy_vars_ue->dlsch_ue[0][0]->pmi_alloc),phy_vars_ue->dlsch_ue[0][0]);
    
    len += sprintf(&buffer[len], "[UE PROC] DLSCH Total %d, Error %d, FER %d\n",phy_vars_ue->dlsch_received[0],phy_vars_ue->dlsch_errors[0],phy_vars_ue->dlsch_fer[0]);
    len += sprintf(&buffer[len], "[UE PROC] DLSCH (SI) Total %d, Error %d\n",phy_vars_ue->dlsch_SI_received[0],phy_vars_ue->dlsch_SI_errors[0]);
    len += sprintf(&buffer[len], "[UE PROC] DLSCH (RA) Total %d, Error %d\n",phy_vars_ue->dlsch_ra_received[0],phy_vars_ue->dlsch_ra_errors[0]);
    len += sprintf(&buffer[len], "[UE PROC] DLSCH Bitrate %dkbps\n",(phy_vars_ue->bitrate[0]/1000));
    len += sprintf(&buffer[len], "[UE PROC] Total Received Bits %dkbits\n",(phy_vars_ue->total_received_bits[0]/1000));

  }
  buffer[len]='\0';

  return len;
} // is_clusterhead

int dump_eNB_stats(PHY_VARS_eNB *phy_vars_eNB, char* buffer, int len) {

  unsigned int success=0;
  u8 eNB,UE_id,i,j;
  u32 ulsch_errors=0;
  u32 ulsch_round_attempts[4]={0,0,0,0},ulsch_round_errors[4]={0,0,0,0};
  phy_vars_eNB->total_dlsch_bitrate = 0;//phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_bitrate + phy_vars_eNB->total_dlsch_bitrate;
  phy_vars_eNB->total_transmitted_bits = 0;// phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits +  phy_vars_eNB->total_transmitted_bits;
  phy_vars_eNB->total_system_throughput = 0;//phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits + phy_vars_eNB->total_system_throughput;
  if (phy_vars_eNB==NULL)
    return 0;

  for (eNB=0;eNB<number_of_cards;eNB++) {
    len += sprintf(&buffer[len],"[eNB PROC] eNB %d/%d Frame %d: RX Gain %d dB, I0 %d dBm (%d,%d) dB \n",
		   eNB,number_of_cards,
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
      if (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti>0) {
	phy_vars_eNB->total_dlsch_bitrate = phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_bitrate + phy_vars_eNB->total_dlsch_bitrate;
	phy_vars_eNB->total_transmitted_bits = phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits +  phy_vars_eNB->total_transmitted_bits;
	phy_vars_eNB->total_system_throughput = phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits + phy_vars_eNB->total_system_throughput;
	if (phy_vars_eNB->eNB_UE_stats[UE_id].mode == PUSCH) 
	  success = success + ((phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[1]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[2]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[3]) - (phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[0]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[1]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[2]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[3]));
      }
#else
      phy_vars_eNB->total_dlsch_bitrate = phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_bitrate + phy_vars_eNB->total_dlsch_bitrate;
      phy_vars_eNB->total_transmitted_bits = phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits +  phy_vars_eNB->total_transmitted_bits;
      phy_vars_eNB->total_system_throughput = phy_vars_eNB->eNB_UE_stats[UE_id].total_transmitted_bits + phy_vars_eNB->total_system_throughput;
      success = success + ((phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[1]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[2]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[3]) - (phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[0]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[1]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[2]+phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[3]));
#endif
    }

    len += sprintf(&buffer[len],"[eNB PROC] Total DLSCH Bitrate for the System %dkbps\n",(phy_vars_eNB->total_dlsch_bitrate/1000));
    len += sprintf(&buffer[len],"[eNB PROC] Total Bits successfully transitted %dKbits in %dframe(s)\n",(phy_vars_eNB->total_transmitted_bits/1000),phy_vars_eNB->frame+1);
    len += sprintf(&buffer[len],"[eNB PROC] Average System Throughput %dKbps\n",(phy_vars_eNB->total_system_throughput)/((phy_vars_eNB->frame+1)*10));
    len += sprintf(&buffer[len],"[eNB PROC] Total Successful DLSCH Transmissions %d in %dframe(s)\n",success,phy_vars_eNB->frame+1);
    if(phy_vars_eNB->transmission_mode[0] == 5){

    len += sprintf(&buffer[len],"[eNB PROC] For TM5:FULL MU-MIMO Transmissions/Total Transmissions = %d/%d\n",phy_vars_eNB->FULL_MUMIMO_transmissions,phy_vars_eNB->check_for_total_transmissions);

    len += sprintf(&buffer[len],"[eNB PROC] For TM5:MU-MIMO Transmissions/Total Transmissions = %d/%d\n",phy_vars_eNB->check_for_MUMIMO_transmissions,phy_vars_eNB->check_for_total_transmissions);
    
    len += sprintf(&buffer[len],"[eNB PROC] For TM5:SU-MIMO Transmissions/Total Transmissions = %d/%d\n\n",phy_vars_eNB->check_for_SUMIMO_transmissions,phy_vars_eNB->check_for_total_transmissions);

    }

    
  }
  
  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
#ifdef OPENAIR2
    if (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti>0) {
#endif
      len += sprintf(&buffer[len],"[eNB PROC] UE %d (%x) RSSI: (%d,%d) dBm, Sector %d, DLSCH Mode %d, DLSCH Rate adaptation %d, ULSCH Allocation mode %d, UE_DL_mcs %d, UE_UL_MCS %d, UE_UL_NB_RB %d\n",
		     UE_id,
		     phy_vars_eNB->eNB_UE_stats[UE_id].crnti,
		     phy_vars_eNB->eNB_UE_stats[UE_id].UL_rssi[0],
		     phy_vars_eNB->eNB_UE_stats[UE_id].UL_rssi[1],
		     phy_vars_eNB->eNB_UE_stats[UE_id].sector,
		     openair_daq_vars.dlsch_transmission_mode,
		     openair_daq_vars.dlsch_rate_adaptation,
		     openair_daq_vars.ulsch_allocation_mode,
		     (openair_daq_vars.dlsch_rate_adaptation == 0) ? openair_daq_vars.target_ue_dl_mcs : ((phy_vars_eNB->eNB_UE_stats[UE_id].DL_cqi[0])),
		     openair_daq_vars.target_ue_ul_mcs,
		     openair_daq_vars.ue_ul_nb_rb
		     );
      
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
      
      len += sprintf(&buffer[len],"[eNB PROC] Timing advance %d samples (%d 16Ts)\n",
		     phy_vars_eNB->eNB_UE_stats[UE_id].UE_timing_offset,
		     phy_vars_eNB->eNB_UE_stats[UE_id].UE_timing_offset>>2);
      
      len += sprintf(&buffer[len],"[eNB PROC] Mode = %s(%d)\n",
		     mode_string[phy_vars_eNB->eNB_UE_stats[UE_id].mode],
		     phy_vars_eNB->eNB_UE_stats[UE_id].mode);
      
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

		       

	len += sprintf(&buffer[len],"[eNB PROC] ULSCH errors %d/%d (%d,%d,%d,%d) : %d/%d (%d,%d,%d,%d) : %d/%d (%d,%d,%d,%d) \n",
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_errors[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[0][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[0][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[0][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[0][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[0][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_errors[1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[1][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[1][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[1][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[1][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[1][3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_errors[2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_decoding_attempts[2][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[2][0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[2][1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[2][2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].ulsch_round_errors[2][3]);
	len += sprintf(&buffer[len],"[eNB PROC] DLSCH errors %d/%d (%d/%d,%d/%d,%d/%d,%d/%d)\n",
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_l2_errors,
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[1],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[2],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_NAK[3],
		       phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[3]);

	len += sprintf(&buffer[len],"[eNB PROC] DLSCH Bitrate %dkbps\n",(phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_bitrate/1000));
	len += sprintf(&buffer[len],"[eNB PROC] Transmission Mode %d\n",phy_vars_eNB->transmission_mode[UE_id]);
 
	if(phy_vars_eNB->transmission_mode[UE_id] == 5){
	    if(phy_vars_eNB->mu_mimo_mode[UE_id].dl_pow_off == 0)
	      len += sprintf(&buffer[len],"[eNB PROC] ****UE %d is in MU-MIMO mode****\n",UE_id);
	    else if(phy_vars_eNB->mu_mimo_mode[UE_id].dl_pow_off == 1)
	      len += sprintf(&buffer[len],"[eNB PROC] ****UE %d is in SU-MIMO mode****\n",UE_id);
	    else
	      len += sprintf(&buffer[len],"[eNB PROC] ****UE %d is not scheduled****\n",UE_id);

	    len += sprintf(&buffer[len],"[eNB PROC] RB Allocation from Subband 1 to 7: ");
	    
	    for (i=0;i<7;i++)
	      len += sprintf(&buffer[len],"%d ",
			     phy_vars_eNB->mu_mimo_mode[UE_id].rballoc_sub[i]);
	    len += sprintf(&buffer[len],"\n");
	    
    
	    len += sprintf(&buffer[len],"[eNB PROC] Total Number of Allocated PRBs = %d\n\n",phy_vars_eNB->mu_mimo_mode[UE_id].pre_nb_available_rbs);
	}

    
#ifdef OPENAIR2
      }
    }
#endif
  }
  buffer[len]='\0';
  
  return len;
}
