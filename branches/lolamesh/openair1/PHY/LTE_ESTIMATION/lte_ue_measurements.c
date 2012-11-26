// this function fills the PHY_vars->PHY_measurement structure

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "emmintrin.h"

#ifdef __SSE3__
#include "pmmintrin.h"
#include "tmmintrin.h"
#else
__m128i zeroPMI;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroPMI,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroPMI,(xmmy)))
#endif

//#define k1 1000
#define k1 ((long long int) 512)
#define k2 ((long long int) (1024-k1))

#ifdef USER_MODE
void print_shorts(char *s,__m128i *x) {

  short *tempb = (short *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7]
         );

}
void print_ints(char *s,__m128i *x) {

  int *tempb = (int *)x;

  printf("%s  : %d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3]
         );

}
#endif

__m128i pmi128_re,pmi128_im;
__m128i mmtmpPMI0,mmtmpPMI1,mmtmpPMI2,mmtmpPMI3;

s16 get_PL(u8 Mod_id,u8 eNB_index) {

  PHY_VARS_UE *phy_vars_ue = PHY_vars_UE_g[Mod_id];

  /*  
    msg("get_PL : rssi %d, eNB power %d\n",
	dB_fixed(phy_vars_ue->PHY_measurements.rssi)-phy_vars_ue->rx_total_gain_dB,
	// phy_vars_ue->lte_frame_parms.pdsch_config_common.referenceSignalPower); // apaposto
        phy_vars_ue->lte_frame_parms[eNB_index]->pdsch_config_common.referenceSignalPower);  // apaposto
  */

  return((s16)(phy_vars_ue->rx_total_gain_dB-dB_fixed(phy_vars_ue->PHY_measurements.rssi) + 
                 phy_vars_ue->lte_frame_parms[eNB_index]->pdsch_config_common.referenceSignalPower)); 
}

 
void ue_rrc_measurements(PHY_VARS_UE *phy_vars_ue,
			 u8 slot,
			 u8 abstraction_flag, u8 eNB_id) { 

  int aarx,i,rb;
  s16 *rxF;

  u16 Nid_cell = phy_vars_ue->lte_frame_parms[eNB_id]->Nid_cell; 
  u8 eNB_offset,nu,l,nushift,k;
  u16 off,off2;
  s16 rx_power_correction;


  // if the fft size an odd power of 2, the output of the fft is shifted one too much, so we need to compensate for that

   if ( (abstraction_flag==0) && ((phy_vars_ue->lte_frame_parms[eNB_id]->ofdm_symbol_size == 128) ||
				   (phy_vars_ue->lte_frame_parms[eNB_id]->ofdm_symbol_size == 512)) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;
  
  for (eNB_offset = 0;eNB_offset<1+phy_vars_ue->PHY_measurements.n_adj_cells;eNB_offset++) {

    if (eNB_offset==0)
      phy_vars_ue->PHY_measurements.rssi = 0;


    LOG_D(PHY,"ue_rrc_measurements: eNB_offset %d => rssi %d\n",eNB_offset,phy_vars_ue->PHY_measurements.rssi);
    // recompute nushift with eNB_offset corresponding to adjacent eNB on which to perform channel estimation
    //    printf("[PHY][UE %d] Frame %d slot %d Doing ue_rrc_measurements rsrp/rssi (Nid_cell %d, Nid2 %d, nushift %d, eNB_offset %d)\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,slot,Nid_cell,Nid2,nushift,eNB_offset);
    if (eNB_offset > 0)
      Nid_cell = phy_vars_ue->PHY_measurements.adj_cell_id[eNB_offset-1];


    nushift =  Nid_cell%6;



    phy_vars_ue->PHY_measurements.rsrp[eNB_offset] = 0;


    if (abstraction_flag == 0) {
      
      // compute RSRP using symbols 0 and 4-frame_parms->Ncp

      for (l=0,nu=0;l<=(4-phy_vars_ue->lte_frame_parms[eNB_id]->Ncp);l+=(4-phy_vars_ue->lte_frame_parms[eNB_id]->Ncp),nu=3) {  
	k = (nu + nushift)%6;
	LOG_D(PHY,"[UE %d] Frame %d slot %d Doing ue_rrc_measurements rsrp/rssi (Nid_cell %d, nushift %d, eNB_offset %d, k %d)\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,slot,Nid_cell,nushift,eNB_offset,k);
	for (aarx=0;aarx<phy_vars_ue->lte_frame_parms[eNB_id]->nb_antennas_rx;aarx++) { 
	  rxF = (s16 *)&phy_vars_ue->lte_ue_common_vars[eNB_id]->rxdataF[aarx][(l*phy_vars_ue->lte_frame_parms[eNB_id]->ofdm_symbol_size)<<1];
	  off  = (phy_vars_ue->lte_frame_parms[eNB_id]->first_carrier_offset+k)<<2; 
	  off2 = (phy_vars_ue->lte_frame_parms[eNB_id]->first_carrier_offset)<<2;  

	    
	  for (rb=0;rb<phy_vars_ue->lte_frame_parms[eNB_id]->N_RB_DL;rb++) { 

	    if (l==(4-phy_vars_ue->lte_frame_parms[eNB_id]->Ncp)) { 
		
		//	  printf("rb %d, off %d, off2 %d\n",rb,off,off2);
		
		phy_vars_ue->PHY_measurements.rsrp[eNB_offset] += ((rxF[off]*rxF[off])+(rxF[off+1]*rxF[off+1]));
		off+=24;
		if (off>=(phy_vars_ue->lte_frame_parms[eNB_id]->ofdm_symbol_size<<2)) 
		  off = (1+k)<<2;
		phy_vars_ue->PHY_measurements.rsrp[eNB_offset] += ((rxF[off]*rxF[off])+(rxF[off+1]*rxF[off+1]));
		off+=24;
		if (off>=(phy_vars_ue->lte_frame_parms[eNB_id]->ofdm_symbol_size<<2)) 
		  off = (1+k)<<2;
	      }
	  

	      if ((eNB_offset==0)&&(l==0)) {
		for (i=0;i<6;i++,off2+=4)
		  phy_vars_ue->PHY_measurements.rssi += ((rxF[off2]*rxF[off2])+(rxF[off2+1]*rxF[off2+1]));
		if (off2==(phy_vars_ue->lte_frame_parms[eNB_id]->ofdm_symbol_size<<2)) 
		  off2=4;
		for (i=0;i<6;i++,off2+=4)
		  phy_vars_ue->PHY_measurements.rssi += ((rxF[off2]*rxF[off2])+(rxF[off2+1]*rxF[off2+1]));
	      }
	      //	  printf("slot %d, rb %d => rsrp %d, rssi %d\n",slot,rb,phy_vars_ue->PHY_measurements.rsrp[eNB_offset],phy_vars_ue->PHY_measurements.rssi);
	    }
	}
      }



      phy_vars_ue->PHY_measurements.rsrp[eNB_offset]/=(24*phy_vars_ue->lte_frame_parms[eNB_id]->N_RB_DL); 
			phy_vars_ue->PHY_measurements.rsrp[eNB_offset]*=rx_power_correction;

      //      phy_vars_ue->PHY_measurements.rsrp[eNB_offset] = phy_vars_ue->PHY_measurements.rx_spatial_power[eNB_offset][0][0]/(2*phy_vars_ue->lte_frame_parms.N_RB_DL);
      if (eNB_offset == 0) {
	phy_vars_ue->PHY_measurements.rssi/=(24*phy_vars_ue->lte_frame_parms[eNB_id]->N_RB_DL); 
	phy_vars_ue->PHY_measurements.rssi*=rx_power_correction;
      }
      if (phy_vars_ue->PHY_measurements.rssi>0)
	phy_vars_ue->PHY_measurements.rsrq[eNB_offset] = 100*phy_vars_ue->PHY_measurements.rsrp[eNB_offset]*phy_vars_ue->lte_frame_parms[eNB_id]->N_RB_DL/phy_vars_ue->PHY_measurements.rssi;
      else
	phy_vars_ue->PHY_measurements.rsrq[eNB_offset] = -12000;
      
      //((200*phy_vars_ue->PHY_measurements.rsrq[eNB_offset]) + ((1024-200)*100*phy_vars_ue->PHY_measurements.rsrp[eNB_offset]*phy_vars_ue->lte_frame_parms.N_RB_DL/phy_vars_ue->PHY_measurements.rssi))>>10;
    }
    else {   // Do abstraction of RSRP and RSRQ
      phy_vars_ue->PHY_measurements.rssi = phy_vars_ue->PHY_measurements.rx_power_avg[0];

    }
    if (((phy_vars_ue->frame %100) == 0) && (slot == 1)) {
      if (eNB_offset == 0)
	LOG_D(PHY,"[UE %d] Frame %d, slot %d RRC Measurements => rssi %3.1f dBm\n",phy_vars_ue->Mod_id,
	      phy_vars_ue->frame,slot,10*log10(phy_vars_ue->PHY_measurements.rssi)-phy_vars_ue->rx_total_gain_dB);
      LOG_D(PHY,"[UE %d] Frame %d, slot %d RRC Measurements => rsrp/rsrq[%d][%d] %3.1f (%3.1f) dBm %3.1f dB\n",
	    phy_vars_ue->Mod_id,  
	    phy_vars_ue->frame,slot,eNB_offset,
	    (eNB_offset>0) ? phy_vars_ue->PHY_measurements.adj_cell_id[eNB_offset-1] : phy_vars_ue->lte_frame_parms[eNB_id]->Nid_cell, 
	    (dB_fixed_times10(phy_vars_ue->PHY_measurements.rsrp[eNB_offset])/10.0)-phy_vars_ue->rx_total_gain_dB-dB_fixed(phy_vars_ue->lte_frame_parms[eNB_id]->N_RB_DL*12), 
	    (10*log10(phy_vars_ue->PHY_measurements.rx_spatial_power[eNB_offset][0][0])/10.0)-phy_vars_ue->rx_total_gain_dB-dB_fixed(phy_vars_ue->lte_frame_parms[eNB_id]->N_RB_DL*12)-6.02, 
	    (10*log10(phy_vars_ue->PHY_measurements.rsrq[eNB_offset]))-20);
    
    }
  }
}

void lte_ue_measurements(PHY_VARS_UE *phy_vars_ue,
			   unsigned int subframe_offset,
			   unsigned char N0_symbol,
			 unsigned char abstraction_flag, 
			 u8 eNB_index){ 


    int aarx,aatx,eNB_id=0,rx_power_correction=1,gain_offset=0;
    int rx_power[NUMBER_OF_CONNECTED_eNB_MAX];
    int i;
    unsigned int limit,subband;
    __m128i *dl_ch0_128,*dl_ch1_128;
    int *dl_ch0,*dl_ch1;
    LTE_DL_FRAME_PARMS *frame_parms = phy_vars_ue->lte_frame_parms[eNB_index]; 

  phy_vars_ue->PHY_measurements.nb_antennas_rx = frame_parms->nb_antennas_rx;

#ifdef CBMIMO1
  if (openair_daq_vars.rx_rf_mode == 0)
    gain_offset = 25;
  else
    gain_offset = 0;
#endif

#ifndef __SSE3__
    zeroPMI = _mm_xor_si128(zeroPMI,zeroPMI);
#endif
  
    if (phy_vars_ue->init_averaging == 1) {
      for (eNB_id=0;eNB_id<phy_vars_ue->n_connected_eNB;eNB_id++) {
	phy_vars_ue->PHY_measurements.rx_power_avg[eNB_id] = 0;
      }

      for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
	phy_vars_ue->PHY_measurements.n0_power[aarx] = 0;
	phy_vars_ue->PHY_measurements.n0_power_dB[aarx] = 0;
      }
    
      phy_vars_ue->PHY_measurements.n0_power_tot = 0;
      phy_vars_ue->PHY_measurements.n0_power_tot_dB = 0;
      phy_vars_ue->PHY_measurements.n0_power_avg = 0;
      phy_vars_ue->PHY_measurements.n0_power_avg_dB = 0;
    }

    for (eNB_id=0;eNB_id<1+phy_vars_ue->n_connected_eNB;eNB_id++) 
      rx_power[eNB_id] = 0;

    // if the fft size an odd power of 2, the output of the fft is shifted one too much, so we need to compensate for that
    if ( (abstraction_flag==0) && ((frame_parms->ofdm_symbol_size == 128) ||
				   (frame_parms->ofdm_symbol_size == 512)) )
      rx_power_correction = 2;
    else
      rx_power_correction = 1;
  
  
    // noise measurements
    // for abstraction we do noise measurements based on the precalculated phy_vars_ue->N0
    // otherwise if there is a symbol where we can take noise measurements on, we measure there
    // otherwise do not update the noise measurements 
  
    if (abstraction_flag!=0) {
      phy_vars_ue->PHY_measurements.n0_power_tot = 0;
      for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
	phy_vars_ue->PHY_measurements.n0_power[aarx] = pow(10.0,phy_vars_ue->N0/10.0)*pow(10.0,((double)phy_vars_ue->rx_total_gain_dB)/10.0);
	phy_vars_ue->PHY_measurements.n0_power_dB[aarx] = (unsigned short) dB_fixed(phy_vars_ue->PHY_measurements.n0_power[aarx]);
	phy_vars_ue->PHY_measurements.n0_power_tot +=  phy_vars_ue->PHY_measurements.n0_power[aarx];
      } 
      phy_vars_ue->PHY_measurements.n0_power_tot_dB = (unsigned short) dB_fixed(phy_vars_ue->PHY_measurements.n0_power_tot);
      phy_vars_ue->PHY_measurements.n0_power_tot_dBm = phy_vars_ue->PHY_measurements.n0_power_tot_dB - phy_vars_ue->rx_total_gain_dB;
    }
    else if (N0_symbol != 0) {
      phy_vars_ue->PHY_measurements.n0_power_tot = 0;
      for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
#ifndef HW_PREFIX_REMOVAL
	phy_vars_ue->PHY_measurements.n0_power[aarx] = signal_energy(&phy_vars_ue->lte_ue_common_vars[eNB_index]->rxdata[aarx][subframe_offset+frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples0],frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples); 
#else
	phy_vars_ue->PHY_measurements.n0_power[aarx] = signal_energy(&phy_vars_ue->lte_ue_common_vars[eNB_index]->rxdata[aarx][subframe_offset+frame_parms->ofdm_symbol_size],frame_parms->ofdm_symbol_size); 
#endif
	phy_vars_ue->PHY_measurements.n0_power_dB[aarx] = (unsigned short) dB_fixed(phy_vars_ue->PHY_measurements.n0_power[aarx]);
	phy_vars_ue->PHY_measurements.n0_power_tot +=  phy_vars_ue->PHY_measurements.n0_power[aarx];
      }

      phy_vars_ue->PHY_measurements.n0_power_tot_dB = (unsigned short) dB_fixed(phy_vars_ue->PHY_measurements.n0_power_tot);
      phy_vars_ue->PHY_measurements.n0_power_tot_dBm = phy_vars_ue->PHY_measurements.n0_power_tot_dB - phy_vars_ue->rx_total_gain_dB + gain_offset;
      //    printf("PHY measurements UE %d: n0_power %d (%d)\n",phy_vars_ue->Mod_id,phy_vars_ue->PHY_measurements.n0_power_tot_dBm,phy_vars_ue->PHY_measurements.n0_power_tot_dB);
    }
    else {
      phy_vars_ue->PHY_measurements.n0_power_tot_dBm = phy_vars_ue->PHY_measurements.n0_power_tot_dB - phy_vars_ue->rx_total_gain_dB + gain_offset;
    }

    // signal measurements  
    for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
      for (aatx=0; aatx<frame_parms->nb_antennas_tx_eNB; aatx++) {
	for (eNB_id=0;eNB_id<phy_vars_ue->n_connected_eNB;eNB_id++) {
	  phy_vars_ue->PHY_measurements.rx_spatial_power[eNB_id][aatx][aarx] = 
	    (signal_energy_nodc(&phy_vars_ue->lte_ue_common_vars[eNB_index]->dl_ch_estimates[eNB_id][(aatx<<1) + aarx][0], 
				(frame_parms->nb_prefix_samples))*rx_power_correction) - phy_vars_ue->PHY_measurements.n0_power[aarx];
	
	  if (phy_vars_ue->PHY_measurements.rx_spatial_power[eNB_id][aatx][aarx]<0)
	    phy_vars_ue->PHY_measurements.rx_spatial_power[eNB_id][aatx][aarx] = 0; //phy_vars_ue->PHY_measurements.n0_power[aarx];
	
	  phy_vars_ue->PHY_measurements.rx_spatial_power_dB[eNB_id][aatx][aarx] = (unsigned short) dB_fixed(phy_vars_ue->PHY_measurements.rx_spatial_power[eNB_id][aatx][aarx]);
	
	  if (aatx==0)
	    phy_vars_ue->PHY_measurements.wideband_cqi[eNB_id][aarx] = phy_vars_ue->PHY_measurements.rx_spatial_power[eNB_id][aatx][aarx];
	  else
	    phy_vars_ue->PHY_measurements.wideband_cqi[eNB_id][aarx] += phy_vars_ue->PHY_measurements.rx_spatial_power[eNB_id][aatx][aarx];
	}
      }
  
      for (eNB_id = 0; eNB_id < phy_vars_ue->n_connected_eNB; eNB_id++){
	//      phy_measurements->rx_power[eNB_id][aarx]/=frame_parms->nb_antennas_tx;
	phy_vars_ue->PHY_measurements.wideband_cqi_dB[eNB_id][aarx] = (unsigned short) dB_fixed(phy_vars_ue->PHY_measurements.wideband_cqi[eNB_id][aarx]);
	rx_power[eNB_id] += phy_vars_ue->PHY_measurements.wideband_cqi[eNB_id][aarx];
	//      phy_vars_ue->PHY_measurements.rx_avg_power_dB[eNB_id] += phy_vars_ue->PHY_measurements.rx_power_dB[eNB_id][aarx];
      }

    }

    // filter to remove jitter
    if (phy_vars_ue->init_averaging == 0) {
      for (eNB_id = 0; eNB_id < phy_vars_ue->n_connected_eNB; eNB_id++)
	phy_vars_ue->PHY_measurements.rx_power_avg[eNB_id] = (int) 
	  (((k1*((long long int)(phy_vars_ue->PHY_measurements.rx_power_avg[eNB_id]))) + 
	    (k2*((long long int)(rx_power[eNB_id]))))>>10);
      phy_vars_ue->PHY_measurements.n0_power_avg = (int)
	(((k1*((long long int) (phy_vars_ue->PHY_measurements.n0_power_avg))) + 
	  (k2*((long long int) (phy_vars_ue->PHY_measurements.n0_power_tot))))>>10);
    }
    else {
      for (eNB_id = 0; eNB_id < phy_vars_ue->n_connected_eNB; eNB_id++)
	phy_vars_ue->PHY_measurements.rx_power_avg[eNB_id] = rx_power[eNB_id];
      phy_vars_ue->PHY_measurements.n0_power_avg = phy_vars_ue->PHY_measurements.n0_power_tot;
      phy_vars_ue->init_averaging = 0;
    }

    for (eNB_id = 0; eNB_id < phy_vars_ue->n_connected_eNB; eNB_id++) {
      phy_vars_ue->PHY_measurements.rx_power_avg_dB[eNB_id] = dB_fixed( phy_vars_ue->PHY_measurements.rx_power_avg[eNB_id]);
      phy_vars_ue->PHY_measurements.wideband_cqi_tot[eNB_id] = dB_fixed2(rx_power[eNB_id],phy_vars_ue->PHY_measurements.n0_power_tot);
      phy_vars_ue->PHY_measurements.wideband_cqi_avg[eNB_id] = dB_fixed2(phy_vars_ue->PHY_measurements.rx_power_avg[eNB_id],phy_vars_ue->PHY_measurements.n0_power_avg);
      phy_vars_ue->PHY_measurements.rx_rssi_dBm[eNB_id] = phy_vars_ue->PHY_measurements.rx_power_avg_dB[eNB_id] - phy_vars_ue->rx_total_gain_dB + gain_offset;
    }
    phy_vars_ue->PHY_measurements.n0_power_avg_dB = dB_fixed( phy_vars_ue->PHY_measurements.n0_power_avg);

    for (eNB_id = 0; eNB_id < phy_vars_ue->n_connected_eNB; eNB_id++) {
      if (frame_parms->mode1_flag==0) {
	// cqi/pmi information
      
	for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
	  dl_ch0    = &phy_vars_ue->lte_ue_common_vars[eNB_index]->dl_ch_estimates[eNB_id][aarx][4]; 
	  dl_ch1    = &phy_vars_ue->lte_ue_common_vars[eNB_index]->dl_ch_estimates[eNB_id][2+aarx][4]; 
	
	  for (subband=0;subband<7;subband++) {
	  
	    // cqi
	    if (aarx==0)
	      phy_vars_ue->PHY_measurements.subband_cqi_tot[eNB_id][subband]=0;
	  
	    if (subband<6) {
	      /*
		for (i=0;i<48;i++)
		msg("subband %d (%d) : %d,%d\n",subband,i,((short *)dl_ch0)[2*i],((short *)dl_ch0)[1+(2*i)]);
	      */
	      phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband] = 
		(signal_energy_nodc(dl_ch0,48) + signal_energy_nodc(dl_ch1,48))*rx_power_correction;
	      if ( phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband] < phy_vars_ue->PHY_measurements.n0_power[aarx])
		phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband]=0;
	      else
		phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband]-=phy_vars_ue->PHY_measurements.n0_power[aarx];
	      
	      phy_vars_ue->PHY_measurements.subband_cqi_tot[eNB_id][subband] += phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband];
	      phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB_id][aarx][subband] = dB_fixed2(phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband],
											      phy_vars_ue->PHY_measurements.n0_power[aarx]);
	    }
	    else {
	      //	    for (i=0;i<12;i++)
	      //	      printf("subband %d (%d) : %d,%d\n",subband,i,((short *)dl_ch0)[2*i],((short *)dl_ch0)[1+(2*i)]); 
	      phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband] = (signal_energy_nodc(dl_ch0,12) + signal_energy_nodc(dl_ch1,12))*rx_power_correction - phy_vars_ue->PHY_measurements.n0_power[aarx];
	      phy_vars_ue->PHY_measurements.subband_cqi_tot[eNB_id][subband] += phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband];
	      phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB_id][aarx][subband] = dB_fixed2(phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband],
											      phy_vars_ue->PHY_measurements.n0_power[aarx]);			
	    }
	    dl_ch1+=48;
	    dl_ch0+=48;
	    //	  msg("subband_cqi[%d][%d][%d] => %d (%d dB)\n",eNB_id,aarx,subband,phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband],phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB_id][aarx][subband]);
	  }
	
	}
	for (subband=0;subband<7;subband++) {
	  phy_vars_ue->PHY_measurements.subband_cqi_tot_dB[eNB_id][subband] = dB_fixed2(phy_vars_ue->PHY_measurements.subband_cqi_tot[eNB_id][subband],phy_vars_ue->PHY_measurements.n0_power_tot);
	  //	  msg("subband_cqi_tot[%d][%d] => %d dB (n0 %d)\n",eNB_id,subband,phy_vars_ue->PHY_measurements.subband_cqi_tot_dB[eNB_id][subband],phy_vars_ue->PHY_measurements.n0_power_tot);
	}	
      
	for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
	  dl_ch0_128    = (__m128i *)&phy_vars_ue->lte_ue_common_vars[eNB_index]->dl_ch_estimates[eNB_id][aarx][8];  
	  dl_ch1_128    = (__m128i *)&phy_vars_ue->lte_ue_common_vars[eNB_index]->dl_ch_estimates[eNB_id][2+aarx][8]; 
	  /*
	    #ifdef DEBUG_PHY	
	    if(eNB_id==0){
	    print_shorts("Ch0",dl_ch0_128);
	    print_shorts("Ch1",dl_ch1_128);
	    printf("eNB_ID = %d\n",eNB_id);
	    }
	    #endif
	  */
	  for (subband=0;subband<7;subband++) {
	  
	  
	    // pmi
	  
	    pmi128_re = _mm_xor_si128(pmi128_re,pmi128_re);
	    pmi128_im = _mm_xor_si128(pmi128_im,pmi128_im);
	    // limit is the number of groups of 4 REs in a subband (12 = 4 RBs, 3 = 1 RB)
	    // for 5 MHz channelization, there are 7 subbands, 6 of size 4 RBs and 1 of size 1 RB
	    limit = (subband < 6) ? 12 : 3;
	    for (i=0;i<limit;i++) {
	    
	      // For each RE in subband perform ch0 * conj(ch1)
	      // multiply by conjugated channel
	      // if(eNB_id==0){
	      //print_shorts("ch0",dl_ch0_128);
	      //print_shorts("ch1",dl_ch1_128);
	      // }
	      // if(i==0){
	      mmtmpPMI0 = _mm_xor_si128(mmtmpPMI0,mmtmpPMI0);
	      mmtmpPMI1 = _mm_xor_si128(mmtmpPMI1,mmtmpPMI1);
	      //	    }
	      // if(eNB_id==0)
	      // print_ints("Pre_re",&mmtmpPMI0);

	      mmtmpPMI0 = _mm_madd_epi16(dl_ch0_128[0],dl_ch1_128[0]);
	      //  if(eNB_id==0)
	      //  print_ints("re",&mmtmpPMI0);
	    
	      // mmtmpPMI0 contains real part of 4 consecutive outputs (32-bit)
	      // print_shorts("Ch1",dl_ch1_128);
	    
	      mmtmpPMI1 = _mm_shufflelo_epi16(dl_ch1_128[0],_MM_SHUFFLE(2,3,0,1));//_MM_SHUFFLE(2,3,0,1)
	      // print_shorts("mmtmpPMI1:",&mmtmpPMI1);
	      mmtmpPMI1 = _mm_shufflehi_epi16(mmtmpPMI1,_MM_SHUFFLE(2,3,0,1));
	      // print_shorts("mmtmpPMI1:",&mmtmpPMI1);

	      mmtmpPMI1 = _mm_sign_epi16(mmtmpPMI1,*(__m128i*)&conjugate[0]);
	      // print_shorts("mmtmpPMI1:",&mmtmpPMI1);
	      mmtmpPMI1 = _mm_madd_epi16(mmtmpPMI1,dl_ch0_128[0]);
	      //  if(eNB_id==0)
	      //  print_ints("im",&mmtmpPMI1);
	      // mmtmpPMI1 contains imag part of 4 consecutive outputs (32-bit)
	    
	      pmi128_re = _mm_add_epi32(pmi128_re,mmtmpPMI0);
	      pmi128_im = _mm_add_epi32(pmi128_im,mmtmpPMI1);
	      dl_ch0_128++;
	      dl_ch1_128++;
	    }
	    phy_vars_ue->PHY_measurements.subband_pmi_re[eNB_id][subband][aarx] = (((int *)&pmi128_re)[0] + ((int *)&pmi128_re)[1] + ((int *)&pmi128_re)[2] + ((int *)&pmi128_re)[3])>>2;
	    //	  if(eNB_id==0)
	    // printf("in lte_ue_measurements.c: pmi_re %d\n",phy_vars_ue->PHY_measurements.subband_pmi_re[eNB_id][subband][aarx]);
	    phy_vars_ue->PHY_measurements.subband_pmi_im[eNB_id][subband][aarx] = (((int *)&pmi128_im)[0] + ((int *)&pmi128_im)[1] + ((int *)&pmi128_im)[2] + ((int *)&pmi128_im)[3])>>2;
	    //	  if(eNB_id==0)
	    // printf("in lte_ue_measurements.c: pmi_im %d\n",phy_vars_ue->PHY_measurements.subband_pmi_im[eNB_id][subband][aarx]);
	    phy_vars_ue->PHY_measurements.wideband_pmi_re[eNB_id][aarx] += phy_vars_ue->PHY_measurements.subband_pmi_re[eNB_id][subband][aarx];	  phy_vars_ue->PHY_measurements.wideband_pmi_im[eNB_id][aarx] += phy_vars_ue->PHY_measurements.subband_pmi_im[eNB_id][subband][aarx];
	    //	    msg("subband_pmi[%d][%d][%d] => (%d,%d)\n",eNB_id,subband,aarx,phy_vars_ue->PHY_measurements.subband_pmi_re[eNB_id][subband][aarx],phy_vars_ue->PHY_measurements.subband_pmi_im[eNB_id][subband][aarx]);
	  
	  } // subband loop
	} // rx antenna loop  
      }  // if frame_parms->mode1_flag == 0
      else {
	// cqi information only for mode 1
	for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
	  dl_ch0    = &phy_vars_ue->lte_ue_common_vars[eNB_index]->dl_ch_estimates[eNB_id][aarx][4]; 
	
	  for (subband=0;subband<7;subband++) {
	  
	    // cqi
	    if (aarx==0)
	      phy_vars_ue->PHY_measurements.subband_cqi_tot[eNB_id][subband]=0;
	  
	    if (subband<6) {
	      //	    for (i=0;i<48;i++)
	      //	      printf("subband %d (%d) : %d,%d\n",subband,i,((short *)dl_ch0)[2*i],((short *)dl_ch0)[1+(2*i)]);
	      phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband] = 
		(signal_energy_nodc(dl_ch0,48) )*rx_power_correction - phy_vars_ue->PHY_measurements.n0_power[aarx];
	    
	      phy_vars_ue->PHY_measurements.subband_cqi_tot[eNB_id][subband] += phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband];
	      phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB_id][aarx][subband] = dB_fixed2(phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband],
											      phy_vars_ue->PHY_measurements.n0_power[aarx]);
	    }
	    else {
	      //	    for (i=0;i<12;i++)
	      //	      printf("subband %d (%d) : %d,%d\n",subband,i,((short *)dl_ch0)[2*i],((short *)dl_ch0)[1+(2*i)]); 
	      phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband] = (signal_energy_nodc(dl_ch0,12) )*rx_power_correction - phy_vars_ue->PHY_measurements.n0_power[aarx];
	      phy_vars_ue->PHY_measurements.subband_cqi_tot[eNB_id][subband] += phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband];
	      phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB_id][aarx][subband] = dB_fixed2(phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband],
											      phy_vars_ue->PHY_measurements.n0_power[aarx]);							
	    }
	    dl_ch1+=48;
	    //	  msg("subband_cqi[%d][%d][%d] => %d (%d dB)\n",eNB_id,aarx,subband,phy_vars_ue->PHY_measurements.subband_cqi[eNB_id][aarx][subband],phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB_id][aarx][subband]);
	  }
	}
	for (subband=0;subband<7;subband++) {
	  phy_vars_ue->PHY_measurements.subband_cqi_tot_dB[eNB_id][subband] = dB_fixed2(phy_vars_ue->PHY_measurements.subband_cqi_tot[eNB_id][subband],phy_vars_ue->PHY_measurements.n0_power_tot);
	}
      }

      phy_vars_ue->PHY_measurements.rank[eNB_id] = 0;
      for (i=0;i<NUMBER_OF_SUBBANDS;i++) {
	phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB_id][i] = 0;
	if (frame_parms->nb_antennas_rx>1) {
	  if (phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB_id][0][i] >= phy_vars_ue->PHY_measurements.subband_cqi_dB[eNB_id][1][i])
	    phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB_id][i] = 0;
	  else
	    phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB_id][i] = 1;
	}
	else
	  phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB_id][i] = 0;
      }
      // if(eNB_id==0)
      // printf("in lte_ue_measurements: selected rx_antenna[eNB_id==0]:%u\n", phy_vars_ue->PHY_measurements.selected_rx_antennas[eNB_id][i]);
    }  // eNB_id loop

    _mm_empty();
    _m_empty();

  }


  void lte_ue_measurements_emul(PHY_VARS_UE *phy_vars_ue,u8 last_slot,u8 eNB_id) {

    msg("[PHY] EMUL UE lte_ue_measurements_emul last slot %d, eNB_id %d\n",last_slot,eNB_id);
  }

