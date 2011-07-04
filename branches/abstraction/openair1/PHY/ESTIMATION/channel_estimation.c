/*
* @defgroup _PHY_PARAMETER_ESTIMATION_
* @ingroup _physical_layer_ref_implementation_
* @{
\section _phy_parameter_estimation_ Parameter Estimation Blocks
This section deals with the difference parameter estimation blocks.
*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"


// Channel Estimation
// Parameters:
// 
// rx_offset - synchronization offset from start of frame (UE receiver)
// pilot_offset - the place within the frame
// ignore_prefix - When HW removes cyclic prefix, set to 1, otherwise 0
// sch_index - the actual SCH used for that position
// sch_type - indicates whether it is a CHSCH or a SCH pilot
void phy_channel_estimation_top(int rx_offset,
				int pilot_offset,
				int ignore_prefix,
				int sch_index,
				unsigned char nb_antennas_rx,
				SCH_t sch_type) {

  int aa,i;
  short *rx_buffer;
#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif //USER_MODE
  int number_of_symbols;
  int rx_energy[nb_antennas_rx];
  int n0_energy[nb_antennas_rx];
  int tmp_rx_energy;
  char sch_type_str[8];


  if (sch_type == CHSCH) {
    number_of_symbols = NUMBER_OF_CHSCH_SYMBOLS;
    memcpy(sch_type_str, (char*) "CHSCH", 6);
  }
  else {
    number_of_symbols = NUMBER_OF_SCH_SYMBOLS;
    memcpy(sch_type_str, (char*) "SCH", 4);
  }


  tmp_rx_energy = 0;


  for (aa=0;aa<nb_antennas_rx;aa++) {

    // get frequency-domain representation of CHSCH

    rx_energy[aa] = 0;
    n0_energy[aa] = 1;

    // Compute N0
    if (ignore_prefix == 1) {
      if (mac_xface->is_cluster_head == 0) { // get background noise for MR
	n0_energy[aa] += signal_energy((int *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(CYCLIC_PREFIX_LENGTH+rx_offset+(NUMBER_OF_CHSCH_SYMBOLS_MAX-1) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)%FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX],
				       OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX-CYCLIC_PREFIX_LENGTH);
      }
      else {   // get background noise for CH
	n0_energy[aa] += signal_energy((int *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(CYCLIC_PREFIX_LENGTH+rx_offset+(TX_RX_SWITCH_SYMBOL+5) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)%FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX],
				       OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX-CYCLIC_PREFIX_LENGTH);
      }	
    }
    else {
      if (mac_xface->is_cluster_head == 0) { // get background noise for MR
	n0_energy[aa] += signal_energy((int *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(CYCLIC_PREFIX_LENGTH+rx_offset+(NUMBER_OF_CHSCH_SYMBOLS_MAX-1) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)%FRAME_LENGTH_COMPLEX_SAMPLES],
				       OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES-CYCLIC_PREFIX_LENGTH);
      }
      else {   // get background noise for CH
	n0_energy[aa] += signal_energy((int *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(CYCLIC_PREFIX_LENGTH+rx_offset+(TX_RX_SWITCH_SYMBOL+5) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)%FRAME_LENGTH_COMPLEX_SAMPLES],
				       OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES-CYCLIC_PREFIX_LENGTH);
      }
    }
    
    // Compute SCH energies
    for (i=0;i<number_of_symbols;i++) {

      if (ignore_prefix == 1) {
	rx_buffer = (short *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[rx_offset +CYCLIC_PREFIX_LENGTH +
								  ((i+pilot_offset)<<(LOG2_NUMBER_OF_OFDM_CARRIERS))];

	rx_energy[aa] += signal_energy((int *)rx_buffer,
				       OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX-CYCLIC_PREFIX_LENGTH);

	rx_buffer = (short *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[rx_offset +
								  ((i+pilot_offset)<<(LOG2_NUMBER_OF_OFDM_CARRIERS))];
      }
      else {
	rx_buffer = (short *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[rx_offset + CYCLIC_PREFIX_LENGTH +
								  ((i+pilot_offset)<<(LOG2_NUMBER_OF_OFDM_CARRIERS)) +
								  (i+1+pilot_offset)*CYCLIC_PREFIX_LENGTH];

	rx_energy[aa] += signal_energy((int *)rx_buffer,
				       OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX);

	rx_buffer = (short *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[rx_offset + 
								  ((i+pilot_offset)<<(LOG2_NUMBER_OF_OFDM_CARRIERS)) +
								  (i+1+pilot_offset)*CYCLIC_PREFIX_LENGTH];
      }

    
      if (sch_type == CHSCH) {
	fft(rx_buffer,
	    (short *)&PHY_vars->chsch_data[sch_index].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
	    (short *)twiddle_fft,
	    rev,
	    LOG2_NUMBER_OF_OFDM_CARRIERS,
	    LOG2_NUMBER_OF_OFDM_CARRIERS/2,
	    0); 
	

	phy_channel_estimation((short *)&PHY_vars->chsch_data[sch_index].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       (short *)&PHY_vars->chsch_data[sch_index].channel[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       (short *)&PHY_vars->chsch_data[sch_index].channel_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       (short *)&PHY_vars->chsch_data[sch_index].channel_matched_filter_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       (short *)&PHY_vars->chsch_data[sch_index].CHSCH_conj_f[i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       15,//LOG2_CHSCH_RX_F_AMP,
			       (nb_antennas_rx==1) ? 1 : 0);


			   
      }
      else {
	
	fft(rx_buffer,
	    (short *)&PHY_vars->sch_data[sch_index].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
	    (short *)twiddle_fft,
	    rev,
	    LOG2_NUMBER_OF_OFDM_CARRIERS,
	    LOG2_NUMBER_OF_OFDM_CARRIERS/2,
	    0);
	
	phy_channel_estimation((short *)&PHY_vars->sch_data[sch_index].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       (short *)&PHY_vars->sch_data[sch_index].channel[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       (short *)&PHY_vars->sch_data[sch_index].channel_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       (short *)&PHY_vars->sch_data[sch_index].channel_matched_filter_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       (short *)&PHY_vars->sch_data[sch_index].SCH_conj_f[i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			       15,//LOG2_SCH_RX_F_AMP,
			       (nb_antennas_rx==1) ? 1 : 0);
      }	
    }

#ifdef USER_MODE
#ifdef DEBUG_PHY	    
      if (sch_type == CHSCH) {
	sprintf(fname,"chsch%d_rxsigF%d.m",sch_index,aa);
	sprintf(vname,"chsch%d_rxsF%d",sch_index,aa);
	write_output(fname,vname,
		     (short *)&PHY_vars->chsch_data[sch_index].rx_sig_f[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
	

	sprintf(fname,"chsch%d_channelF%d.m",sch_index,aa);
	sprintf(vname,"chsch%d_chanF%d",sch_index,aa);
	write_output(fname,vname,
		     (short *)&PHY_vars->chsch_data[sch_index].channel_f[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
	
	sprintf(fname,"chsch%d_channel%d.m",sch_index,aa);
	sprintf(vname,"chsch%d_chan%d",sch_index,aa);
	write_output(fname,vname,
		     (short *)&PHY_vars->chsch_data[sch_index].channel[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
      }
      else {
	sprintf(fname,"sch%d_rxsigF%d.m",sch_index,aa);
	sprintf(vname,"sch%d_rxsF%d",sch_index,aa);
	write_output(fname,vname,
		     (short *)&PHY_vars->sch_data[sch_index].rx_sig_f[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
	

	sprintf(fname,"sch%d_channelF%d.m",sch_index,aa);
	sprintf(vname,"sch%d_chanF%d",sch_index,aa);
	write_output(fname,vname,
		     (short *)&PHY_vars->sch_data[sch_index].channel_f[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
	
	sprintf(fname,"sch%d_channel%d.m",sch_index,aa);
	sprintf(vname,"sch%d_chan%d",sch_index,aa);
	write_output(fname,vname,
		     (short *)&PHY_vars->sch_data[sch_index].channel[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
      }	
#endif // DEBUG_PHY
#endif // USER_MODE								    


    rx_energy[aa] /= number_of_symbols;
    n0_energy[aa] /= number_of_symbols;

    // fill the PHY_measurements structure

    PHY_vars->PHY_measurements.rx_power_dB[sch_index][aa] = (short) dB_fixed(rx_energy[aa]); 
    
    PHY_vars->PHY_measurements.rx_rssi_dBm[sch_index][aa] =     
      PHY_vars->PHY_measurements.rx_power_dB[sch_index][aa] -
      PHY_vars->rx_vars[0].rx_total_gain_dB;
    
    tmp_rx_energy += rx_energy[aa];
    
    PHY_vars->PHY_measurements.n0_power_dB[sch_index][aa] = (short) dB_fixed(n0_energy[aa]);
    
    PHY_vars->PHY_measurements.n0_power[sch_index][aa] = n0_energy[aa];
    PHY_vars->PHY_measurements.rx_power[sch_index][aa] = rx_energy[aa];


  }


  phy_subband_powers(sch_index,
		     sch_type,
		     NB_ANTENNAS_TX,
		     nb_antennas_rx,
		     &n0_energy[0]);


  tmp_rx_energy/= nb_antennas_rx;

  PHY_vars->PHY_measurements.rx_avg_power_dB[sch_index]= ((short) dB_fixed(tmp_rx_energy)) - 
    PHY_vars->rx_vars[0].rx_total_gain_dB;

#ifdef DEBUG_PHY
  if ( ( (mac_xface->frame % 100) == 0)) {
	
    for (aa=0;aa<nb_antennas_rx;aa++) 
      {
	msg("[openair][PHY][CHANEST] frame %d, antenna %d: %s %d %d Signal Power = %d dBm (digital = %d (%d dB), rx_gain = %d dB, N0 = %d (%d dB))\n",
	    mac_xface->frame,
	    aa,
	    sch_type_str,
	    sch_index,
	    pilot_offset,
	    PHY_vars->PHY_measurements.rx_rssi_dBm[sch_index][aa],
	    PHY_vars->PHY_measurements.rx_power[sch_index][aa],
	    PHY_vars->PHY_measurements.rx_power_dB[sch_index][aa],
	    PHY_vars->rx_vars[0].rx_total_gain_dB,
	    PHY_vars->PHY_measurements.n0_power[sch_index][aa],
	    PHY_vars->PHY_measurements.n0_power_dB[sch_index][aa]
	    );
      }

  }  
#endif //DEBUG_PHY

}


void phy_channel_estimation(short *rxsig_f,
			    short *channel_est_t,
			    short *channel_est_f,
			    short *channel_matched_filter_f,
			    short *pilot_conj_f, 
			    char log2_pilot_amp, 
			    char smoothen_flag) {
  //		       char not_synched_flag) {
  
  int i;

  /*
  msg("chest: rxf = %x, chestt=%x, chestf=%x,pilf=%x,pilamp=%d,smoothen=%d\n",
      rxsig_f,
      channel_est_t,
      channel_est_f,
      pilot_conj_f,
      log2_pilot_amp,
      smoothen_flag);
  */

  
  // remove estimate in frequency-domain
  mult_cpx_vector(rxsig_f,
		  pilot_conj_f,
		  channel_est_f,
		  NUMBER_OF_OFDM_CARRIERS,
		  log2_pilot_amp);
  
  //go to time-domain for tracking/interpolation
  //    msg("[openair][PHY][SCH %d] frame %d: Decoding -> Channel Estimation 2\n",0,frame);
  
  fft(channel_est_f,
      channel_est_t,
      (short *)twiddle_ifft,
      rev,
      LOG2_NUMBER_OF_OFDM_CARRIERS,
      LOG2_NUMBER_OF_OFDM_CARRIERS/2,
      1);

  if (smoothen_flag == 1) { // do interpolation by time-domain windowing for smoothened estimate in frequency-domain

    // remove noise beyond channel duration
    bzero(channel_est_t,(NUMBER_OF_OFDM_CARRIERS-(2*CYCLIC_PREFIX_LENGTH))*8);
    
    //  msg("[openair][PHY][SCH %d] frame %d: Decoding -> Channel Estimation 3\n",0,frame);
    // go back to frequency domain  
    
    fft(channel_est_t,
	channel_est_f,
	(short *)twiddle_fft,
	rev,
	LOG2_NUMBER_OF_OFDM_CARRIERS,
	LOG2_NUMBER_OF_OFDM_CARRIERS/2,
	1);
  }
  

  // Create conjugate of channel for matched filtering in (Re,Im,Im,Re) format for SIMD complex multiply
  memcpy(channel_matched_filter_f,
	 channel_est_f,
	 2*OFDM_SYMBOL_SIZE_BYTES_NO_PREFIX);

  for (i=0;i<NUMBER_OF_OFDM_CARRIERS;i++) {
    ((short*)channel_matched_filter_f)[2+(i<<2)] = - ((short*)channel_est_f)[1+(i<<2)];
    ((short*)channel_matched_filter_f)[3+(i<<2)] = ((short*)channel_est_f)[0+(i<<2)];   // conjugate channel response
    
  }
  

}

 
void phy_subband_powers(unsigned int sch_index,
			SCH_t sch_type,
			unsigned int nb_antennas_tx,
			unsigned int nb_antennas_rx,
			int *n0) {

  int i,ind,ind2,fg,aatx,aatx2,aa,aaa;

  int temp[4],temp_agg[NB_ANTENNAS_RX][NUMBER_OF_FREQUENCY_GROUPS];
  short *rxsigf;
  int Re,Im;
  int wideband_sinr,sinr_tmp,sinr_fg_max;

    
    
  
  for (aa=0;aa<nb_antennas_rx;aa++) {
    if (sch_type == CHSCH)
      rxsigf = (short *)&PHY_vars->chsch_data[sch_index].rx_sig_f[aa][0];
    else
      rxsigf = (short *)&PHY_vars->sch_data[sch_index].rx_sig_f[aa][0];

    ind = FIRST_CARRIER_OFFSET;
    ind2=0;
    fg = 0;
    for (aatx=0;aatx<nb_antennas_tx;aatx++)
      temp[aatx] = 0;
    for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++)
      temp_agg[aa][i]=0;


    wideband_sinr = 0;

    for (i=0;
	 i<NUMBER_OF_USEFUL_CARRIERS;
	 i+=nb_antennas_tx) {
      
      
      for (aatx=0;
	   aatx<nb_antennas_tx;
	   aatx++) {
	
	Re = (int)rxsigf[ind<<2];
	Im = (int)rxsigf[1+(ind<<2)];
	if (ind==0) { 
	  Re=0;
	  Im=0;
	}
	
	temp[aatx]       += (Re*Re + Im*Im);
	temp_agg[aa][fg] += (Re*Re + Im*Im);
	/*	
	if (fg == 0) {
	  printf("aatx %d, aarx %d, ind %d , temp[aatx] %d (%d),temp_agg[fg] %d(%d)\n",aatx,aa,ind,temp[aatx],dB_fixed(nb_antennas_tx*temp[aatx]/(NUMBER_OF_CARRIERS_PER_GROUP*n0[aa])),temp_agg[fg],dB_fixed(temp_agg[fg]/(NUMBER_OF_CARRIERS_PER_GROUP*n0[aa])),n0[aa]);
	}
	*/

	ind++;
	ind2++;
	

	if (ind==NUMBER_OF_OFDM_CARRIERS)
	  ind=0;
	
	if (ind2==NUMBER_OF_CARRIERS_PER_GROUP) {
	  for (aatx2=0;aatx2<nb_antennas_tx;aatx2++) {
	    if (sch_type == CHSCH)
	      PHY_vars->chsch_data[sch_index].subband_spatial_sinr[aatx2][aa][fg] = dB_fixed(nb_antennas_tx*temp[aatx2]/(n0[aa]*NUMBER_OF_CARRIERS_PER_GROUP));
	    else
	      PHY_vars->sch_data[sch_index].subband_spatial_sinr[aatx2][aa][fg] = dB_fixed(nb_antennas_tx*temp[aatx2]/(n0[aa]*NUMBER_OF_CARRIERS_PER_GROUP));
	    temp[aatx2]=0;
	  }
	  if (aa==(nb_antennas_rx-1)) {

	    // compute maximum SINR over receive antennas
	    sinr_fg_max=0;
	    for (aaa=0;aaa<nb_antennas_rx;aaa++) {
	      sinr_tmp = temp_agg[aa][fg]/(n0[aa]*NUMBER_OF_CARRIERS_PER_GROUP);
	      if (sinr_fg_max < sinr_tmp)
		sinr_fg_max = (sinr_fg_max < sinr_tmp) ? sinr_tmp : sinr_fg_max;
	    }
	    // accumulate sinr for wideband_sinr measurement
	    wideband_sinr+=sinr_fg_max;

	    // Fill measurement information
	    if (sch_type == CHSCH) {
	      PHY_vars->chsch_data[sch_index].subband_aggregate_sinr[fg] = dB_fixed(sinr_fg_max);
	      PHY_vars->chsch_data[sch_index].subband_spatial_sinr[aatx2][aa][fg] = dB_fixed(nb_antennas_tx*temp[aatx2]/(n0[aa]*NUMBER_OF_CARRIERS_PER_GROUP));
	    }
	    else {
	      PHY_vars->sch_data[sch_index].subband_aggregate_sinr[fg] = dB_fixed(sinr_fg_max);
	      PHY_vars->sch_data[sch_index].subband_spatial_sinr[aatx2][aa][fg] = dB_fixed(nb_antennas_tx*temp[aatx2]/(n0[aa]*NUMBER_OF_CARRIERS_PER_GROUP));
	    }

	  }
	  ind2=0;
	  fg++;
	}
      }
    }
    
    
#ifdef DEBUG_PHY

    for (aatx=0;aatx<nb_antennas_tx;aatx++) {
      if (sch_type == CHSCH) {
	msg("[PHY][ESTIMATION] Frame %d: Subband Spatial SINR for CHSCH %d TX %d RX %d: ",mac_xface->frame,sch_index,aatx,aa);
	for (fg=0;fg<NUMBER_OF_FREQUENCY_GROUPS;fg++) {
	  msg("%2d ",PHY_vars->chsch_data[sch_index].subband_spatial_sinr[aatx][aa][fg]);
	}
      }
      else {
	msg("[PHY][ESTIMATION] Frame %d: Subband Spatial SINR for SCH %d TX %d RX %d: ",mac_xface->frame,sch_index,aatx,aa);
	for (fg=0;fg<NUMBER_OF_FREQUENCY_GROUPS;fg++) {
	  msg("%2d ",PHY_vars->sch_data[sch_index].subband_spatial_sinr[aatx][aa][fg]);
	}
      }
 
      msg("\n");
    }
    
#endif    
  }
  
#ifdef DEBUG_PHY
  if (sch_type == CHSCH) {
    msg("[PHY][ESTIMATION] Frame %d: Subband Aggregate SINR for CHSCH %d : ",mac_xface->frame,sch_index);
    for (fg=0;fg<NUMBER_OF_FREQUENCY_GROUPS;fg++) {
      msg("%2d ",PHY_vars->chsch_data[sch_index].subband_aggregate_sinr[fg]);
    }
    msg("\n");
  }
  else {
    msg("[PHY][ESTIMATION] Frame %d: Subband Aggregate SINR for SCH %d : ",mac_xface->frame,sch_index);
    for (fg=0;fg<NUMBER_OF_FREQUENCY_GROUPS;fg++) {
      msg("%2d ",PHY_vars->sch_data[sch_index].subband_aggregate_sinr[fg]);
    }
    msg("\n");
  }
#endif //DEBUG_PHY  

  PHY_vars->chsch_data[sch_index].wideband_sinr = dB_fixed(wideband_sinr/NUMBER_OF_FREQUENCY_GROUPS);


}

/** @} */
