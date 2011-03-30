/*!\brief Initilization routines for PHY SCH*/
///
/// Compute SCH signals for transmission and reception
///

#include "PHY/defs.h"
#include "PHY/types.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"

short tmp_buffer_sch[4*OFDM_SYMBOL_SIZE_SAMPLES_MAX*NUMBER_OF_SCH_SYMBOLS_MAX]__attribute__ ((aligned(16))) ;

  // SCH_f_sync for initial timing acquisition (for clusterheads!) 
void create_times4_sch_symbol(unsigned char n) {

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif //USER_MODE

  int i,aa;

#ifdef DEBUG_PHY
  msg("[openair][PHY][CODING] Initializing SCH_f_sync %d\n",n);
#endif //DEBUG_PHY
  //generate time-domain SCH signal

  memset((char *)tmp_buffer_sch,0,4*OFDM_SYMBOL_SIZE_BYTES*NUMBER_OF_SCH_SYMBOLS);

  for (aa=0;aa<4;aa++) {
    //    msg("n = %d, aa=%d -> %p\n",n,aa,PHY_vars->sch_data[n].SCH_f_sync[aa;


    PHY_ofdm_mod((int *)PHY_vars->sch_data[n].SCH_f_txr[aa],
		 (int *)tmp_buffer_sch,
		 LOG2_NUMBER_OF_OFDM_CARRIERS,
		 NUMBER_OF_SCH_SYMBOLS,
		 CYCLIC_PREFIX_LENGTH,
		 twiddle_ifft,
		 rev,
		 EXTENSION_TYPE);
    

    // go to frequency-domain
    /*
#ifdef DEBUG_PHY
    msg("tmp_buffer_synch = %p, f_sync = %p, twiddle_fft_times4 = %p, rev4 = %p\n",
	tmp_buffer_sch,
	PHY_vars->sch_data[n].SCH_f_sync[aa],
	twiddle_fft_times4,
	rev_times4);
#endif //DEBUG_PHY
    */
    fft(tmp_buffer_sch,
	(short *)PHY_vars->sch_data[n].SCH_f_sync[aa],
	twiddle_fft_times4,
	rev_times4,
	2+LOG2_NUMBER_OF_OFDM_CARRIERS, 
	4,
	0);
    


   // generate special SIMD format (RE -IM IM RE) of conjugate sync symbol
    for (i=0;i<4*NUMBER_OF_OFDM_CARRIERS;i++) {
      ((s16*)PHY_vars->sch_data[n].SCH_f_sync[aa])[2+(i<<2)] = - ((s16*)PHY_vars->sch_data[n].SCH_f_sync[aa])[1+(i<<2)];
      ((s16*)PHY_vars->sch_data[n].SCH_f_sync[aa])[3+(i<<2)] = ((s16*)PHY_vars->sch_data[n].SCH_f_sync[aa])[0+(i<<2)];   
      

     
    }
  }
#ifdef USER_MODE
#ifdef DEBUG_PHY
  sprintf(fname,"SCH%d_sync_4xf0.m",n);
  sprintf(vname,"sch%d_sync_f_4x0",n);

  write_output(fname,vname,
	       (s16 *)PHY_vars->sch_data[n].SCH_f_sync[0],
	       8*NUMBER_OF_OFDM_CARRIERS,
	       2,
	       1);
  sprintf(fname,"SCH%d_sync_4xf1.m",n);
  sprintf(vname,"sch%d_sync_f_4x1",n);

  write_output(fname,vname,
	       (s16 *)PHY_vars->sch_data[n].SCH_f_sync[1],
	       8*NUMBER_OF_OFDM_CARRIERS,
	       2,
	       1);
#endif //DEBUG_PHY
#endif //USER_MODE
   

   
}

/// pre-compute transformed versions of SCH for channel estimation
void phy_sch_init(unsigned char n,unsigned char nb_antennas_tx) {

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif // USER_MODE

  int ind,i,pos;

  short sch_amp;


  Zero_Buffer_nommx((void *)PHY_vars->sch_data[n].SCH_conj_f,
	 2*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_SCH_SYMBOLS);
  Zero_Buffer_nommx((void *)PHY_vars->sch_data[n].SCH_f,
	 2*2*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_SCH_SYMBOLS);
  
  for (i=0;i<nb_antennas_tx;i++) {
    Zero_Buffer_nommx((void *)PHY_vars->sch_data[n].SCH_f_tx[i],
		2*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_SCH_SYMBOLS);
    Zero_Buffer_nommx((void *)PHY_vars->sch_data[n].SCH_f_txr[i],
		2*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_SCH_SYMBOLS);
    Zero_Buffer_nommx((void *)PHY_vars->sch_data[n].SCH_f_sync[i],
		2*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_SCH_SYMBOLS);
  }


  sch_amp = SCH_AMP; //factor nb_antennas_tx is added in phy_generate_sch

  ind = 0;
  
  for (i=0;i<NUMBER_OF_SCH_SYMBOLS*NUMBER_OF_USEFUL_CARRIERS;i++) {
    
    if (i%32 == 0)
      ind++;
    
    pos = (FIRST_CARRIER_OFFSET+i)%NUMBER_OF_OFDM_CARRIERS;
    if (pos == HALF_NUMBER_OF_USEFUL_CARRIERS)
      pos = FIRST_CARRIER_OFFSET;


    ((short *)PHY_vars->sch_data[n].SCH_f)[pos<<1]          = ((((PHY_config->PHY_sch[n].sch_seq_re[ind] >> (i%32))& 1) == 0) ? -ONE_OVER_SQRT2_Q15 : ONE_OVER_SQRT2_Q15);  // Real part
    ((short *)PHY_vars->sch_data[n].SCH_f)[1+(pos<<1)]      = ((((PHY_config->PHY_sch[n].sch_seq_im[ind] >> (i%32))& 1) == 0) ? -ONE_OVER_SQRT2_Q15 : ONE_OVER_SQRT2_Q15);  // Imag part
    
  
    // Special format for SIMD RX routine!
    ((short *)PHY_vars->sch_data[n].SCH_conj_f)[pos<<2]     = ((short *)PHY_vars->sch_data[n].SCH_f)[pos<<1];      // Re
    ((short *)PHY_vars->sch_data[n].SCH_conj_f)[1+(pos<<2)] = ((short *)PHY_vars->sch_data[n].SCH_f)[1+(pos<<1)];  // -Im
    ((short *)PHY_vars->sch_data[n].SCH_conj_f)[2+(pos<<2)] = -((short *)PHY_vars->sch_data[n].SCH_f)[1+(pos<<1)]; // Im
    ((short *)PHY_vars->sch_data[n].SCH_conj_f)[3+(pos<<2)] = ((short *)PHY_vars->sch_data[n].SCH_f)[pos<<1];      // Re

  }

  for (i=0;i<NUMBER_OF_OFDM_CARRIERS;i++) {
  
  
    ((short*)(PHY_vars->sch_data[n].SCH_f_txr[i%4]))[i<<1] = (short)(4*(sch_amp*(int)(((short *)PHY_vars->sch_data[n].SCH_f)[(i<<1)]))>>15);
    ((short*)(PHY_vars->sch_data[n].SCH_f_txr[i%4]))[1+(i<<1)] = (short)(4*(sch_amp*(int)(((short *)PHY_vars->sch_data[n].SCH_f)[1+(i<<1)]))>>15);

  } // 


#ifdef USER_MODE
#ifdef DEBUG_PHY
  sprintf(fname,"sch%d_syncf_tx.m",n);
  sprintf(vname,"sch%d_sync_f_tx",n);

  write_output(fname,vname,
	       (s16 *)PHY_vars->sch_data[n].SCH_f_tx[0],NUMBER_OF_OFDM_CARRIERS,1,1);

  sprintf(fname,"sch%d_syncf_txr.m",n);
  sprintf(vname,"sch%d_sync_f_txr",n);

  write_output(fname,vname,
	       (s16 *)PHY_vars->sch_data[n].SCH_f_txr[0],NUMBER_OF_OFDM_CARRIERS,1,1);

  sprintf(fname,"sch%d_syncf.m",n);
  sprintf(vname,"sch%d_sync_f",n);
  write_output(fname,vname,
	       (s16 *)PHY_vars->sch_data[n].SCH_f,NUMBER_OF_OFDM_CARRIERS,1,1);

#endif //DEBUG_PHY

  create_times4_sch_symbol(n);

#endif //USER_MODE 

  //  msg_nrt("[openair][PHY][Init] SCH Init Done\n");


}

static int UL_GAIN[16] = {128,91,74,64,57,52,48,45,43,40,39,37,35,34,33,32};
unsigned char phy_generate_sch(unsigned int stream_index,
			       unsigned int sch_index,
			       unsigned int symbol, 
			       unsigned short freq_alloc, 
			       unsigned char extension,
			       unsigned char nb_antennas_tx) {

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif //USER_MODE

  int aa,i,j,ii,jj;
  int nb_active_groups=0,amplification_factor;

  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++)
    if ((freq_alloc & (1<<i)) > 0)  // This freq group is allocated to the sch resources
      nb_active_groups++;
  
  amplification_factor = nb_antennas_tx*UL_GAIN[nb_active_groups-1];
  /*  
  msg("SCH %d: freq_alloc %x, gain_lin = %d (%d,%d)\n",sch_index,freq_alloc,
      (short)((((SCH_AMP*(int)(((short*)PHY_vars->sch_data[sch_index].SCH_f)[0]))>>15)*amplification_factor)>>5),
      amplification_factor,nb_active_groups);
  */

#ifdef DEBUG_PHY
    msg("[OPENAIR][PHY][CODING] Generate SCH %d, symbol %d, number of groups %d, amplification %d (%d,%d)\n",sch_index,symbol,nb_active_groups,amplification_factor,NUMBER_OF_OFDM_CARRIERS,NUMBER_OF_CARRIERS_PER_GROUP);
#endif //DEBUG_PHY


  for (aa = 0 ; aa< nb_antennas_tx; aa++) {

    for (j=0;j<NUMBER_OF_OFDM_CARRIERS;j++) {
      PHY_vars->sch_data[sch_index].SCH_f_tx[aa][j] = 0;
    }
  }

  jj=0;


  for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j++) {
    ii=FIRST_CARRIER_OFFSET+j;
    jj=(j+stream_index)%nb_antennas_tx;

    for (i = 0 ; i<NUMBER_OF_FREQUENCY_GROUPS ; i++) {
    
      if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sch resources
	((short *)PHY_vars->sch_data[sch_index].SCH_f_tx[jj])[ii<<1]     = (short)((((SCH_AMP*(int)(((short*)PHY_vars->sch_data[sch_index].SCH_f)[ii<<1]))>>15)*amplification_factor)>>5);
	((short *)PHY_vars->sch_data[sch_index].SCH_f_tx[jj])[1+(ii<<1)] = (short)((((SCH_AMP*(int)(((short*)PHY_vars->sch_data[sch_index].SCH_f)[1+(ii<<1)]))>>15)*amplification_factor)>>5);

	//	msg("SCH %d: Pilot index %d, ant %d -> %d\n",sch_index,ii,jj,((short*)PHY_vars->sch_data[sch_index].SCH_f_tx[jj])[ii<<1]);
      } // freq_alloc_test
      ii=(ii+NUMBER_OF_CARRIERS_PER_GROUP);
      if (ii>=NUMBER_OF_OFDM_CARRIERS)
	ii-=NUMBER_OF_OFDM_CARRIERS;
    } // groups
  } // carriers per group


  for (aa = 0 ; aa< nb_antennas_tx; aa++) {

    PHY_ofdm_mod(PHY_vars->sch_data[sch_index].SCH_f_tx[aa],
		 (int *)&PHY_vars->tx_vars[aa].TX_DMA_BUFFER[(extension == 1) ? symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES :symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX ],   
		 LOG2_NUMBER_OF_OFDM_CARRIERS,
		 1,
		 CYCLIC_PREFIX_LENGTH,
		 twiddle_ifft,
		 rev,
		 (extension== 1) ? EXTENSION_TYPE : NONE
		 );
    
    
#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    
    sprintf(fname,"sch%d_syncf.m",sch_index);
    sprintf(vname,"sch%d_sync_f",sch_index);

    write_output(fname,vname,
		 (s16 *)PHY_vars->sch_data[sch_index].SCH_f,NUMBER_OF_OFDM_CARRIERS,1,1);

    sprintf(fname,"sch%d_tx%d_f.m",sch_index,aa);
    sprintf(vname,"sch%d_tx%dF",sch_index,aa);
    
    write_output(fname,vname,
		 (short *)&PHY_vars->sch_data[sch_index].SCH_f_tx[aa][0],NUMBER_OF_SCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,1,1);
    
    sprintf(fname,"sch%d_conj_f.m",sch_index);
    sprintf(vname,"sch%d_conjF",sch_index);
    
    write_output(fname,vname,
		 (short *)&PHY_vars->sch_data[sch_index].SCH_conj_f[0],2*(NUMBER_OF_SCH_SYMBOLS)*NUMBER_OF_OFDM_CARRIERS,2,1);
    
    sprintf(fname,"sch%d_tx%d.m",sch_index,aa);
    sprintf(vname,"sch%dtx%d",sch_index,aa);
    
    write_output(fname,vname,
		 (short *)&PHY_vars->tx_vars[aa].TX_DMA_BUFFER[((extension == 1) ? (symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES) : (symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX))],
		 ((extension == 1) ? (OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES) : (OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)),
		 1,1);
    
#endif    
#endif
  }
  

  //  PHY_vars->sch_data[sch_index].sch_symbol_pos = symbol;



#ifdef BIT8_TXMUX
  bit8_txmux(((extension == 1) ? OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES : OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)*(NUMBER_OF_SCH_SYMBOLS),
(extension == 1) ? symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES :symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX );
#endif //BIT8_TXMUX


  return(nb_active_groups);

}

#ifndef USER_MODE
/// Function to be called from real-time thread to complete initialization of SCH
/// This is required since using MMX/SSE2 resources from kernel processes is not allowed.

void phy_sch_init_rt_part(unsigned char sch_index) { 

  create_times4_sch_symbol(sch_index);

}      
#endif // USER_MODE

#ifdef OPENAIR2
void fill_sch_measurement_info(unsigned char sach_ind,
			       UL_MEAS *UL_meas,
			       unsigned short freq_alloc) {

  char I0,i,temp,temp2,RSSI;


  //RSSI will contain the maximum RSSI over RX antennas
  //I0 will contain the maximum interference level over RX antennas
  I0 = -104;
  RSSI = -104;
  for (i=0;i<NB_ANTENNAS_RX;i++) {
    
    temp  = PHY_vars->PHY_measurements.rx_rssi_dBm[sach_ind][i];
    temp2 = PHY_vars->PHY_measurements.n0_power_dB[1][i] - PHY_vars->rx_vars[0].rx_total_gain_dB;
    I0 = (temp2>I0) ? temp2 : I0;
    RSSI = (temp>RSSI) ? temp : RSSI;
  }

  // This has to be computed again for the allocated subbands, add this to channel estimation
  UL_meas->Wideband_rssi_dBm               = RSSI;
  UL_meas->Wideband_interference_level_dBm = I0;
  //  UL_meas->Wideband_sinr_dB                = PHY_vars->sch_data[chbch_ind].wideband_sinr;

  // Copy aggregate SINR measurements
  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
    if ((freq_alloc & (1<<i))!=0)
      UL_meas->Sub_band_sinr[i] = PHY_vars->sch_data[sach_ind].subband_aggregate_sinr[i];
    else
      UL_meas->Sub_band_sinr[i] = 0;
  }

}
#endif //OPENAIR2

#ifndef USER_MODE
EXPORT_SYMBOL(phy_generate_sch);
#endif // USER_MODE
