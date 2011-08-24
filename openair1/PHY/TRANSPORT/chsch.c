/*!\brief Initilization routines for PHY CHSCH*/
///
/// Compute CHSCH signals for transmission and reception
///


#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"

#ifndef USER_MODE
#include "SCHED/defs.h"
#else
#define printk printf
#endif


short tmp_buffer_synch[4*OFDM_SYMBOL_SIZE_SAMPLES_MAX*NUMBER_OF_CHSCH_SYMBOLS_MAX]__attribute__ ((aligned(16))) ;

  // CHSCH_f_sync for initial timing acquisition 
void create_times4_sync_symbol(unsigned char n) {

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif //USER_MODE



  int i,aa;

#ifdef DEBUG_PHY
  msg("[openair][PHY][CODING] Initializing CHSCH_f_sync %d\n",n);
#endif
  //generate time-domain CHSCH signal

  memset((char *)tmp_buffer_synch,0,4*OFDM_SYMBOL_SIZE_BYTES*NUMBER_OF_CHSCH_SYMBOLS);

  for (aa=0;aa<4;aa++) {
    //    msg("n = %d, aa=%d -> %p\n",n,aa,PHY_vars->chsch_data[n].CHSCH_f_sync[aa]);


    PHY_ofdm_mod((int *)PHY_vars->chsch_data[n].CHSCH_f_txr[aa],
		 (int *)tmp_buffer_synch,
		 LOG2_NUMBER_OF_OFDM_CARRIERS,
		 NUMBER_OF_CHSCH_SYMBOLS,
		 CYCLIC_PREFIX_LENGTH,
		 twiddle_ifft,
		 rev,
		 EXTENSION_TYPE);
    

    // go to frequency-domain
    /*
    msg("tmp_buffer_synch = %p, f_sync = %p, twiddle_fft_times4 = %p, rev4 = %p\n",
	tmp_buffer_synch,
	PHY_vars->chsch_data[n].CHSCH_f_sync[aa],
	twiddle_fft_times4,
	rev_times4);
    */
    fft(tmp_buffer_synch,
	(short *)PHY_vars->chsch_data[n].CHSCH_f_sync[aa],
	twiddle_fft_times4,
	rev_times4,
	2+LOG2_NUMBER_OF_OFDM_CARRIERS, 
	4,
	0);
    


   // generate special SIMD format (RE -IM IM RE) of conjugate sync symbol
    for (i=0;i<4*NUMBER_OF_OFDM_CARRIERS;i++) {
      ((s16*)PHY_vars->chsch_data[n].CHSCH_f_sync[aa])[2+(i<<2)] = - ((s16*)PHY_vars->chsch_data[n].CHSCH_f_sync[aa])[1+(i<<2)];
      ((s16*)PHY_vars->chsch_data[n].CHSCH_f_sync[aa])[3+(i<<2)] = ((s16*)PHY_vars->chsch_data[n].CHSCH_f_sync[aa])[0+(i<<2)];   
      

     
    }
  }
#ifdef USER_MODE
#ifdef DEBUG_PHY
  sprintf(fname,"CHSCH%d_sync_4xf0.m",n);
  sprintf(vname,"chsch%d_sync_f_4x0",n);

  write_output(fname,vname,
	       (s16 *)PHY_vars->chsch_data[n].CHSCH_f_sync[0],
	       8*NUMBER_OF_OFDM_CARRIERS,
	       2,
	       1);
  sprintf(fname,"CHSCH%d_sync_4xf1.m",n);
  sprintf(vname,"chsch%d_sync_f_4x1",n);

  write_output(fname,vname,
	       (s16 *)PHY_vars->chsch_data[n].CHSCH_f_sync[1],
	       8*NUMBER_OF_OFDM_CARRIERS,
	       2,
	       1);
#endif
#endif //
   

   
}
  
/// pre-compute transformed versions of CHSCH for synch/channel estimation
void phy_chsch_init(unsigned char chbch_ind,
		    unsigned char nb_antennas_tx) {

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif //USER_MODE


  int ind,i,pos;
  short chsch_amp;


  //  short symbol_multiple,freq_multiple;
  //  unsigned short freq_group,position,index,symbol_index,pos2=0;

  //  freq_multiple = INTDEPTH_CHBCH;
  //  symbol_multiple   = NUMBER_OF_OFDM_CARRIERS;


  //  msg_nrt("Clearing CHSCH_conj_f\n");
  Zero_Buffer_nommx((void *)PHY_vars->chsch_data[chbch_ind].CHSCH_conj_f,
	 2*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_CHSCH_SYMBOLS);
  //  msg_nrt("Clearing CHSCH_f\n");
  Zero_Buffer_nommx((void *)PHY_vars->chsch_data[chbch_ind].CHSCH_f,
	 2*2*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_CHSCH_SYMBOLS);

  
  for (i=0;i<nb_antennas_tx;i++){
    //    msg_nrt("Clearing CHSCH_conj_f %d\n",i);
    Zero_Buffer_nommx((void *)PHY_vars->chsch_data[chbch_ind].CHSCH_f_tx[i],
		2*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_CHSCH_SYMBOLS);
    //    msg_nrt("Clearing CHSCH_f_txr %d\n",i);
    Zero_Buffer_nommx((void *)PHY_vars->chsch_data[chbch_ind].CHSCH_f_txr[i],
		2*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_CHSCH_SYMBOLS);
    //    msg_nrt("Clearing CHSCH_f_sync %d\n",i);
    Zero_Buffer_nommx((void *)PHY_vars->chsch_data[chbch_ind].CHSCH_f_sync[i],
		2*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_CHSCH_SYMBOLS);

  }


  chsch_amp = CHSCH_AMP*nb_antennas_tx;

  ind = 0;
  
  for (i=0;i<NUMBER_OF_CHSCH_SYMBOLS*NUMBER_OF_USEFUL_CARRIERS;i++) {
    
    if (i%32 == 0)
      ind++;
    
    pos = (FIRST_CARRIER_OFFSET+i)%NUMBER_OF_OFDM_CARRIERS;
    if (pos == HALF_NUMBER_OF_USEFUL_CARRIERS)
      pos = FIRST_CARRIER_OFFSET;


    ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[pos<<1]          = (((PHY_config->PHY_chsch[chbch_ind].chsch_seq_re[ind] >> (i%32))& 1) == 0) ? -ONE_OVER_SQRT2_Q15 : ONE_OVER_SQRT2_Q15;  // Real part
    ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[1+(pos<<1)]      = (((PHY_config->PHY_chsch[chbch_ind].chsch_seq_im[ind] >> (i%32))& 1) == 0) ? -ONE_OVER_SQRT2_Q15 : ONE_OVER_SQRT2_Q15;  // Imag part
    
  
    // Special format for SIMD RX routine!
    ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_conj_f)[pos<<2]     = ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[pos<<1];      // Re
    ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_conj_f)[1+(pos<<2)] = ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[1+(pos<<1)];  // -Im
    ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_conj_f)[2+(pos<<2)] = -((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[1+(pos<<1)]; // Im
    ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_conj_f)[3+(pos<<2)] = ((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[pos<<1];      // Re


  }

  //  for (aa=0;aa<nb_antennas_tx;aa++)
  //    for (i=0;i<NUMBER_OF_OFDM_CARRIERS;i++)
  //      PHY_vars->chsch_data[chbch_ind].CHSCH_f_tx[aa][i]=0;



  for (i=0;i<NUMBER_OF_OFDM_CARRIERS;i++) {
  
    ((short *)(PHY_vars->chsch_data[chbch_ind].CHSCH_f_tx[i%nb_antennas_tx]))[i<<1]  = (short)((chsch_amp*(int)(((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[i<<1]))>>15);
    ((short *)(PHY_vars->chsch_data[chbch_ind].CHSCH_f_tx[i%nb_antennas_tx]))[1+(i<<1)]  = (short)((chsch_amp*(int)(((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[1+(i<<1)]))>>15);
    ((short *)(PHY_vars->chsch_data[chbch_ind].CHSCH_f_txr[i%4]))[i<<1]                 = (short)(4*(chsch_amp*(int)(((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[(i<<1)]))>>15);
    ((short *)(PHY_vars->chsch_data[chbch_ind].CHSCH_f_txr[i%4]))[1+(i<<1)]             = (short)(4*(chsch_amp*(int)(((short *)PHY_vars->chsch_data[chbch_ind].CHSCH_f)[1+(i<<1)]))>>15);
      
  } // 

  /*
  for (i=0;i<16;i++) {
    printk("CHSCH_f_tx[0][%d]= %x\n",i,PHY_vars->chsch_data[0].CHSCH_f_tx[0][i]);
  }
  */

#ifdef USER_MODE
#ifdef DEBUG_PHY
  for (i=0;i<NB_ANTENNAS_TX;i++) {
    sprintf(fname,"chsch%d_syncf_tx%d.m",chbch_ind,i);
    sprintf(vname,"chsch%d_sync_f_tx%d",chbch_ind,i);

    write_output(fname,vname,
		 (s16 *)PHY_vars->chsch_data[chbch_ind].CHSCH_f_tx[i],
		 NUMBER_OF_OFDM_CARRIERS,1,1);
  }

  sprintf(fname,"chsch%d_syncf_txr.m",chbch_ind);
  sprintf(vname,"chsch%d_sync_f_txr",chbch_ind);

  write_output(fname,vname,
	       (s16 *)PHY_vars->chsch_data[chbch_ind].CHSCH_f_txr[0],NUMBER_OF_OFDM_CARRIERS,1,1);

  sprintf(fname,"chsch%d_syncf.m",chbch_ind);
  sprintf(vname,"chsch%d_sync_f",chbch_ind);
  write_output(fname,vname,
	       (s16 *)PHY_vars->chsch_data[chbch_ind].CHSCH_f,NUMBER_OF_OFDM_CARRIERS,1,1);

#endif //DEBUG_PHY
  create_times4_sync_symbol(chbch_ind);
#endif //USER_MODE 
  //  msg_nrt("[openair][PHY][CHSCH Init] CHSCH Init Done\n");
}

#ifdef OPENAIR2
void fill_chsch_measurement_info(unsigned char chbch_ind,
				 DL_MEAS *DL_meas) {

  char I0,i,temp,temp2,RSSI;


  //RSSI will contain the maximum RSSI over RX antennas
  //I0 will contain the maximum interference level over RX antennas
  I0 = -104;
  RSSI = -104;
  for (i=0;i<NB_ANTENNAS_RX;i++) {
    
    temp  = PHY_vars->PHY_measurements.rx_rssi_dBm[chbch_ind][i];
    temp2 = PHY_vars->PHY_measurements.n0_power_dB[1][i] - PHY_vars->rx_vars[0].rx_total_gain_dB;
    I0 = (temp2>I0) ? temp2 : I0;
    RSSI = (temp>RSSI) ? temp : RSSI;
  }


  DL_meas->Wideband_rssi_dBm               = RSSI;
  DL_meas->Wideband_interference_level_dBm = I0;
  DL_meas->Wideband_sinr_dB                = PHY_vars->chsch_data[chbch_ind].wideband_sinr;

  // Copy aggregate SINR measurements
  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
    DL_meas->Sub_band_sinr[i] = PHY_vars->chsch_data[chbch_ind].subband_aggregate_sinr[i];
  }

}
#endif //OPENAIR2

#ifndef USER_MODE
/// Function to be called from real-time thread to complete initialization of CHSCH
/// This is required since using MMX/SSE2 resources from kernel processes is not allowed.

void phy_chsch_init_rt_part(unsigned char chbch_ind) { 

  create_times4_sync_symbol(chbch_ind);

}      
#endif // USER_MODE


