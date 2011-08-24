#ifndef USER_MODE
#define __NO_VERSION__
#include "rtai_math.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif //CBMIMO1


#else // USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#endif // USER_MODE

#include "PHY/defs.h"
#include "PHY/types.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"
#include "PHY/TOOLS/defs.h"

#ifndef USER_MODE
#include "SCHED/defs.h"
#endif

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif

#ifndef EXPRESSMIMO_TARGET
#include "xmmintrin.h"
#ifdef SSSE3
#include "tmmintrin.h"
#endif
#endif //EXPRESSMIMO_TARGET

#ifndef USER_MODE 
#include <linux/crc32.h>
#else
#define crc32(X,Y,Z) 0
#endif //
//#define crc32(X,Y,Z) 0

// precompute interleaving for chbch


#ifndef USER_MODE
#define openair_get_mbox() (*(unsigned int *)mbox)
#endif //USER_MODE

#ifdef OPENAIR2
/*!\brief This routine implements the MAC interface for the CHBCH on transmission.  It scans the MACPHY_REQ table
@param chbch_ind Index for the chbch to be generated.  The index is in correspondance with an associated chsch_ind.
*/

void phy_generate_chbch_top(unsigned char chbch_ind) {

  unsigned char i; //,j;

  
  for(i=0;i<NB_REQ_MAX;i++) {
         
    /*
      if (((mac_xface->frame/5) % 20) == 0)
      msg("[PHY][CODING] Frame %d: Req %d(%p), Active %d, Type %d\n",
      mac_xface->frame,
      i,
      &Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req,
      Macphy_req_table[0].Macphy_req_table_entry[i].Active,
      (Macphy_req_table[0].Macphy_req_table_entry[i].Active == 1)?
      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type:-1);
        
    */

    if(Macphy_req_table[0].Macphy_req_table_entry[i].Active){
      if (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == CHBCH) {

	
	Macphy_req_table[0].Macphy_req_table_entry[i].Active=0;
	//	Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_req.Phy_Resources_Entry->Active=0;
	Macphy_req_table[0].Macphy_req_cnt--;
	/*
	  if ((mac_xface->frame % 100) == 0) {
	  for (j=0;j<136;j+=4)
	  msg("[PHY][CODING] CHBCH_PDU(%d) %x : %x %x %x %x\n",sizeof(CHBCH_PDU),j,
	  ((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Chbch_pdu)[j],
	  ((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Chbch_pdu)[j+1],
	  ((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Chbch_pdu)[j+2],
	  ((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Chbch_pdu)[j+3]);
	  }
	*/

#ifdef DEBUG_PHY
	if (((mac_xface->frame/5) % 200) == 0)
	  msg("[OPENAIR1][PHY][CODING] Frame %d: Calling generate chbch %d (%d,%d,%d,%d)\n",
	      mac_xface->frame,
	      chbch_ind,
	      NUMBER_OF_CHSCH_SYMBOLS,
	      NUMBER_OF_CHBCH_SYMBOLS,
	      SAMPLE_OFFSET_CHSCH_NO_PREFIX,
	      SAMPLE_OFFSET_CHBCH_NO_PREFIX);
#endif //DEBUG_PHY
	/*	
	if (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Chbch_pdu->Num_ul_sach > 0)
	  msg("[OPENAIR1][PHY][CODING] Frame %d: Chbch has UL_SACH allocation\n", 
	      mac_xface->frame);
	
	*/

	phy_generate_chbch(chbch_ind,
			   0,
			   NB_ANTENNAS_TX,
			   (unsigned char *)Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Chbch_pdu);
      }
    }
  }

}

#endif //OPENAIR2

unsigned char phy_generate_chbch(unsigned char chsch_ind,
				 unsigned char extension_switch,
				 unsigned char nb_antennas_tx,
				 unsigned char *chbch_pdu) {

  unsigned int crc;
  int i,j,n,ii,jj,off,aa;
  short pilot_ind;

  //  int ant_index;
  unsigned char *bch_data = PHY_vars->chbch_data[chsch_ind].encoded_data[0];
  short gain_lin;
  int *fft_input[nb_antennas_tx];
  short *fft_input16[nb_antennas_tx];

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif // USER_MODE

  
  unsigned int chbch_size_bits,chbch_size_bytes;
  unsigned char chbch_power;
  unsigned int tx_energy;


  // Encode data

  chbch_size_bits = (NUMBER_OF_USEFUL_CARRIERS-NUMBER_OF_CHBCH_PILOTS*CHBCH_FREQUENCY_REUSE_FACTOR)*NUMBER_OF_CHBCH_SYMBOLS;
  chbch_size_bytes = chbch_size_bits>>3;

#ifdef DEBUG_PHY

  msg("[openair1][PHY][CODING] Generate CHBCH (%d,%d,%d,%d,%d) for %d antennas, PDU SIZE %d bytes\n",
      chsch_ind,NUMBER_OF_CHSCH_SYMBOLS,NUMBER_OF_CHBCH_SYMBOLS,
      SAMPLE_OFFSET_CHSCH,SAMPLE_OFFSET_CHBCH,nb_antennas_tx,
      chbch_size_bytes);
#endif // DEBUG_PHY

  // scramble data
  

  crc = crc24(chbch_pdu,
 	      (chbch_size_bytes-4)<<3);

  

  // scramble data (without CRC and zero-padding)
  for (i=0;
       i<chbch_size_bytes-4;
       i++) {
    PHY_vars->chbch_data[chsch_ind].tx_pdu[0][i] = chbch_pdu[i]^scrambling_sequence[i+chsch_ind];
  }

  // Place crc
  *(unsigned int *)&PHY_vars->chbch_data[chsch_ind].tx_pdu[0][chbch_size_bytes-4] = crc>>8;

  
  ccodedot11_encode(chbch_size_bytes,
		    PHY_vars->chbch_data[chsch_ind].tx_pdu[0],
		    PHY_vars->chbch_data[chsch_ind].encoded_data[0],
		    0);

#ifdef DEBUG_PHY
#ifdef USER_MODE
  write_output("chbch_encoded_output.m","chbch_encoded_out",
	       PHY_vars->chbch_data[chsch_ind].encoded_data[0],
	       2*NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_CHBCH_SYMBOLS,
	       1,
	       4);

#endif //USER_MODE
#endif //DEBUG_PHY
  // Set up FFT Input buffer
  for (i=0;i<nb_antennas_tx;i++) {
    fft_input[i]   = PHY_vars->chbch_data[chsch_ind].fft_input[i];
    fft_input16[i] = (short *)fft_input[i];
    // clear FFT input buffer (for zero carriers)
    Zero_Buffer((void *)fft_input[i],
		NUMBER_OF_OFDM_CARRIERS_BYTES * NUMBER_OF_CHBCH_SYMBOLS);

  }

  //  printf("NUMBER_OF_OFDM_CARRIERS_BYTES = %d (%p,%p)\n",NUMBER_OF_OFDM_CARRIERS_BYTES,fft_input,beacon_pilot);




  gain_lin = (CHSCH_AMP * ONE_OVER_SQRT2_Q15*nb_antennas_tx)>>15;

  
  for (i=0;i<nb_antennas_tx;i++){

    if (extension_switch == 1) {
      
      PHY_ofdm_mod((int *)PHY_vars->chsch_data[0].CHSCH_f_tx[i],
		   (int *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[SAMPLE_OFFSET_CHSCH],
		   LOG2_NUMBER_OF_OFDM_CARRIERS,
		   NUMBER_OF_CHSCH_SYMBOLS,
		   CYCLIC_PREFIX_LENGTH,
		   twiddle_ifft,
		   rev,
		   EXTENSION_TYPE
		   );
      
      PHY_ofdm_mod((int *)PHY_vars->chsch_data[chsch_ind].CHSCH_f_tx[i],
		   (int *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[((chsch_ind) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)+SAMPLE_OFFSET_CHSCH],
		   LOG2_NUMBER_OF_OFDM_CARRIERS,
		   NUMBER_OF_CHSCH_SYMBOLS,
		   CYCLIC_PREFIX_LENGTH,
		   twiddle_ifft,
		   rev,
		   EXTENSION_TYPE
		   );
    }
    else {
      
      PHY_ofdm_mod((int *)PHY_vars->chsch_data[0].CHSCH_f_tx[i],
		   (int *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[SAMPLE_OFFSET_CHSCH_NO_PREFIX],
		   LOG2_NUMBER_OF_OFDM_CARRIERS,
		   NUMBER_OF_CHSCH_SYMBOLS,
		   CYCLIC_PREFIX_LENGTH,
		   twiddle_ifft,
		   rev,
		   NONE
		   );
      
      PHY_ofdm_mod((int *)PHY_vars->chsch_data[chsch_ind].CHSCH_f_tx[i],
		   (int *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[((chsch_ind) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)+SAMPLE_OFFSET_CHSCH_NO_PREFIX],
		   LOG2_NUMBER_OF_OFDM_CARRIERS,
		   NUMBER_OF_CHSCH_SYMBOLS,
		   CYCLIC_PREFIX_LENGTH,
		   twiddle_ifft,
		   rev,
		   NONE
		   );

    }
  }    
  
  
    // QPSK MODULATION
    jj=0; //CHBCH bit index
    //jjj=0;
    for (n=0;
	 n<NUMBER_OF_CHBCH_SYMBOLS;
	 n++) {
      
      pilot_ind = 0;
      off = n<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS); //offset of current symbol
      
      for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j+=nb_antennas_tx) {
	ii = FIRST_CARRIER_OFFSET+j; //offset of current subcarrier within symbol
	//      jjj = 0;//j % (nb_antennas_tx);// Comment/Uncomment for space-diversity
	for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) { //increment here by frequency reuse factor 
	
	  //	  ii = (FIRST_CARRIER_OFFSET + (i*NUMBER_OF_CARRIERS_PER_GROUP) + j)% NUMBER_OF_OFDM_CARRIERS;
	  //	printf("jj:%d,jjj:%d,ii:%d\n",jj,jjj,ii);
	
	  for (aa=0;aa<nb_antennas_tx;aa++) {
	    if (pilot_ind == CHBCH_FREQUENCY_REUSE_FACTOR*NUMBER_OF_CHBCH_PILOTS) {
	      fft_input16[aa][off+((aa+ii)<<1)]   = (bch_data[jj]==0)                 ? (-gain_lin) : gain_lin;
	      fft_input16[aa][1+off+((aa+ii)<<1)] = (bch_data[jj+chbch_size_bits]==0) ? (-gain_lin) : gain_lin;
	      jj++;
	    }
	    else {  // This is for pilots
	      if (pilot_ind/NUMBER_OF_CHBCH_PILOTS==(chsch_ind-1))
		fft_input[aa][(off>>1)+ii+aa] = PHY_vars->chsch_data[chsch_ind].CHSCH_f_tx[aa][ii+aa]; 
	      else
		fft_input[aa][(off>>1)+ii+aa] = 0; 
	      /*	    
			    #ifdef DEBUG_PHY
			    #ifdef USER_MODE
			    msg("[OPENAIR][PHY][CHBCH] fft_input[%d][%d+%d] = %d\n",aa,(off>>1),ii+aa,fft_input[aa][(off>>1)+ii+aa]);
			    #endif //USER_MODE
			    #endif //DEBUG_PHY
	      */		  
	      pilot_ind++;
	    }
	  }
	  ii=(ii+NUMBER_OF_CARRIERS_PER_GROUP);
	  if (ii>=NUMBER_OF_OFDM_CARRIERS)
	    ii-=NUMBER_OF_OFDM_CARRIERS;	
	} // groups (i)
      
	ii = ii + 1;
	if (ii>NUMBER_OF_OFDM_CARRIERS)
	  ii-=NUMBER_OF_OFDM_CARRIERS;      
      } // carriers per group (j)
    
    
    }   // symbols (n)

    // Add pilots for frequency offset compensation by puncturing encoded stream with CHSCH pilots

    /*
      for (s=0;s<NUMBER_OF_CHBCH_SYMBOLS;s++)
      for (n=0;n<NUMBER_OF_CHBCH_PILOTS;n++){

      pilot_offset = PHY_vars->chbch_data[chsch_ind].pilot_indices[n];

      for (i=0;i<nb_antennas_tx;i++)
      fft_input[i][(s<<LOG2_NUMBER_OF_OFDM_CARRIERS) + pilot_offset] = PHY_vars->chsch_data[chsch_ind].CHSCH_f_tx[i][pilot_offset%NUMBER_OF_OFDM_CARRIERS];


      } 
    */


    tx_energy = 0;
    for (i=0;i<nb_antennas_tx;i++) {
    
      if (extension_switch == 1) {
	PHY_ofdm_mod(fft_input[i],
		     (int *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[SAMPLE_OFFSET_CHBCH],   
		     LOG2_NUMBER_OF_OFDM_CARRIERS,
		     NUMBER_OF_CHBCH_SYMBOLS,
		     CYCLIC_PREFIX_LENGTH,
		     twiddle_ifft,
		     rev,
		     (extension_switch == 1) ? EXTENSION_TYPE : NONE
		     );
      
	tx_energy += signal_energy(&PHY_vars->tx_vars[i].TX_DMA_BUFFER[SAMPLE_OFFSET_CHBCH],
				   (NUMBER_OF_CHBCH_SYMBOLS)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
      }
      else {
	PHY_ofdm_mod(fft_input[i],
		     (int *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[SAMPLE_OFFSET_CHBCH_NO_PREFIX],   
		     LOG2_NUMBER_OF_OFDM_CARRIERS,
		     NUMBER_OF_CHBCH_SYMBOLS,
		     CYCLIC_PREFIX_LENGTH,
		     twiddle_ifft,
		     rev,
		     NONE
		     );
      
	tx_energy += signal_energy(&PHY_vars->tx_vars[i].TX_DMA_BUFFER[SAMPLE_OFFSET_CHBCH_NO_PREFIX],
				   (NUMBER_OF_CHBCH_SYMBOLS)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX);
      
      }
    }
    chbch_power = dB_fixed(tx_energy);

#ifdef BIT8_TXMUX
  bit8_txmux(((extension_switch == 1) ? OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES : OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)*(NUMBER_OF_CHSCH_SYMBOLS_MAX+NUMBER_OF_CHBCH_SYMBOLS),0);
#endif //BIT8_TXMUX


#ifdef DEBUG_PHY
#ifdef USER_MODE  
  for (i=0;i<nb_antennas_tx;i++) {
    sprintf(fname,"chbch%d%d_data.m",chsch_ind,i);
    sprintf(vname,"chbch%d%d_dat",chsch_ind,i);
    write_output(fname,vname,
		 (short *)fft_input[i],
		 NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,
		 1,
		 1);
    sprintf(fname,"chbch%d%d_sig.m",chsch_ind,i);
    sprintf(vname,"chbch%d%d",chsch_ind,i);    
    write_output(fname,vname,
		 (short *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[(extension_switch == 1) ? SAMPLE_OFFSET_CHSCH :SAMPLE_OFFSET_CHSCH_NO_PREFIX],
		 (extension_switch == 1) ? (OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES * (NUMBER_OF_CHSCH + NUMBER_OF_CHBCH_SYMBOLS)): (OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*(NUMBER_OF_CHSCH + NUMBER_OF_CHBCH_SYMBOLS)),
		 1,
#ifdef BIT8_TXMUX
		 5
#else
		 1
#endif
		 );
  }
#endif // USER_MODE
#endif // DEBUG_PHY			 



#ifdef DEBUG_PHY
  if ((mac_xface->frame % 100) == 0)
    msg("[OPENAIR1][PHY][CHBCH %d] Power %d, crc %x\n",chsch_ind,chbch_power,crc);
#endif //DEBUG_PHY
#ifdef BIT8_TXMUX
  chbch_power-=BIT8_TX_SHIFT_DB;
#endif

  return(chbch_power);
}


#ifdef OPENAIR2
void phy_decode_chbch_top(void) {

  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry[2],*req_ptr;

  int crc_status[2];
  unsigned char chsch_indices[2] = {1, 2};
  unsigned char chsch_index;
  unsigned char *chbch_pdu_rx[2];

  int i,chbch_ind=0;
  int rssi1_max,rssi2_max;
  int diff;
  

  rssi1_max = max(PHY_vars->PHY_measurements.rx_rssi_dBm[1][0],PHY_vars->PHY_measurements.rx_rssi_dBm[1][1]);
  rssi2_max = max(PHY_vars->PHY_measurements.rx_rssi_dBm[2][0],PHY_vars->PHY_measurements.rx_rssi_dBm[2][1]);
  


  // get pointers to MAC pdu buffers
  chbch_ind=0;

  //  msg("[PHY][CODING][CHBCH] Searching for CHBCH request\n");


  
  for(i=0;i<NB_REQ_MAX;i++){

    if ( (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type==CHBCH) &&
	 (Macphy_req_table[0].Macphy_req_table_entry[i].Active == 1) ){

      req_ptr = &Macphy_req_table[0].Macphy_req_table_entry[i];
      Macphy_data_req_entry[req_ptr->Macphy_data_req.CH_index] = req_ptr;
      //      msg("CH_index %d\n",req_ptr->Macphy_data_req.CH_index);
      chbch_ind++;

      if (chbch_ind == 2)
	i=NB_REQ_MAX;
    }
  }

  if (chbch_ind != 2) {
    msg("[PHY][CHBCH] TTI %d: Number of CHBCH decode requests not equal to 2 (%d)!!!!\n",mac_xface->frame,chbch_ind);
    return;
  }



  // save MAC pdu pointers
  chbch_pdu_rx[0] = (unsigned char*)Macphy_data_req_entry[0]->Macphy_data_req.Dir.Req_rx.Pdu.Chbch_pdu;
  chbch_pdu_rx[1] = (unsigned char*)Macphy_data_req_entry[1]->Macphy_data_req.Dir.Req_rx.Pdu.Chbch_pdu;

  // now check for RF configuration
  if ( (rssi1_max > CHBCH_RSSI_MIN) && (rssi2_max > CHBCH_RSSI_MIN) ) { // both are there so decode them both
    //    msg("Doing dual_stream\n");
    dual_stream_flag = 1;
    openair_daq_vars.synch_source = 1;

    phy_decode_chbch_2streams_ml(chsch_indices,
				 ML,
				 NB_ANTENNAS_RX,
				 NB_ANTENNAS_TXRX,
				 chbch_pdu_rx,
				 &crc_status[0],
				 sizeof(CHBCH_PDU));
  }

  else if ( (rssi1_max > CHBCH_RSSI_MIN) ) {// CHBCH 1 is there so decode it only
    //    msg("Doing stream 0\n");
    dual_stream_flag = 0;
    openair_daq_vars.synch_source = 1;

    crc_status[0] = phy_decode_chbch(1,NB_ANTENNAS_RX,NB_ANTENNAS_TXRX,chbch_pdu_rx[0],sizeof(CHBCH_PDU));
    crc_status[1] = -1;
  }
    
  else if ( (rssi2_max > CHBCH_RSSI_MIN) ) { // CHBCH 2 is there so decode it only
    //    msg("Doing stream 1\n");
    dual_stream_flag = 0;
    openair_daq_vars.synch_source = 2;

    crc_status[0] = -1;
    crc_status[1] = phy_decode_chbch(2,NB_ANTENNAS_RX,NB_ANTENNAS_TXRX,chbch_pdu_rx[1],sizeof(CHBCH_PDU));
  }
  
  if ((crc_status[0] == -1 )|| (rssi1_max < CHBCH_RSSI_MIN)) {
    PHY_vars->chbch_data[1].pdu_errors++;
    PHY_vars->chbch_data[1].pdu_errors_conseq++;
  }
  else
    PHY_vars->chbch_data[1].pdu_errors_conseq=0;

  if ((crc_status[1] == -1) || (rssi2_max < CHBCH_RSSI_MIN)) {
    PHY_vars->chbch_data[2].pdu_errors++;
    PHY_vars->chbch_data[2].pdu_errors_conseq++;
  }
  else
    PHY_vars->chbch_data[2].pdu_errors_conseq=0;

  if (mac_xface->frame % 128 == 0)
    for (chsch_index = 0; chsch_index<4; chsch_index++) {
      diff = PHY_vars->chbch_data[chsch_index].pdu_errors - PHY_vars->chbch_data[chsch_index].pdu_errors_last;
      PHY_vars->chbch_data[chsch_index].pdu_fer = (diff*100)>>7;
      PHY_vars->chbch_data[chsch_index].pdu_errors_last = PHY_vars->chbch_data[chsch_index].pdu_errors;
    }


  if (PHY_vars->chbch_data[1].pdu_errors_conseq == 10) {
    mac_xface->out_of_sync_ind(0,0);
  }
  if (PHY_vars->chbch_data[2].pdu_errors_conseq == 10) {
    mac_xface->out_of_sync_ind(0,1);
  }
  
  if ((PHY_vars->chbch_data[1].pdu_errors_conseq >= 10) && (PHY_vars->chbch_data[2].pdu_errors_conseq >= 10) ) {
#ifdef DEBUG_PHY
    msg("[OPENAIR1][PHY][CHBCH] Frame %d : consecutive error count reached on both CHBCH, resynchronizing\n",mac_xface->frame);
#endif //DEBUG_PHY
    openair_daq_vars.mode=openair_NOT_SYNCHED;
    openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
    openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1
    mac_xface->frame = -1;
    openair_daq_vars.synch_wait_cnt=0;
    openair_daq_vars.sched_cnt=-1;
  }

#ifdef DEBUG_PHY	
  if (mac_xface->frame % 100 == 0) {
    msg("[PHY][CHBCH] Frame %d : CHBCH %d error count = %d\n",
	mac_xface->frame,
	1,
	PHY_vars->chbch_data[chbch_ind].pdu_errors);
    msg("[PHY][CHBCH] Frame %d : CHBCH %d error count = %d\n",
	mac_xface->frame,
	1,
	PHY_vars->chbch_data[chbch_ind].pdu_errors);
  }
#endif // DEBUG_PHY	

  
  
  Macphy_data_req_entry[0]->Macphy_data_req.Dir.Req_rx.crc_status[0] = crc_status[0];
  Macphy_data_req_entry[1]->Macphy_data_req.Dir.Req_rx.crc_status[0] = crc_status[1];
  /*  
  msg("[PHY][CODING][CHBCH] MACPHY_req_table_entry.CHBCH_PDU=%p,CRC status %d, rssi1_max %d dBm\n",
      Macphy_data_req_entry[0]->Macphy_data_req.Dir.Req_rx.Pdu.Chbch_pdu,
      Macphy_data_req_entry[0]->Macphy_data_req.Dir.Req_rx.crc_status[0],
      rssi1_max);
  */

  fill_chsch_measurement_info(1,
			      Macphy_data_req_entry[0]->Macphy_data_req.Dir.Req_rx.Meas.DL_meas);
  fill_chsch_measurement_info(2,
			      Macphy_data_req_entry[1]->Macphy_data_req.Dir.Req_rx.Meas.DL_meas);
	/*
	if ((mac_xface->frame % 100) == 0) {

	  msg("[PHY][CHBCH] Frame %d: Filled measurement information from CHBCH (%d,%d,%d) : ",
	      mac_xface->frame,
	      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_rssi_dBm,
	      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_interference_level_dBm,
	      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_sinr_dB);

	  for (i=0;i<16;i++)
	    msg("%d ",Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Sub_band_sinr[i]);
	
	}
	*/


  //  msg("CHBCH : Clearing requests : req_cnt %d\n",Macphy_req_table[0].Macphy_req_cnt);
  
  mac_xface->macphy_data_ind(0,
			     &Macphy_data_req_entry[0]->Macphy_data_req.Dir.Req_rx,
			     CHBCH,
			     0);
  
  Macphy_req_table[0].Macphy_req_cnt--;
  Macphy_data_req_entry[0]->Active=0;

  
  mac_xface->macphy_data_ind(0,
			     &Macphy_data_req_entry[1]->Macphy_data_req.Dir.Req_rx,
			     CHBCH,
			     8);
  

  Macphy_req_table[0].Macphy_req_cnt--;
  Macphy_data_req_entry[1]->Active=0;


}

#endif //USER_MODE





#ifndef EXPRESSMIMO_TARGET

#ifdef USER_MODE
void print_bytes(__m128i x,char *s) {

  char *tempb = (char *)&x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",s,
	 tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7],
	 tempb[8],tempb[9],tempb[10],tempb[11],tempb[12],tempb[13],tempb[14],tempb[15]);

}


void print_shorts(__m128i x,char *s) {

  short *tempb = (short *)&x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d\n",s,
	 tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7]
	 );

}

#endif // USER_MODE

static __m64 perror64 __attribute__ ((aligned(16)));
static __m64 Rsymb_conj64 __attribute__ ((aligned(16)));

/*
static short sch_out[256*4] __attribute__ ((aligned(16)));			// Ouput of the phase correction procedure
static short sch_out_prev[256*4] __attribute__ ((aligned(16)));			// Ouput of the phase correction procedure
static short phase_out[256*4] __attribute__ ((aligned(16)));			// Ouput of the phase correction procedure
*/

static unsigned char I0_compensation_table[11] = {0,0,1,1,1,2,2,2,3,3,3};
 
int phy_decode_chbch(unsigned char chbch_ind,
                     unsigned char nb_antennas_rx,
                     unsigned char nb_antennas_tx,
                     unsigned char *chbch_mac_pdu,
                     unsigned int chbch_pdu_length_bytes) { 
  
  unsigned short i,j=0,i2,n,ii,aa,iii,jj,aa_tx;
  
  int *input;
  char *demod_data;
  
  unsigned int oldcrc,crc,rxp[nb_antennas_rx];
  
  
  unsigned int avg,maxh,minh;
  
  unsigned char log2_avg,log2_maxh,log2_perror_amp=0;
  
  short stat;
  
  struct complex16 *Rchsch,*Rsymb;
  __m64 *Rchsch64,temp64;
  register __m64 mm0,mm1;
  
  int ind,ind64;
  int chr,chi,norm;
  int rx_energy[nb_antennas_rx];
  int n0_energy[nb_antennas_rx];
  unsigned char *chbch_pdu;
  unsigned int chbch_size_bits=((NUMBER_OF_USEFUL_CARRIERS-NUMBER_OF_CHBCH_PILOTS*CHBCH_FREQUENCY_REUSE_FACTOR)*NUMBER_OF_CHBCH_SYMBOLS);
  unsigned int chbch_size_bytes;
  unsigned int time_in,time_out;

  unsigned int off,amp_shift,pilot_ind,aatx;
  char *Isymb2;
  int tmp_rx_energy;
  short *Rsymb64_ptr=NULL,*perror64_ptr=NULL;

  __m128i *Csymb[nb_antennas_rx],*Isymb,temp;
  register __m128i xmm0,xmm1,xmm2,xmm3,xmm4;
  unsigned char I0_shift[NB_ANTENNAS_RX];
  char I0_min=0,I0_argmin=0,I0_diff;

  int rate_hz, freq_offset;
  static int freq_offset_filt = 0;
  float phase_offset;
  const float pi = 3.14159265358979;
  struct complex32 phase_temp;
  
#ifdef USER_MODE
  char fname[40],vname[40];
#endif //USER_MODE
  
#ifdef DEBUG_PHY
  msg("[openair][PHY][CODING] Decode CHBCH %d (%d), nb_tx %d, nb_rx %d, rx_offset %d\n",
      chbch_ind,
      (NUMBER_OF_CHSCH_SYMBOLS*NUMBER_OF_CHSCH)+NUMBER_OF_CHBCH_SYMBOLS,
      nb_antennas_tx,nb_antennas_rx,PHY_vars->rx_vars[0].offset);
  
  
#endif // DEBUG_PHY 

  // 1. Measurements

  // fk 20080314:
  // Here we measure the energy on the CHBCH 
  // The measurements on the CHSCH are made in phy_channel_estimation_top 



  /// CHBCH Data detection begins here
  phase_offset = 0;
  /*
  Zero_Buffer(sch_out,NUMBER_OF_OFDM_CARRIERS*8);
  Zero_Buffer(sch_out_prev,NUMBER_OF_OFDM_CARRIERS*8);
  */

  // 2. FFT
  for (aa = 0 ; aa < nb_antennas_rx ; aa++) {


    for (i=0;
         i<(NUMBER_OF_CHBCH_SYMBOLS);
         i++ ){
      
#ifndef USER_MODE      
      if (openair_daq_vars.mode == openair_NOT_SYNCHED) { 
        input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(PHY_vars->rx_vars[0].offset +
                                                      ((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)+
                                                      (i+1+SYMBOL_OFFSET_CHBCH)*CYCLIC_PREFIX_LENGTH)%FRAME_LENGTH_SAMPLES];
      }
      else {
#ifdef HW_PREFIX_REMOVAL
        input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)];
#else
        input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS) +
                                                     (i+SYMBOL_OFFSET_CHBCH+1)*CYCLIC_PREFIX_LENGTH];
#endif //HW_PREFIX_REMOVAL
      }
#else //USER_MODE

      input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(PHY_vars->rx_vars[0].offset +
                                                    ((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)+
                                                    (i+1+SYMBOL_OFFSET_CHBCH)*CYCLIC_PREFIX_LENGTH)%FRAME_LENGTH_SAMPLES];
      
#endif //USER_MODE

      fft((short *)&input[0],
	  (short *)&PHY_vars->chbch_data[chbch_ind].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
	  (short *)twiddle_fft,
	  rev,
	  LOG2_NUMBER_OF_OFDM_CARRIERS,
	  LOG2_NUMBER_OF_OFDM_CARRIERS/2,
	  0);
      

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    sprintf(fname,"chbch_fft%d.m",aa);
    sprintf(vname,"chbch_F%d",aa);
    write_output(fname,vname,
                 (short *)&PHY_vars->chbch_data[chbch_ind].rx_sig_f[aa][0],
                 2*NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);
#endif // DEBUG_PHY
#endif // USER_MODE

    /*  
    // frequency offset estimation (version 1)

    //msg("[PHY][CHBCH PHASE EST] Ant %d symbol %d:\n", aa, i);
 
    if (i>0) {
      mmxcopy(sch_out_prev,sch_out,NUMBER_OF_OFDM_CARRIERS*8);
    }


    Rchsch = (struct complex16 *)&PHY_vars->chsch_data[chbch_ind].CHSCH_conj_f[0];  
    Rsymb  = (struct complex16 *)&PHY_vars->chbch_data[chbch_ind].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];

    mult_cpx_vector((short *)Rsymb,
		    (short *)Rchsch,
		    sch_out,
		    NUMBER_OF_OFDM_CARRIERS,
		    15);

    if (i>0) {
      Zero_Buffer(phase_out,NUMBER_OF_OFDM_CARRIERS*8);
      mult_cpx_vector_h(sch_out,sch_out_prev,phase_out,NUMBER_OF_OFDM_CARRIERS,0,1);
    }

      ind = FIRST_CARRIER_OFFSET;
      for (i2=0;i2<NUMBER_OF_CHBCH_PILOTS;i2+=nb_antennas_tx) {
	if (ind != 0) { // skip DC carrier
	  for (aatx=0;aatx<nb_antennas_tx;aatx++) {
	    msg("[PHY][CHBCH PHASE EST] Ant %d pilot %d: x(t-1) (%d,%d), x(t) (%d,%d), prod (%d, %d)\n",
		aatx, ind,
		sch_out_prev[ind<<2],sch_out_prev[(ind<<2)+1],
		sch_out[ind<<2],sch_out[(ind<<2)+1],
		phase_out[ind<<2],phase_out[(ind<<2)+1]);
	    phase_offset += atan2((double)phase_out[(ind<<2)+1], (double) phase_out[ind<<2]);
	    //msg("[PHY][CHBCH PHASE EST] Ant %d pilot %d: phase_offset = %g\n",aatx,ind,phase_offset);
	    ind++;
	  }
	}
	else { // not a DC carrier
	  ind+=nb_antennas_tx;
	}
	
	ind+=(NUMBER_OF_CARRIERS_PER_GROUP-nb_antennas_tx);
	if (ind>=NUMBER_OF_OFDM_CARRIERS)
	  ind-=NUMBER_OF_OFDM_CARRIERS;
      }
    */

    // 3. Phase error compensation

    Rchsch = (struct complex16 *)&PHY_vars->chsch_data[chbch_ind].rx_sig_f[aa][0];  
    Rsymb  = (struct complex16 *)&PHY_vars->chbch_data[chbch_ind].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
    norm = phy_chbch_phase_comp(Rchsch,
				Rsymb,
				chbch_ind, 
				nb_antennas_tx, 
				&PHY_vars->chbch_data[chbch_ind].perror[aa][i],
				1);

    // For the estimation of the frequency offset we use Eqn 17 of [Kay 1987]. 
    // We multiply the current estimate of the phase with the previous, take the argument and sum up.
    if (i>0) {
      phase_temp.r = (int)PHY_vars->chbch_data[chbch_ind].perror[aa][i-1].r*(int)PHY_vars->chbch_data[chbch_ind].perror[aa][i].r+(int)PHY_vars->chbch_data[chbch_ind].perror[aa][i-1].i*(int)PHY_vars->chbch_data[chbch_ind].perror[aa][i].i;
      phase_temp.i = (int)PHY_vars->chbch_data[chbch_ind].perror[aa][i-1].r*(int)PHY_vars->chbch_data[chbch_ind].perror[aa][i].i-(int)PHY_vars->chbch_data[chbch_ind].perror[aa][i-1].i*(int)PHY_vars->chbch_data[chbch_ind].perror[aa][i].r;
      phase_offset += (atan2(phase_temp.i,phase_temp.r));

#ifdef DEBUG_PHY
      if (mac_xface->frame % 100 == 0) {
	msg("[PHY][CHBCH] Ant %d symbol %d: perror = %d+1j*%d (angle=%d)\n",
	    aa, i,
	    phase_temp.r,
	    phase_temp.i,
	    (int) (phase_offset*(1<<13)));
      }
#endif
    }
    } //symbol i

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    
    /*
    write_output("chsch_conj_F.m","chsch_conjF",
                 (short *)&PHY_vars->chsch_data[chbch_ind].CHSCH_conj_f[0],
                 2*NUMBER_OF_CHSCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);
    */
    
    sprintf(fname,"chbch_channelF%d.m",aa);
    sprintf(vname,"chbch_chanF%d",aa);
    write_output(fname,vname,
                 (short *)&PHY_vars->chsch_data[chbch_ind].channel_f[aa][0],
                 2*NUMBER_OF_OFDM_CARRIERS,2,1);
    
    sprintf(fname,"chbch_channel%d.m",aa);
    sprintf(vname,"chbch_chan%d",aa);
    write_output(fname,vname,
                 (short *)&PHY_vars->chsch_data[chbch_ind].channel[aa][0],
                 2*NUMBER_OF_OFDM_CARRIERS,2,1);
    
#endif // DEBUG_PHY
#endif // USER_MODE

  } // antenna aa

  //time_ns = NS_PER_CHUNK
  //rate_hz = 1/time_ns * 1e9;
#ifdef CLOCK768
  //TBD
#else
  rate_hz = 20313;
#endif

  phase_offset /= (NUMBER_OF_CHBCH_SYMBOLS-1);
  freq_offset =  (int) (phase_offset/(2*pi)*rate_hz);
  freq_offset_filt = ((freq_offset_filt * (1<<14)) + (freq_offset * (1<<14))) >> 15;


  if (mac_xface->frame % 100 == 0) {

    msg("[PHY][CHBCH] Estimated phase/frequency offset = %d, %d (filt: %d) Hz\n", (int)(phase_offset*(1<<13)), freq_offset, freq_offset_filt);

  }



  // check if phase estimate is to weak
#ifndef USER_MODE
  if (norm < 2) {
    
    if (openair_daq_vars.mode == openair_NOT_SYNCHED) {      
#ifdef DEBUG_PHY
      msg("[PHY][CHBCH] frame %d: CHBCH decoding aborted, perror_amp to small ...\n",mac_xface->frame);
#endif //DEBUG_PHY
    }
    return(-1);
  }
#endif //USER_MODE


#ifdef DEBUG_PHY
#ifndef USER_MODE  
  time_out = openair_get_mbox();

  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CODING][CHBCH] Frame %d: chbch transform+CPOC : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif //DEBUG_PHY



  // 4. Channel Compensation
  

  //  msg("chbch: computing average channel strength\n");
  //  msg("[openair][PHY][CHBCH %d] frame %d: Decoding -> Channel Estimation 4\n",chbch_ind,frame);
  avg = 0;
  maxh = 0;

#ifndef USER_MODE
  time_in = openair_get_mbox();
#endif //USER_MODE

  // Compute I0 mismatch between receive antennas
  // I0_min and I0_argmin contain the interference level and index of the antenna with the weakest interference
  I0_min = PHY_vars->PHY_measurements.n0_power_dB[0][0];
  I0_argmin = 0;

  for (aa=1;aa<nb_antennas_rx;aa++){
    if (I0_min > PHY_vars->PHY_measurements.n0_power_dB[0][aa]) {
      I0_argmin = aa;
      I0_min =  PHY_vars->PHY_measurements.n0_power_dB[0][aa];
    }    
  }

  for (aa=0;aa<nb_antennas_rx;aa++) {
    I0_diff = PHY_vars->PHY_measurements.n0_power_dB[0][aa] - I0_min;
    if (I0_diff > 10)
      I0_diff=10;
    I0_shift[aa] = I0_compensation_table[I0_diff];
  }

  aa =I0_argmin;

  for (i=0;i<NUMBER_OF_USEFUL_CARRIERS;i++) {
    ii = (FIRST_CARRIER_OFFSET + i)% NUMBER_OF_OFDM_CARRIERS;
    chr = (unsigned int)((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[0+(ii<<2)];  // real-part
    chi = (unsigned int)((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[1+(ii<<2)];  // -imag-part
    avg += (chr*chr + chi*chi);
    //      maxh = cmax(maxh,chr*chr + chi*chi); 
    //      minh = cmin(minh,chr*chr + chi*chi); 
    
    /*
      ((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[2+(ii<<2)] = - ((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[1+(ii<<2)];
      ((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[3+(ii<<2)] = ((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[0+(ii<<2)];   // conjugate channel response
      
      
      if (i<16)
      msg("%d %d %d %d\n",
      ((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[0+(ii<<2)],
      ((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[1+(ii<<2)],
      ((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[2+(ii<<2)],
      ((short*)PHY_vars->chsch_data[chbch_ind].channel_f[aa])[3+(ii<<2)]);
    */

    }

  //  msg("chbch:avg=%d\n",avg);
  
  
  // find maximum bit position of I/Q components for rescaling
  avg/=(NUMBER_OF_USEFUL_CARRIERS);

  // square-root of compensated average energy
  log2_maxh = log2_approx(avg)/2;// + 12 - 15;



#ifndef USER_MODE
  if (log2_maxh < 1) {
    
    if (openair_daq_vars.mode == openair_NOT_SYNCHED) {      
#ifdef DEBUG_PHY
      msg("[PHY][CHBCH] frame %d: CHBCH decoding aborted, channel_amp to small (%d)...\n",mac_xface->frame,avg);
#endif //DEBUG_PHY
    }
    return(-1);
  }
#endif //USER_MODE

  //  return(-1);

  //  msg("chbch: applying channel comp\n");

  for (aa=0;aa<nb_antennas_rx;aa++){
    for (i=0;
         i<NUMBER_OF_CHBCH_SYMBOLS;
         i++ ){
      
#ifdef USER_MODE
#ifdef DEBUG_PHY
      msg("[openair][PHY][CHBCH %d] Ant %d Compensating symbol %d\n",chbch_ind,aa,i);
#endif //DEBUG_PHY
#endif //USER_MODE

      
      mult_cpx_vector((short *)&PHY_vars->chbch_data[chbch_ind].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
                      (short *)&PHY_vars->chsch_data[chbch_ind].channel_matched_filter_f[aa][0],
                      (short *)&PHY_vars->chbch_data[chbch_ind].rx_sig_f2[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
                      NUMBER_OF_OFDM_CARRIERS,
                      log2_maxh+I0_shift[aa]);   // This is the approximate square-root of the average energy
     

    }
    
    //    msg("[openair][PHY][CHBCH %d] frame %d: Decoding -> Deinterleaving\n",chbch_ind,frame);


#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    
    sprintf(fname,"chbch_demod_output%d.m",aa);
    sprintf(vname,"chbch_demod_out%d",aa);
    
    write_output(fname,vname,
                 &PHY_vars->chbch_data[chbch_ind].rx_sig_f2[aa][0],2*NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);
#endif // DEBUG_PHY
#endif // USER_MODE  
  }


    



#ifdef DEBUG_PHY
#ifndef USER_MODE
  time_out = openair_get_mbox();
  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CODING][CHBCH] Frame %d: chbch channel compensation : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);



  
  time_in = openair_get_mbox();
#endif //USER_MODE
#endif //DEBUG_PHY

  // 5. MR combining + Rescaling


  // Compute overall shift

  amp_shift = 0;//(log2_avg>=9) ? (log2_avg-9) : 0;

  switch (nb_antennas_rx) {
  case 1:
    xmm1 = _mm_cvtsi32_si128(amp_shift);
    break;
  case 2:
  case 3:
    xmm1 = _mm_cvtsi32_si128(1+amp_shift);
    break;
  case 4:
    xmm1 = _mm_cvtsi32_si128(2+amp_shift);
    break;
  default:
    xmm1 = _mm_cvtsi32_si128(amp_shift);
    break;
  }

  Isymb = (__m128i *)PHY_vars->chbch_data[chbch_ind].rx_sig_f4;
  iii=0;
  for (n=0;
       n<NUMBER_OF_CHBCH_SYMBOLS;
       n++) {

    for (aa=0;aa<nb_antennas_rx;aa++) {
      Csymb[aa] = (__m128i *)&PHY_vars->chbch_data[chbch_ind].rx_sig_f2[aa][(n)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
    }
  
    ii = FIRST_CARRIER_OFFSET>>1;   // Point to first useful carrier 



    // NUMBER_OF_USEFUL_CARRIERS MUST BE A MULTIPLE OF 8 TO WORK!!! ELSE FIX WRITES AT END OF LOOP
    for (i = 0 ; i<NUMBER_OF_USEFUL_CARRIERS; i+=8) {

      //            printf("****************n=%d,i=%d,ii=%d\n",n,i,ii);
      xmm0 = _mm_xor_si128(xmm0,xmm0);
      xmm2 = _mm_xor_si128(xmm2,xmm2);
      xmm3 = _mm_xor_si128(xmm3,xmm3);
      xmm4 = _mm_xor_si128(xmm4,xmm4);


      // MR Combining

      //            print_shorts(Csymb[0][ii],"Csymb[0][ii]");
      //      print_shorts(((__m128i *)&PHY_vars->chbch_data[chbch_ind].rx_sig_f2[0][4*ii+(n<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS))])[0],"Csymb[0][ii]");

      for (aa=0;aa<nb_antennas_rx;aa++){
        xmm0 = _mm_adds_epi16(xmm0,Csymb[aa][ii]);
        xmm2 = _mm_adds_epi16(xmm2,Csymb[aa][ii+1]);
        xmm3 = _mm_adds_epi16(xmm3,Csymb[aa][ii+2]);
        xmm4 = _mm_adds_epi16(xmm4,Csymb[aa][ii+3]);
      }
      //     temp = xmm0;
      //      print_shorts(temp,"xmm0=");
      //      temp = xmm2;
      //      print_shorts(temp,"xmm2=");
      //      temp = xmm3;
      //      print_shorts(temp,"xmm3=");
      //      temp = xmm4;
      //      print_shorts(temp,"xmm4=");

      xmm0 = _mm_shuffle_epi32(xmm0,0x88);     // flip the two innermost 32-bit words
      //temp = xmm0;

      //      print_shorts(temp,"xmm0(shuffle)=");

      xmm2 = _mm_shuffle_epi32(xmm2,0x88);     // flip the two innermost 32-bit words

      //      temp = xmm2;
      //      print_shorts(temp,"xmm2(shuffle)=");

      xmm3 = _mm_shuffle_epi32(xmm3,0x88);     // flip the two innermost 32-bit words
      xmm4 = _mm_shuffle_epi32(xmm4,0x88);     // flip the two innermost 32-bit words
      xmm0 = _mm_unpacklo_epi64(xmm0,xmm2);

      //      temp = xmm0;
      //      print_shorts(temp,"xmm0(unpack)=");

      xmm3 = _mm_unpacklo_epi64(xmm3,xmm4);

      //      temp = xmm3;
      //      print_shorts(temp,"xmm3(unpack)=");

      xmm0 = _mm_sra_epi16(xmm0,xmm1);         // remove channel amplitude
      xmm3 = _mm_sra_epi16(xmm3,xmm1);         // remove channel amplitude

      //      temp = xmm0;
      //      print_shorts(temp,"xmm0(shift)=");
      //      temp = xmm3;
      //      print_shorts(temp,"xmm0(shift)=");

      Isymb[iii++] = _mm_packs_epi16(xmm0,xmm3);      // pack to 8 bits with saturation

      //            temp = Isymb[iii-1];
      //            print_bytes(temp,"xmm0(packs)=");

      ii+=4;
      if (ii==(NUMBER_OF_OFDM_CARRIERS>>1))
        ii=0;
      
        
    } // useful carriers (i)

  } //symbols (n) 

  // 6. frequency deinterleaving and demodulation

  demod_data = PHY_vars->chbch_data[chbch_ind].demod_data;
  jj=0;


  for (n=0;
       n<NUMBER_OF_CHBCH_SYMBOLS;
       n++) {

    // Frequency Deinterleaving  
    Isymb2 = (char*)&PHY_vars->chbch_data[chbch_ind].rx_sig_f4[(n*NUMBER_OF_USEFUL_CARRIERS)];
    pilot_ind = 0;

    for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j+=nb_antennas_tx) {
      ii = j;
      for (i = 0 ; i<NUMBER_OF_FREQUENCY_GROUPS ; i++) {
        
        // This is the offset for the jth carrier in the ith group
        //      ii = (FIRST_CARRIER_OFFSET + (i*NUMBER_OF_CARRIERS_PER_GROUP) + j)% NUMBER_OF_OFDM_CARRIERS;
        //      printf("jj:%d(%d),jj+size:%d(%d),ii:%d\n",jj,Isymb2[(ii<<1)],jj+chbch_size_bits,Isymb2[1+(ii<<1)],ii);
        for (aa=0;aa<nb_antennas_tx;aa++) {
          if (pilot_ind == NUMBER_OF_CHBCH_PILOTS*CHBCH_FREQUENCY_REUSE_FACTOR) {
            demod_data[jj]                 = Isymb2[((aa+ii)<<1)]>>4;    // Real component
            demod_data[jj+chbch_size_bits] = Isymb2[((aa+ii)<<1)+1]>>4;  // Imaginary components
            jj++;
          }
          else
            pilot_ind++;
        }
        
        ii+=NUMBER_OF_CARRIERS_PER_GROUP;
      } // groups
    } // carriers per group
  }

#ifdef DEBUG_PHY  
#ifndef USER_MODE
  time_out = openair_get_mbox();
  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CODING][CHBCH] Frame %d: chbch deinterleaving : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif //DEBUG_PHY

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
  write_output("chbch_decode_input.m","chbch_decode_in",
               &PHY_vars->chbch_data[chbch_ind].demod_data[0],
               2*NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_CHBCH_SYMBOLS,1,4);
#endif // DEBUG_PHY
#endif // USER_MODE  

  // 7. Viterbi Decoding


  //  msg("[openair][PHY][CHBCH %d] frame %d: Decoding -> Viterbi, num_bytes %d,%p,%p\n",chbch_ind,frame,mac_xface->mac_tch->bch_rx[chbch_ind].size,mac_xface->mac_tch->bch_rx[chbch_ind].data,PHY_vars->chbch_data[chbch_ind].demod_data);

  chbch_pdu  = PHY_vars->chbch_data[chbch_ind].demod_pdu;
  chbch_size_bytes = chbch_size_bits>>3;



  Zero_Buffer(chbch_pdu,
              chbch_size_bytes+8);



#ifndef USER_MODE
  time_in = openair_get_mbox();
#endif //USER_MODE

  //  printf("chbch_size_bits = %d\n",chbch_size_bits);

  phy_viterbi_dot11_sse2(PHY_vars->chbch_data[chbch_ind].demod_data,
                         chbch_pdu,
                         chbch_size_bits);


#ifndef USER_MODE
  time_out = openair_get_mbox();
#endif // USER_MODE


  
  
  
  // descramble data
  for (i=0;
       i<chbch_size_bytes-4;
       i++) {
    chbch_pdu[i] = chbch_pdu[i] ^ scrambling_sequence[i+chbch_ind];

    //    msg("%x:%x\n",i,chbch_pdu[i]);
    // store the needed part for the MAC
    if (i<chbch_pdu_length_bytes)
      chbch_mac_pdu[i] = chbch_pdu[i];
  }

  oldcrc= *((unsigned int *)(&chbch_pdu[chbch_size_bytes-4]));
  oldcrc&=0x00ffffff;

  crc = crc24(chbch_pdu,
              (chbch_size_bytes-4)<<3)>>8;

#ifdef DEBUG_PHY    
  msg("[OPENAIR][PHY][CHBCH] Received CRC: %x\n",oldcrc);
  msg("[OPENAIR][PHY][CHBCH] Computed CRC : %x\n",crc);
#endif // DEBUG_PHY
  

#ifdef USER_MODE
#ifdef DEBUG_PHY    
  write_output("scrambler.m","scrambling_seq",
               (void*)scrambling_sequence,
               1024,
               1,5);
 
#endif //DEBUG_PHY
#endif //USER_MODE

#ifdef DEBUG_PHY
#ifndef USER_MODE
  time_out = openair_get_mbox();
  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CODING][CHBCH] Frame %d: chbch channel decoding : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif //DEBUG_PHY

  _mm_empty();

  return (crc==oldcrc) ? 0 : -1;
}


void phy_decode_chbch_2streams(unsigned char chbch_ind[2],
			       int mode, //CHBCH_RX_t mode,
#ifdef BIT8_RXDEMUX
			       unsigned char rxdemux_done,
#endif
			       unsigned char nb_antennas_rx,
			       unsigned char nb_antennas_tx,
			       unsigned char *chbch_mac_pdu[2],
			       int ret[2],
			       unsigned int chbch_pdu_length_bytes) { 
  
  unsigned short i,j=0,n,ii,aa,iii,jj,c;
  
  unsigned char chbch_1st;
  
  unsigned int oldcrc,crc;
  
  unsigned char log2_maxh;

  unsigned int avg;

  int norm[2];

  struct complex16 *Rsymb;
  struct complex16 *Rchsch;
  
  int chr,chi;

  unsigned int chbch_size_bits=((NUMBER_OF_USEFUL_CARRIERS-NUMBER_OF_CHBCH_PILOTS*CHBCH_FREQUENCY_REUSE_FACTOR)*NUMBER_OF_CHBCH_SYMBOLS);
  unsigned int chbch_size_bytes;

#ifndef USER_MODE
  unsigned int time_in,time_out;
#endif

  unsigned int pilot_ind,aatx;

  int *input;
  char *demod_data;
  char *Isymb2;
  unsigned char *chbch_pdu;
  int *x[2],*y[2];

  __m128i *Csymb,*Isymb,temp;
  register __m128i xmm0,xmm1,xmm2,xmm3,xmm4;

#ifdef USER_MODE
  char fname[40],vname[40];
#endif //USER_MODE
  
  // 1. Measurements

  // we decode both streams, so we don't need the following code
  /*
  if (PHY_vars->PHY_measurements.rx_avg_power_dB[chbch_ind[0]] > PHY_vars->PHY_measurements.rx_avg_power_dB[chbch_ind[1]]) {
    chbch_1st = chbch_ind[0];
  }  
  else {
    chbch_1st = chbch_ind[1];
  }
  // for debugging we set
  chbch_1st = chbch_ind[0];
  */

#ifdef DEBUG_PHY
  if ((mac_xface->frame % 100) == 0)
    msg("[OPENAIR][PHY][CHBCH] Decoding CHBCH %d and %d, rx_offset %d\n",chbch_ind[0],chbch_ind[1],PHY_vars->rx_vars[0].offset);
#endif
  
  // 2. FFT
  for (aa = 0 ; aa < nb_antennas_rx ; aa++) {


    for (i=0;
	 i<(NUMBER_OF_CHBCH_SYMBOLS);
	 i++ ){
      
#ifndef USER_MODE      
      if (openair_daq_vars.mode == openair_NOT_SYNCHED) { 
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(PHY_vars->rx_vars[0].offset +
						      ((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)+
						      (i+1+SYMBOL_OFFSET_CHBCH)*CYCLIC_PREFIX_LENGTH)%FRAME_LENGTH_SAMPLES];
      }
      else {
#ifdef HW_PREFIX_REMOVAL
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)];
#else
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS) +
						     (i+SYMBOL_OFFSET_CHBCH+1)*CYCLIC_PREFIX_LENGTH];
#endif //HW_PREFIX_REMOVAL
      }
#else //USER_MODE

      input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(PHY_vars->rx_vars[0].offset +
						    ((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)+
						    (i+1+SYMBOL_OFFSET_CHBCH)*CYCLIC_PREFIX_LENGTH)%FRAME_LENGTH_SAMPLES];
      
#endif //USER_MODE

      fft((short *)&input[0],
	  (short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
	  (short *)twiddle_fft,
	  rev,
	  LOG2_NUMBER_OF_OFDM_CARRIERS,
	  LOG2_NUMBER_OF_OFDM_CARRIERS/2,
	  0);

      //copy the result of the fft also to the other chbch_data structure
      /*
      msg("OFDM_SYMBOL_SIZE_BYTES_NO_PREFIX=%d\n",OFDM_SYMBOL_SIZE_BYTES_NO_PREFIX);
      mmxcopy((short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)], 
	      (short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)], 
	      OFDM_SYMBOL_SIZE_BYTES_NO_PREFIX);
      */

      // FIXME: the mmxcopy above does not work for some unkwon reason
      //        therefore we just do another fft for now
      fft((short *)&input[0],
	  (short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
	  (short *)twiddle_fft,
	  rev,
	  LOG2_NUMBER_OF_OFDM_CARRIERS,
	  LOG2_NUMBER_OF_OFDM_CARRIERS/2,
	  0);

      
      // 3. Phase error compensation for both streams

      Rsymb  = (struct complex16 *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
      Rchsch = (struct complex16 *)&PHY_vars->chsch_data[chbch_ind[0]].rx_sig_f[aa][0];	
      norm[0] = phy_chbch_phase_comp(Rchsch,
				     Rsymb,
				     chbch_ind[0], 
				     nb_antennas_tx, 
				     &PHY_vars->chbch_data[chbch_ind[0]].perror[aa][i],
				     1);


#ifdef DEBUG_PHY
#ifdef USER_MODE
      if ((mac_xface->frame % 100) == 0) 
	msg("[OPENAIR][PHY][CHBCH %d] Ant %d : symbol %d, norm = %d perror = (%d,%d)\n",chbch_ind[0],aa,i,norm[0],PHY_vars->chbch_data[chbch_ind[0]].perror[aa][i].r,PHY_vars->chbch_data[chbch_ind[0]].perror[aa][i].i);
#endif //USER_MODE
#endif //DEBUG_PHY

      Rsymb  = (struct complex16 *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
      Rchsch = (struct complex16 *)&PHY_vars->chsch_data[chbch_ind[1]].rx_sig_f[aa][0];	
      norm[1] = phy_chbch_phase_comp(Rchsch,
				     Rsymb,
				     chbch_ind[1], 
				     nb_antennas_tx, 
				     &PHY_vars->chbch_data[chbch_ind[1]].perror[aa][i],
				     1);


#ifdef DEBUG_PHY
#ifdef USER_MODE
      if ((mac_xface->frame % 100) == 0) 
	msg("[OPENAIR][PHY][CHBCH %d] Ant %d : symbol %d, norm = %d perror = (%d,%d)\n",chbch_ind[1],aa,i,norm[1],PHY_vars->chbch_data[chbch_ind[1]].perror[aa][i].r,PHY_vars->chbch_data[chbch_ind[1]].perror[aa][i].i);
#endif //USER_MODE
#endif //DEBUG_PHY
      

    }

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    sprintf(fname,"chbch%d_rxsigF%d.m",chbch_ind[0],aa);
    sprintf(vname,"chbch%d_rxF%d",chbch_ind[0],aa);
    write_output(fname,vname,
		 (short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][0],
		 2*NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);

    sprintf(fname,"chbch%d_rxsigF%d.m",chbch_ind[1],aa);
    sprintf(vname,"chbch%d_rxF%d",chbch_ind[1],aa);
    write_output(fname,vname,
		 (short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f[aa][0],
		 2*NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);

#endif // DEBUG_PHY
#endif // USER_MODE
  }


#ifdef DEBUG_PHY
#ifndef USER_MODE  
  time_out = openair_get_mbox();
  if ((mac_xface->frame % 100) == 0)
    msg("[OPENAIR][PHY][CHBCH] Frame %d: chbch transform+phase compensation : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif


#ifndef USER_MODE
  time_in = openair_get_mbox();
#endif //USER_MODE

  // 4. Channel Compensation
  // 4.1 Compute average channel strenght for resacling
  avg = 0;
  log2_maxh = 0;
  for (aa=0;aa<nb_antennas_rx;aa++) {
    for (aatx=0;aatx<nb_antennas_tx;aatx++) {
      for (i=0;i<NUMBER_OF_USEFUL_CARRIERS;i++) {
	ii = (FIRST_CARRIER_OFFSET + i)% NUMBER_OF_OFDM_CARRIERS;
	chr = (unsigned int)((short*)PHY_vars->chsch_data[1].channel_mmse_filter_f[aatx][aa])[0+(ii<<2)];  // real-part
	chi = (unsigned int)((short*)PHY_vars->chsch_data[1].channel_mmse_filter_f[aatx][aa])[1+(ii<<2)];  // -imag-part
	avg += chr*chr + chi*chi;
      //      maxh = cmax(maxh,chr*chr + chi*chi); 
      //      minh = cmin(minh,chr*chr + chi*chi); 
      }
    }
  }
  
  avg/=(nb_antennas_rx * nb_antennas_tx * NUMBER_OF_USEFUL_CARRIERS);

  log2_maxh = log2_approx(avg)/2+12-14;
  //for debugging lets try
  //log2_maxh = 8;
  
#ifdef DEBUG_PHY
#ifdef USER_MODE  
   msg("[OPENAIR][PHY][CHBCH] Channel norm avg=%d, log2maxh=%d\n",avg,log2_maxh);
#endif //USER_MODE
#endif //DEBUG_PHY

#ifndef USER_MODE
  if (log2_maxh < 1) {
    
    if (openair_daq_vars.mode == openair_NOT_SYNCHED) {      
      msg("[PHY][CHBCH] frame %d: CHBCH decoding aborted, channel_amp to small (%d)...\n",mac_xface->frame,avg);
    }
    ret[0]=-1;
    ret[1]=-1;
    return;
  }
#endif //USER_MODE


#ifdef USER_MODE
#ifdef DEBUG_PHY    
    for (ii=0;ii<NB_ANTENNAS_TXRX;ii++)
      for (jj=0;jj<NB_ANTENNAS_RX;jj++) {


	sprintf(fname,"chsch1_mmse_%d%d.m",ii,jj);
	sprintf(vname,"h%d%d",ii,jj);
	write_output(fname,vname,
		     (s16 *)&PHY_vars->chsch_data[1].channel_mmse_filter_f[ii][jj][0],
		     NUMBER_OF_OFDM_CARRIERS*2,
		     2,
		     1);
      }
#endif //DEBUG_PHY
#endif

    for (c = 0; c<2; c++) {
      if (norm[c]<=2) {
#ifdef DEBUG_PHY
	msg("[PHY][CHBCH] frame %d: CHBCH decoding aborted, perror_amp to small ...\n",mac_xface->frame);
#endif
	ret[c]=-1;
	continue;
      }

      //  4.2 applying MMSE filter


      for (i=0;
	   i<NUMBER_OF_CHBCH_SYMBOLS;
	   i++ ){
      
#ifdef USER_MODE
#ifdef DEBUG_PHY
	msg("[openair][PHY][CHBCH %d] Applying MMSE filter to symbol %d\n",chbch_ind[c],i);
#endif //DEBUG_PHY
#endif //USER_MODE

	x[0] = (int*)&PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f[0][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
	y[0] = (int*)&PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f2[0][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
	x[1] = (int*)&PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f[1][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
	y[1] = (int*)&PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f2[1][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];

	mult_cpx_matrix_vector(PHY_vars->chsch_data[1].channel_mmse_filter_f,
			       x,
			       y,
			       NUMBER_OF_OFDM_CARRIERS,
			       log2_maxh);

	mult_cpx_vector(y[0],PHY_vars->chsch_data[1].idet,y[0],NUMBER_OF_OFDM_CARRIERS,15);
	mult_cpx_vector(y[1],PHY_vars->chsch_data[1].idet,y[1],NUMBER_OF_OFDM_CARRIERS,15);

      }
    


#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    
      for (aa=0;aa<nb_antennas_rx;aa++) {
	sprintf(fname,"chbch%d_mmse_output%d.m",chbch_ind[c],aa);
	sprintf(vname,"chbch%d_mmse_out%d",chbch_ind[c],aa);
    
	write_output(fname,vname,
		     &PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f2[aa][0],
		     2*NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);
	/*
	  write_output(fname,vname,
	  &PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f2[aa][0],
	  2*NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,1,3);
	*/
      }
#endif // DEBUG_PHY
#endif // USER_MODE  


#ifdef DEBUG_PHY
#ifndef USER_MODE
      time_out = openair_get_mbox();
      if ((mac_xface->frame % 100) == 0)
	msg("[PHY][CODING][CHBCH %d] Frame %d: chbch channel compensation : time_in %d,time_out %d\n",chbch_ind[c],mac_xface->frame,time_in,time_out);
      time_in = openair_get_mbox();
#endif //USER_MODE
#endif


      // 5. Clipping 

      Isymb = (__m128i *)PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f4;
      iii=0;
      for (n=0;
	   n<NUMBER_OF_CHBCH_SYMBOLS;
	   n++) {

	Csymb = (__m128i *)&PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f2[c][(n)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
  
	ii = FIRST_CARRIER_OFFSET>>1;   // Point to first useful carrier 


	// NUMBER_OF_USEFUL_CARRIERS MUST BE A MULTIPLE OF 8 TO WORK!!! ELSE FIX WRITES AT END OF LOOP
	for (i = 0 ; i<NUMBER_OF_USEFUL_CARRIERS; i+=8) {

	  xmm0 = Csymb[ii];
	  xmm2 = Csymb[ii+1];
	  xmm3 = Csymb[ii+2];
	  xmm4 = Csymb[ii+3];

	  //     temp = xmm0;
	  //      print_shorts(temp,"xmm0=");
	  //      temp = xmm2;
	  //      print_shorts(temp,"xmm2=");
	  //      temp = xmm3;
	  //      print_shorts(temp,"xmm3=");
	  //      temp = xmm4;
	  //      print_shorts(temp,"xmm4=");

	  xmm0 = _mm_shuffle_epi32(xmm0,0x88);     // flip the two innermost 32-bit words

	  //temp = xmm0;
	  //      print_shorts(temp,"xmm0(shuffle)=");

	  xmm2 = _mm_shuffle_epi32(xmm2,0x88);     // flip the two innermost 32-bit words

	  //      temp = xmm2;
	  //      print_shorts(temp,"xmm2(shuffle)=");

	  xmm3 = _mm_shuffle_epi32(xmm3,0x88);     // flip the two innermost 32-bit words
	  xmm4 = _mm_shuffle_epi32(xmm4,0x88);     // flip the two innermost 32-bit words
	  xmm0 = _mm_unpacklo_epi64(xmm0,xmm2);

	  //      temp = xmm0;
	  //      print_shorts(temp,"xmm0(unpack)=");

	  xmm3 = _mm_unpacklo_epi64(xmm3,xmm4);

	  //      temp = xmm3;
	  //      print_shorts(temp,"xmm3(unpack)=");
	
	  //      xmm0 = _mm_sra_epi16(xmm0,xmm1);         // remove channel amplitude
	  //      xmm3 = _mm_sra_epi16(xmm3,xmm1);         // remove channel amplitude
	
	  //      temp = xmm0;
	  //      print_shorts(temp,"xmm0(shift)=");
	  //      temp = xmm3;
	  //      print_shorts(temp,"xmm0(shift)=");

	  Isymb[iii++] = _mm_packs_epi16(xmm0,xmm3);      // pack to 8 bits with saturation

	  //      temp = Isymb[iii-1];
	  //      print_bytes(temp,"xmm0(packs)=");

	  ii+=4;
	  if (ii==(NUMBER_OF_OFDM_CARRIERS>>1))
	    ii=0;
      
	
	} // useful carriers (i)

      } //symbols (n) 

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    
      sprintf(fname,"chbch%d_clipped_output.m",chbch_ind[c]);
      sprintf(vname,"chbch%d_clipped_out",chbch_ind[c]);
    
      write_output(fname,vname,
		   &PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f4[0],
		   NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_USEFUL_CARRIERS,1,5);
#endif // DEBUG_PHY
#endif // USER_MODE  

      // 6. frequency deinterleaving and demodulation

      demod_data = PHY_vars->chbch_data[chbch_ind[c]].demod_data;
      jj=0;


      for (n=0;
	   n<NUMBER_OF_CHBCH_SYMBOLS;
	   n++) {

	// Frequency Deinterleaving  
	Isymb2 = (char*)&PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f4[(n*NUMBER_OF_USEFUL_CARRIERS)];
	pilot_ind = 0;

	for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j+=nb_antennas_tx) {
	  ii = j;
	  for (i = 0 ; i<NUMBER_OF_FREQUENCY_GROUPS ; i++) {
	
	    // This is the offset for the jth carrier in the ith group
	    //	ii = (FIRST_CARRIER_OFFSET + (i*NUMBER_OF_CARRIERS_PER_GROUP) + j)% NUMBER_OF_OFDM_CARRIERS;
	    //	printf("jj:%d(%d),jj+size:%d(%d),ii:%d\n",jj,Isymb2[(ii<<1)],jj+chbch_size_bits,Isymb2[1+(ii<<1)],ii);
	    for (aa=0;aa<nb_antennas_tx;aa++) {
	      if (pilot_ind == NUMBER_OF_CHBCH_PILOTS*CHBCH_FREQUENCY_REUSE_FACTOR) {
		demod_data[jj]                 = Isymb2[((aa+ii)<<1)]>>4;    // Real component
		demod_data[jj+chbch_size_bits] = Isymb2[((aa+ii)<<1)+1]>>4;  // Imaginary components
		jj++;
	      }
	      else
		pilot_ind++;
	    }
	  
	    ii+=NUMBER_OF_CARRIERS_PER_GROUP;
	  } // groups
	} // carriers per group
      }
  
#ifdef DEBUG_PHY
#ifndef USER_MODE
      time_out = openair_get_mbox();
      if ((mac_xface->frame % 100) == 0)
	msg("[PHY][CODING][CHBCH %d] Frame %d: chbch deinterleaving : time_in %d,time_out %d\n",chbch_ind[c],mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif 

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
      sprintf(fname,"chbch%d_decode_input.m",chbch_ind[c]);
      sprintf(vname,"chbch%d_decode_in",chbch_ind[c]);

      write_output(fname,vname,
		   &PHY_vars->chbch_data[chbch_ind[c]].demod_data[0],
		   2*NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_CHBCH_SYMBOLS,1,4);
#endif // DEBUG_PHY
#endif // USER_MODE

  
      // 7. Viterbi Decoding


      //  msg("[openair][PHY][CHBCH %d] frame %d: Decoding -> Viterbi, num_bytes %d,%p,%p\n",chbch_ind,frame,mac_xface->mac_tch->bch_rx[chbch_ind].size,mac_xface->mac_tch->bch_rx[chbch_ind].data,PHY_vars->chbch_data[chbch_ind].demod_data);

      chbch_pdu  = PHY_vars->chbch_data[chbch_ind[c]].demod_pdu;
      chbch_size_bytes = chbch_size_bits>>3;



      Zero_Buffer(chbch_pdu,
		  chbch_size_bytes+8);



#ifndef USER_MODE
      time_in = openair_get_mbox();
#endif //USER_MODE

      //  printf("chbch_size_bits = %d\n",chbch_size_bits);

      phy_viterbi_dot11_sse2(PHY_vars->chbch_data[chbch_ind[c]].demod_data,
			     chbch_pdu,
			     chbch_size_bits);



   
  
  

      // descramble data
      for (i=0;
	   i<chbch_size_bytes-4;
	   i++) {
	chbch_pdu[i] = chbch_pdu[i] ^ scrambling_sequence[i+chbch_ind[c]];

	//            printf("%d:%d\n",i,chbch_pdu[i]);
	// store the needed part for the MAC
	if (i<chbch_pdu_length_bytes)
	  chbch_mac_pdu[0][i] = chbch_pdu[i];
      }

      oldcrc= *((unsigned int *)(&chbch_pdu[chbch_size_bytes-4]));
      oldcrc&=0x00ffffff;

      crc = crc24(chbch_pdu,
		  (chbch_size_bytes-4)<<3)>>8;

#ifdef DEBUG_PHY    
      if ((mac_xface->frame % 100) == 0) {
	msg("[PHY][CODING][CHBCH %d] Received CRC : %x\n",chbch_ind[c],oldcrc);
	msg("[PHY][CODING][CHBCH %d] Computed CRC : %x\n",chbch_ind[c],crc);
      }
#endif

#ifdef USER_MODE
#ifdef DEBUG_PHY    
      write_output("scrambler.m","scrambling_seq",
		   (void*)scrambling_sequence,
		   1024,
		   1,5);
 
#endif //DEBUG_PHY
#endif //USER_MODE

#ifdef DEBUG_PHY
#ifndef USER_MODE
      time_out = openair_get_mbox();
      if ((mac_xface->frame % 100) == 0)
	msg("[PHY][CODING][CHBCH %d] Frame %d: chbch channel decoding : time_in %d,time_out %d\n",chbch_ind[c],mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif

      ret[c] = (crc==oldcrc) ? 0 : -1;

    }

  _mm_empty();
}


#ifndef SSSE3
#define abs_epi16(x,zero,res,sign)     sign=_mm_cmplt_epi16(x,zero) ; res=_mm_xor_si128(x,sign);   //negate negatives
#endif


static __m128i ONE_OVER_SQRT_8  __attribute__ ((aligned(16))); 



static __m128i rho_rpi __attribute__ ((aligned(16)));
static __m128i  rho_rmi __attribute__ ((aligned(16)));
static __m128i  y0r __attribute__ ((aligned(16)));
static __m128i  y0i __attribute__ ((aligned(16)));
static __m128i  y1r __attribute__ ((aligned(16)));
static __m128i  y1i __attribute__ ((aligned(16)));
static __m128i  y0r_over2 __attribute__ ((aligned(16)));
static __m128i  y0i_over2 __attribute__ ((aligned(16)));
static __m128i  y1r_over2 __attribute__ ((aligned(16)));
static __m128i  y1i_over2 __attribute__ ((aligned(16)));
static __m128i  xmm0 __attribute__ ((aligned(16)));
static __m128i  xmm1 __attribute__ ((aligned(16)));
static __m128i  xmm2 __attribute__ ((aligned(16)));
static __m128i  xmm3 __attribute__ ((aligned(16)));

static __m128i logmax_num_re0 __attribute__ ((aligned(16)));
static __m128i  logmax_num_im0 __attribute__ ((aligned(16)));
static __m128i  logmax_den_re0 __attribute__ ((aligned(16))); 
static __m128i  logmax_den_im0 __attribute__ ((aligned(16)));

static __m128i logmax_num_re1 __attribute__ ((aligned(16)));
static __m128i  logmax_num_im1 __attribute__ ((aligned(16)));
static __m128i  logmax_den_re1 __attribute__ ((aligned(16))); 
static __m128i  logmax_den_im1 __attribute__ ((aligned(16)));

static __m128i A __attribute__ ((aligned(16)));
static __m128i  B __attribute__ ((aligned(16)));
static __m128i  C __attribute__ ((aligned(16)));
static __m128i  D __attribute__ ((aligned(16)));
static __m128i  E __attribute__ ((aligned(16)));
static __m128i  F __attribute__ ((aligned(16)));
static __m128i  G __attribute__ ((aligned(16)));
static __m128i  H __attribute__ ((aligned(16)));

void dual_stream_qpsk_ic(short *stream0_in,
			 short *stream1_in,
			 short *stream0_out,
			 short *stream1_out,
			 short *rho01,
			 short *rho10,
			 int length
			 ) {

  __m128i *rho01_128 = (__m128i *)rho01;
  __m128i *rho10_128 = (__m128i *)rho10;
  __m128i *stream0_128_in = (__m128i *)stream0_in;
  __m128i *stream1_128_in = (__m128i *)stream1_in;
  __m128i *stream0_128_out = (__m128i *)stream0_out;
  __m128i *stream1_128_out = (__m128i *)stream1_out;


  int i;

  ((short*)&ONE_OVER_SQRT_8)[0] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[1] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[2] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[3] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[4] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[5] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[6] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[7] = 23170;

  for (i=0;i<length>>2;i+=2) {



    // STREAM 0

    xmm0 = rho01_128[i];
    xmm1 = rho01_128[i+1];

    // put (rho_r + rho_i)/2sqrt2 in rho_rpi
    // put (rho_r - rho_i)/2sqrt2 in rho_rmi

#ifdef SSSE3   

    rho_rpi = _mm_hadds_epi16(xmm0,xmm1);               
    rho_rmi = _mm_hsubs_epi16(xmm0,xmm1);              
#else


    xmm0 = _mm_shufflelo_epi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm0 = _mm_shufflehi_epi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm0 = _mm_shuffle_epi32(xmm0,0xd8);
    xmm1 = _mm_shufflelo_epi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shufflehi_epi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shuffle_epi32(xmm1,0xd8);
    xmm2 = _mm_unpacklo_epi64(xmm0,xmm1);
    xmm3 = _mm_unpackhi_epi64(xmm0,xmm1);
    rho_rpi = _mm_adds_epi16(xmm2,xmm3);
    rho_rmi = _mm_subs_epi16(xmm2,xmm3);

#endif

    rho_rpi = _mm_mulhi_epi16(rho_rpi,ONE_OVER_SQRT_8);
    rho_rmi = _mm_mulhi_epi16(rho_rmi,ONE_OVER_SQRT_8);

    xmm0 = stream0_128_in[i];
    xmm1 = stream0_128_in[i+1];

    xmm0 = _mm_shufflelo_epi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm0 = _mm_shufflehi_epi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm0 = _mm_shuffle_epi32(xmm0,0xd8);
    xmm1 = _mm_shufflelo_epi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shufflehi_epi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shuffle_epi32(xmm1,0xd8);
    y0r  = _mm_unpacklo_epi64(xmm0,xmm1);
    y0r_over2  = _mm_srai_epi16(y0r,1);
    y0i  = _mm_unpackhi_epi64(xmm0,xmm1);
    y0i_over2  = _mm_srai_epi16(y0i,1);

    xmm0 = stream1_128_in[i];
    xmm1 = stream1_128_in[i+1];


    xmm0 = _mm_shufflelo_epi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm0 = _mm_shufflehi_epi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm0 = _mm_shuffle_epi32(xmm0,0xd8);
    xmm1 = _mm_shufflelo_epi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shufflehi_epi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shuffle_epi32(xmm1,0xd8);
    y1r  = _mm_unpacklo_epi64(xmm0,xmm1);
    y1r_over2  = _mm_srai_epi16(y1r,1);
    y1i  = _mm_unpackhi_epi64(xmm0,xmm1);
    y1i_over2  = _mm_srai_epi16(y1i,1);
    
    // Detection for y0r

    xmm0 = _mm_xor_si128(xmm0,xmm0);

    xmm3 = _mm_subs_epi16(y1r_over2,rho_rpi); 
    abs_epi16(xmm3,xmm0,A,xmm1);       
    xmm2 = _mm_adds_epi16(A,y0i_over2); 
    xmm3 = _mm_subs_epi16(y1i_over2,rho_rmi); 
    abs_epi16(xmm3,xmm0,B,xmm1);       
    logmax_num_re0 = _mm_adds_epi16(B,xmm2); 

    xmm3 = _mm_subs_epi16(y1r_over2,rho_rmi); 
    abs_epi16(xmm3,xmm0,C,xmm1);       
    xmm2 = _mm_subs_epi16(C,y0i_over2); 
    xmm3 = _mm_adds_epi16(y1i_over2,rho_rpi); 
    abs_epi16(xmm3,xmm0,D,xmm1);       
    xmm2 = _mm_adds_epi16(xmm2,D); 
    logmax_num_re0 = _mm_max_epi16(logmax_num_re0,xmm2);  

    xmm3 = _mm_adds_epi16(y1r_over2,rho_rmi); 
    abs_epi16(xmm3,xmm0,E,xmm1);       
    xmm2 = _mm_adds_epi16(E,y0i_over2); 
    xmm3 = _mm_subs_epi16(y1i_over2,rho_rpi); 
    abs_epi16(xmm3,xmm0,F,xmm1);       
    logmax_den_re0 = _mm_adds_epi16(F,xmm2); 

    xmm3 = _mm_adds_epi16(y1r_over2,rho_rpi); 
    abs_epi16(xmm3,xmm0,G,xmm1);       
    xmm2 = _mm_subs_epi16(G,y0i_over2); 
    xmm3 = _mm_adds_epi16(y1i_over2,rho_rmi); 
    abs_epi16(xmm3,xmm0,H,xmm1);       
    xmm2 = _mm_adds_epi16(xmm2,H); 

    logmax_den_re0 = _mm_max_epi16(logmax_den_re0,xmm2);  

    // Detection for y0i

    xmm2 = _mm_adds_epi16(A,y0r_over2); 
    logmax_num_im0 = _mm_adds_epi16(B,xmm2); 

    xmm2 = _mm_subs_epi16(E,y0r_over2); 
    xmm2 = _mm_adds_epi16(xmm2,F); 

    logmax_num_im0 = _mm_max_epi16(logmax_num_im0,xmm2);

    xmm2 = _mm_adds_epi16(C,y0r_over2); 
    logmax_den_im0 = _mm_adds_epi16(D,xmm2); 

    xmm2 = _mm_subs_epi16(G,y0r_over2); 
    xmm2 = _mm_adds_epi16(xmm2,H); 

    logmax_den_im0 = _mm_max_epi16(logmax_den_im0,xmm2);  

    // STREAM 1

    xmm0 = rho10_128[i];
    xmm1 = rho10_128[i+1];
   
#ifdef SSSE3   

    rho_rpi = _mm_hadds_epi16(xmm0,xmm1);               
    rho_rmi = _mm_hsubs_epi16(xmm0,xmm1);              
#else


    xmm0 = _mm_shufflelo_epi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm0 = _mm_shufflehi_epi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm0 = _mm_shuffle_epi32(xmm0,0xd8);
    xmm1 = _mm_shufflelo_epi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shufflehi_epi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shuffle_epi32(xmm1,0xd8);
    xmm2 = _mm_unpacklo_epi64(xmm0,xmm1);
    xmm3 = _mm_unpackhi_epi64(xmm0,xmm1);
    rho_rpi = _mm_adds_epi16(xmm2,xmm3);
    rho_rmi = _mm_subs_epi16(xmm2,xmm3);

#endif

    rho_rpi = _mm_mulhi_epi16(rho_rpi,ONE_OVER_SQRT_8);
    rho_rmi = _mm_mulhi_epi16(rho_rmi,ONE_OVER_SQRT_8);


    // Detection for y1r
    xmm0 = _mm_xor_si128(xmm0,xmm0);

    xmm3 = _mm_subs_epi16(y0r_over2,rho_rpi); 
    abs_epi16(xmm3,xmm0,A,xmm1);       
    xmm2 = _mm_adds_epi16(A,y1i_over2); 
    xmm3 = _mm_subs_epi16(y0i_over2,rho_rmi); 
    abs_epi16(xmm3,xmm0,B,xmm1);       
    logmax_num_re1 = _mm_adds_epi16(B,xmm2); 


    xmm3 = _mm_subs_epi16(y0r_over2,rho_rmi); 
    abs_epi16(xmm3,xmm0,C,xmm1);       
    xmm2 = _mm_subs_epi16(C,y1i_over2); 
    xmm3 = _mm_adds_epi16(y0i_over2,rho_rpi); 
    abs_epi16(xmm3,xmm0,D,xmm1);       
    xmm2 = _mm_adds_epi16(xmm2,D); 
    logmax_num_re1 = _mm_max_epi16(logmax_num_re1,xmm2);  

    xmm3 = _mm_adds_epi16(y0r_over2,rho_rmi); 
    abs_epi16(xmm3,xmm0,E,xmm1);       
    xmm2 = _mm_adds_epi16(E,y1i_over2); 
    xmm3 = _mm_subs_epi16(y0i_over2,rho_rpi); 
    abs_epi16(xmm3,xmm0,F,xmm1);       
    logmax_den_re1 = _mm_adds_epi16(F,xmm2); 


    xmm3 = _mm_adds_epi16(y0r_over2,rho_rpi); 
    abs_epi16(xmm3,xmm0,G,xmm1);       
    xmm2 = _mm_subs_epi16(G,y1i_over2); 
    xmm3 = _mm_adds_epi16(y0i_over2,rho_rmi); 
    abs_epi16(xmm3,xmm0,H,xmm1);       
    xmm2 = _mm_adds_epi16(xmm2,H); 

    logmax_den_re1 = _mm_max_epi16(logmax_den_re1,xmm2);  

    // Detection for y1i

    xmm2 = _mm_adds_epi16(A,y1r_over2); 
    logmax_num_im1 = _mm_adds_epi16(B,xmm2); 

    xmm2 = _mm_subs_epi16(E,y1r_over2); 
    xmm2 = _mm_adds_epi16(xmm2,F); 

    logmax_num_im1 = _mm_max_epi16(logmax_num_im1,xmm2);  


    xmm2 = _mm_adds_epi16(C,y1r_over2); 
    logmax_den_im1 = _mm_adds_epi16(D,xmm2); 



    xmm2 = _mm_subs_epi16(G,y1r_over2); 
    xmm2 = _mm_adds_epi16(xmm2,H); 

    logmax_den_im1 = _mm_max_epi16(logmax_den_im1,xmm2);  

    y0r = _mm_adds_epi16(y0r,logmax_num_re0);
    y0r = _mm_subs_epi16(y0r,logmax_den_re0);
    y0i = _mm_adds_epi16(y0i,logmax_num_im0);
    y0i = _mm_subs_epi16(y0i,logmax_den_im0);

    y1r = _mm_adds_epi16(y1r,logmax_num_re1);
    y1r = _mm_subs_epi16(y1r,logmax_den_re1);
    y1i = _mm_adds_epi16(y1i,logmax_num_im1);
    y1i = _mm_subs_epi16(y1i,logmax_den_im1);

    stream0_128_out[i] = _mm_unpacklo_epi16(y0r,y0i);
    stream0_128_out[i+1] = _mm_unpackhi_epi16(y0r,y0i);
    stream1_128_out[i] = _mm_unpacklo_epi16(y1r,y1i);
    stream1_128_out[i+1] = _mm_unpackhi_epi16(y1r,y1i);

  }

  //  exit(0);
}


static short rot_mf0[4096*4] __attribute__ ((aligned(16)));
static short rot_mf1[4096*4] __attribute__ ((aligned(16)));
static short rho01_tmp[4096*4] __attribute__ ((aligned(16)));
static short rho10_tmp[4096*4] __attribute__ ((aligned(16)));
static short rho01[4096*4] __attribute__ ((aligned(16)));
static short rho10[4096*4] __attribute__ ((aligned(16)));

void phy_decode_chbch_2streams_ml(unsigned char chbch_ind[2],
				  int mode, //CHBCH_RX_t mode,
				  unsigned char nb_antennas_rx,
				  unsigned char nb_antennas_tx,
				  unsigned char *chbch_mac_pdu[2],
				  int ret[2],
				  unsigned int chbch_pdu_length_bytes) { 
  
  unsigned short i,j=0,n,ii,aa,iii,jj,c;
  
  unsigned char chbch_1st;
  
  unsigned int oldcrc,crc;
  
  unsigned char log2_maxh;

  unsigned int avg[2];

  int norm[2];

  struct complex16 *Rsymb;
  struct complex16 *Rchsch;
  
  int chr,chi;

  unsigned int chbch_size_bits=((NUMBER_OF_USEFUL_CARRIERS-NUMBER_OF_CHBCH_PILOTS*CHBCH_FREQUENCY_REUSE_FACTOR)*NUMBER_OF_CHBCH_SYMBOLS);
  unsigned int chbch_size_bytes;

#ifndef USER_MODE
  unsigned int time_in,time_out;
#endif

  unsigned int pilot_ind,aatx;

  int *input;
  char *demod_data;
  char *Isymb2;
  unsigned char *chbch_pdu;
  int *x[2],*y[2];

  __m128i *Csymb,*Isymb,temp;
  register __m128i xmm0,xmm1,xmm2,xmm3,xmm4;
  unsigned char I0_shift[NB_ANTENNAS_RX];
  char I0_min=0,I0_argmin=0,I0_diff;

#ifdef USER_MODE
  char fname[40],vname[40];
#endif //USER_MODE
  
#ifdef DEBUG_PHY
  if ((mac_xface->frame % 100) == 0)
    msg("[OPENAIR][PHY][CHBCH] Decoding CHBCH %d and %d, rx_offset %d\n",chbch_ind[0],chbch_ind[1],PHY_vars->rx_vars[0].offset);
#endif
  
  // 2. FFT
  for (aa = 0 ; aa < nb_antennas_rx ; aa++) {


    for (i=0;
	 i<(NUMBER_OF_CHBCH_SYMBOLS);
	 i++ ){
      
#ifndef USER_MODE      
      if (openair_daq_vars.mode == openair_NOT_SYNCHED) { 
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(PHY_vars->rx_vars[0].offset +
						      ((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)+
						      (i+1+SYMBOL_OFFSET_CHBCH)*CYCLIC_PREFIX_LENGTH)%FRAME_LENGTH_SAMPLES];
      }
      else {
#ifdef HW_PREFIX_REMOVAL
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)];
#else
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS) +
						     (i+SYMBOL_OFFSET_CHBCH+1)*CYCLIC_PREFIX_LENGTH];
#endif //HW_PREFIX_REMOVAL
      }
#else //USER_MODE

      input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(PHY_vars->rx_vars[0].offset +
						    ((i+SYMBOL_OFFSET_CHBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)+
						    (i+1+SYMBOL_OFFSET_CHBCH)*CYCLIC_PREFIX_LENGTH)%FRAME_LENGTH_SAMPLES];
      
#endif //USER_MODE

      fft((short *)&input[0],
	  (short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
	  (short *)twiddle_fft,
	  rev,
	  LOG2_NUMBER_OF_OFDM_CARRIERS,
	  LOG2_NUMBER_OF_OFDM_CARRIERS/2,
	  0);

      // 3. Phase error compensation for both streams

      Rsymb  = (struct complex16 *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
      Rchsch = (struct complex16 *)&PHY_vars->chsch_data[chbch_ind[0]].rx_sig_f[aa][0];	
      norm[0] = phy_chbch_phase_comp(Rchsch,
				     Rsymb,
				     chbch_ind[0], 
				     nb_antennas_tx, 
				     &PHY_vars->chbch_data[chbch_ind[0]].perror[aa][i],
				     0);


#ifdef DEBUG_PHY
#ifdef USER_MODE
      if ((mac_xface->frame % 100) == 0) 
	msg("[OPENAIR][PHY][CHBCH %d] Ant %d : symbol %d, norm = %d perror = (%d,%d)\n",chbch_ind[0],aa,i,norm[0],PHY_vars->chbch_data[chbch_ind[0]].perror[aa][i].r,PHY_vars->chbch_data[chbch_ind[0]].perror[aa][i].i);
#endif //USER_MODE
#endif //DEBUG_PHY

      Rchsch = (struct complex16 *)&PHY_vars->chsch_data[chbch_ind[1]].rx_sig_f[aa][0];	
      norm[1] = phy_chbch_phase_comp(Rchsch,
				     Rsymb,
				     chbch_ind[1], 
				     nb_antennas_tx, 
				     &PHY_vars->chbch_data[chbch_ind[1]].perror[aa][i],
				     0);
      

#ifdef DEBUG_PHY
#ifdef USER_MODE
      if ((mac_xface->frame % 100) == 0) 
	msg("[OPENAIR][PHY][CHBCH %d] Ant %d : symbol %d, norm = %d perror = (%d,%d)\n",chbch_ind[1],aa,i,norm[1],PHY_vars->chbch_data[chbch_ind[1]].perror[aa][i].r,PHY_vars->chbch_data[chbch_ind[1]].perror[aa][i].i);
#endif //USER_MODE
#endif //DEBUG_PHY
      

    }

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    sprintf(fname,"chbch%d_rxsigF%d.m",chbch_ind[0],aa);
    sprintf(vname,"chbch%d_rxF%d",chbch_ind[0],aa);
    write_output(fname,vname,
		 (short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][0],
		 2*NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);

    sprintf(fname,"chbch%d_rxsigF%d.m",chbch_ind[1],aa);
    sprintf(vname,"chbch%d_rxF%d",chbch_ind[1],aa);
    write_output(fname,vname,
		 (short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f[aa][0],
		 2*NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);

#endif // DEBUG_PHY
#endif // USER_MODE
  }


#ifdef DEBUG_PHY
#ifndef USER_MODE  
  time_out = openair_get_mbox();
  if ((mac_xface->frame % 100) == 0)
    msg("[OPENAIR][PHY][CHBCH] Frame %d: chbch transform+phase compensation : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif


#ifndef USER_MODE
  time_in = openair_get_mbox();
#endif //USER_MODE

  // 4. Channel Compensation

  // Compute I0 mismatch between receive antennas
  // I0_min and I0_argmin contain the interference level and index of the antenna with the weakest interference
  I0_min = PHY_vars->PHY_measurements.n0_power_dB[0][0];
  I0_argmin = 0;

  for (aa=1;aa<nb_antennas_rx;aa++){
    if (I0_min > PHY_vars->PHY_measurements.n0_power_dB[0][aa]) {
      I0_argmin = aa;
      I0_min =  PHY_vars->PHY_measurements.n0_power_dB[0][aa];
    }    
  }

  for (aa=0;aa<nb_antennas_rx;aa++) {
    I0_diff = PHY_vars->PHY_measurements.n0_power_dB[0][aa] - I0_min;
    if (I0_diff > 10)
      I0_diff=10;
    I0_shift[aa] = I0_compensation_table[I0_diff];


#ifdef USER_MODE
#ifdef DEBUG_PHY
    printf("I0_shift[%d] = %d\n",aa,I0_shift[aa]);
#endif //DEBUG_PHY
#endif //USER_MODE
  }

  // 4.1 Compute average channel strength for rescaling

  log2_maxh = 0;

  for (c = 0; c<2; c++) {  
    avg[c] = 0;
    
    
    
    for (aa=0;aa<nb_antennas_rx;aa++) {
      for (i=0;i<NUMBER_OF_USEFUL_CARRIERS;i++) {
	ii = (FIRST_CARRIER_OFFSET + i)% NUMBER_OF_OFDM_CARRIERS;
	chr = (unsigned int)((short*)PHY_vars->chsch_data[chbch_ind[c]].channel_matched_filter_f[aa])[0+(ii<<2)];  // real-part
	chi = (unsigned int)((short*)PHY_vars->chsch_data[chbch_ind[c]].channel_matched_filter_f[aa])[1+(ii<<2)];  // -imag-part
	avg[c] += chr*chr + chi*chi;
      }
    }
    
    avg[c]/=(nb_antennas_rx * nb_antennas_tx * NUMBER_OF_USEFUL_CARRIERS);
    
    if (log2_maxh < ((log2_approx(avg[c]/2))))
      log2_maxh = ((log2_approx(avg[c])/2));
  
#ifdef DEBUG_PHY
#ifdef USER_MODE  
   msg("[OPENAIR][PHY][CHBCH] CHBCH IND %d Channel norm avg=%d, log2maxh=%d\n",chbch_ind[c],avg[c],log2_maxh);
#endif //USER_MODE
#endif //DEBUG_PHY
  } // loop on indices

#ifndef USER_MODE
  if (log2_maxh < 1)  {
    
    if (openair_daq_vars.mode == openair_NOT_SYNCHED) {      
      msg("[PHY][CHBCH] frame %d: CHBCH decoding aborted, channel_amp to small (%d)...\n",mac_xface->frame,avg);
    }
    ret[0]=-1;
    ret[1]=-1;
    return;
  }
#endif //USER_MODE


  for (c = 0; c<2; c++) {
    if (norm[c]<=2) {
#ifdef DEBUG_PHY
      msg("[PHY][CHBCH] frame %d: CHBCH decoding aborted, perror_amp to small ...\n",mac_xface->frame);
#endif
      ret[c]=-1;
      continue;
    }
  }

    //  4.2 applying matched filter
    
    
  for (i=0;
       i<NUMBER_OF_CHBCH_SYMBOLS;
       i++ ){
    

    
    for (aa=0;aa<NB_ANTENNAS_RX;aa++) {

    // first rotate matched filters
      rotate_cpx_vector((short *)&PHY_vars->chsch_data[chbch_ind[0]].channel_matched_filter_f[aa][0],
			(short *)&PHY_vars->chbch_data[chbch_ind[0]].perror[aa][i],
			(short *)rot_mf0,
			NUMBER_OF_OFDM_CARRIERS,
			5,
			1);
      rotate_cpx_vector((short *)&PHY_vars->chsch_data[chbch_ind[1]].channel_matched_filter_f[aa][0],
			(short *)&PHY_vars->chbch_data[chbch_ind[1]].perror[aa][i],
			(short *)rot_mf1,
			NUMBER_OF_OFDM_CARRIERS,
			5,
			1);
    
      // matched filter stream 0

      mult_cpx_vector_norep((short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			    rot_mf0,
			    (short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f2[aa][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
			    NUMBER_OF_OFDM_CARRIERS,
			    log2_maxh+I0_shift[aa]);   
        
      if (aa>0)
	add_vector16((short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f2[0][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
		     (short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f2[aa][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
		     (short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f2[0][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
		     NUMBER_OF_OFDM_CARRIERS<<1);
      
      // matched filter stream 1

      mult_cpx_vector_norep((short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
			    rot_mf1,
			    (short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f2[aa][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
			    NUMBER_OF_OFDM_CARRIERS,
			    log2_maxh+I0_shift[aa]);   

      if (aa>0)
	add_vector16((short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f2[0][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
		     (short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f2[aa][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
		     (short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f2[0][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
		     NUMBER_OF_OFDM_CARRIERS<<1);

      
      if (aa==0) {

	// compute complex correlation between stream 0 and 1 relative to strength of stream 0

	mult_cpx_vector_norep_conj(rot_mf1,
				   rot_mf0,
				   rho01,
				   NUMBER_OF_OFDM_CARRIERS,
				   log2_maxh+I0_shift[aa]);

	// compute complex correlation between stream 0 and 1 relative to strength of stream 1 
	// note : this is the conjugate of the desired correlation)

	mult_cpx_vector_norep_conj(rot_mf0,
				   rot_mf1,
				   rho10,
				   NUMBER_OF_OFDM_CARRIERS,
				   log2_maxh+I0_shift[aa]);
      }
      else {

      // compute complex correlation between stream 0 and 1 relative to strength of stream 0

	mult_cpx_vector_norep_conj(rot_mf1,
				   rot_mf0,
				   rho01_tmp,
				   NUMBER_OF_OFDM_CARRIERS,
				   log2_maxh+I0_shift[aa]);

	add_vector16(rho01,
		     rho01_tmp,
		     rho01,
		     NUMBER_OF_OFDM_CARRIERS<<1);

	// compute complex correlation between stream 0 and 1 relative to strength of stream 1 
	// note : this is the conjugate of the desired correlation)

	mult_cpx_vector_norep_conj(rot_mf0,
				   rot_mf1,
				   rho10_tmp,
				   NUMBER_OF_OFDM_CARRIERS,
				   log2_maxh+I0_shift[aa]);
 
	add_vector16(rho10,
		     rho10_tmp,
		     rho10,
		     NUMBER_OF_OFDM_CARRIERS<<1);
      


      }
      

    } //loop on RX antennas
    
    dual_stream_qpsk_ic((short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f2[0][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
			(short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f2[0][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
			(short *)&PHY_vars->chbch_data[chbch_ind[0]].rx_sig_f2[1][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
			(short *)&PHY_vars->chbch_data[chbch_ind[1]].rx_sig_f2[1][i<<(LOG2_NUMBER_OF_OFDM_CARRIERS)],
			rho01,
			rho10,
			NUMBER_OF_OFDM_CARRIERS
			);
    
  } // loop on symbols

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    
  for (c=0;c<2;c++) {
    sprintf(fname,"chbch%d_mf_output.m",chbch_ind[c]);
    sprintf(vname,"chbch%d_mf_out",chbch_ind[c]);
    
    write_output(fname,vname,
		 &PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f2[0][0],
		 NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,1,1);

    sprintf(fname,"chbch%d_rho_output.m",chbch_ind[c]);
    sprintf(vname,"chbch%d_rho_out",chbch_ind[c]);
    write_output(fname,vname,
		 rho01,
		 NUMBER_OF_OFDM_CARRIERS,1,1);

    sprintf(fname,"chbch%d_mf_output_ic.m",chbch_ind[c]);
    sprintf(vname,"chbch%d_mf_out_ic",chbch_ind[c]);
    
    write_output(fname,vname,
		 &PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f2[1][0],
		 NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,1,1);
  }


#endif // DEBUG_PHY
#endif // USER_MODE  


#ifdef DEBUG_PHY
#ifndef USER_MODE
      time_out = openair_get_mbox();
      if ((mac_xface->frame % 100) == 0)
	msg("[PHY][CODING][CHBCH %d] Frame %d: chbch channel compensation : time_in %d,time_out %d\n",chbch_ind[c],mac_xface->frame,time_in,time_out);
      time_in = openair_get_mbox();
#endif //USER_MODE
#endif


      // 5. Clipping 
      for (c=0;c<2;c++) {

	Isymb = (__m128i *)PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f4;
	iii=0;
	for (n=0;
	     n<NUMBER_OF_CHBCH_SYMBOLS;
	     n++) {
	  
	  Csymb = (__m128i *)&PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f2[1][(n)<<(LOG2_NUMBER_OF_OFDM_CARRIERS)];
	  
	  ii = FIRST_CARRIER_OFFSET>>2;   // Point to first useful carrier 
	  
	  
	  // NUMBER_OF_USEFUL_CARRIERS MUST BE A MULTIPLE OF 8 TO WORK!!! ELSE FIX WRITES AT END OF LOOP
	  for (i = 0 ; i<NUMBER_OF_USEFUL_CARRIERS; i+=8) {
	    
	    xmm0 = Csymb[ii];
	    xmm2 = Csymb[ii+1];
	    
#ifdef DEBUG_PHY
#ifdef USER_MODE
	    temp = xmm0;
	    //	    print_shorts(temp,"xmm0=");
	    temp = xmm2;
	    //	    print_shorts(temp,"xmm2=");
#endif //USER_MODE
#endif //DEBUG_PHY

	    
	    Isymb[iii++] = _mm_packs_epi16(xmm0,xmm2);      // pack to 8 bits with saturation
	    
	    temp = Isymb[iii-1];

	    //	    print_bytes(temp,"xmm0(packs)=");
	    
	    ii+=2;
	    if (ii==(NUMBER_OF_OFDM_CARRIERS>>2))
	      ii=0;
	    
	    
	  } // useful carriers (i)
	  
	} //symbols (n) 

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    
	sprintf(fname,"chbch%d_clipped_output.m",chbch_ind[c]);
	sprintf(vname,"chbch%d_clipped_out",chbch_ind[c]);
	
	write_output(fname,vname,
		     &PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f4[0],
		     NUMBER_OF_CHBCH_SYMBOLS*NUMBER_OF_USEFUL_CARRIERS,1,5);
#endif // DEBUG_PHY
#endif // USER_MODE  
	
	// 6. frequency deinterleaving and demodulation
	
	demod_data = PHY_vars->chbch_data[chbch_ind[c]].demod_data;
	jj=0;
	
	
	for (n=0;
	     n<NUMBER_OF_CHBCH_SYMBOLS;
	     n++) {
	  
	  // Frequency Deinterleaving  
	  Isymb2 = (char*)&PHY_vars->chbch_data[chbch_ind[c]].rx_sig_f4[(n*NUMBER_OF_USEFUL_CARRIERS)];
	  pilot_ind = 0;
	  
	  for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j+=nb_antennas_tx) {
	    ii = j;
	    for (i = 0 ; i<NUMBER_OF_FREQUENCY_GROUPS ; i++) {
	      
	      // This is the offset for the jth carrier in the ith group
	      //	ii = (FIRST_CARRIER_OFFSET + (i*NUMBER_OF_CARRIERS_PER_GROUP) + j)% NUMBER_OF_OFDM_CARRIERS;
	      //	printf("jj:%d(%d),jj+size:%d(%d),ii:%d\n",jj,Isymb2[(ii<<1)],jj+chbch_size_bits,Isymb2[1+(ii<<1)],ii);
	      for (aa=0;aa<nb_antennas_tx;aa++) {
		if (pilot_ind == NUMBER_OF_CHBCH_PILOTS*CHBCH_FREQUENCY_REUSE_FACTOR) {
		  demod_data[jj]                 = Isymb2[((aa+ii)<<1)]>>4;    // Real component
		  demod_data[jj+chbch_size_bits] = Isymb2[((aa+ii)<<1)+1]>>4;  // Imaginary components
		  jj++;
		}
		else
		  pilot_ind++;
	      }
	      
	      ii+=NUMBER_OF_CARRIERS_PER_GROUP;
	    } // groups
	  } // carriers per group
	}
	
#ifdef DEBUG_PHY
#ifndef USER_MODE
	time_out = openair_get_mbox();
	if ((mac_xface->frame % 100) == 0)
	  msg("[PHY][CODING][CHBCH %d] Frame %d: chbch deinterleaving : time_in %d,time_out %d\n",chbch_ind[c],mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif 
	
#ifdef USER_MODE  
#ifdef DEBUG_PHY  
	sprintf(fname,"chbch%d_decode_input.m",chbch_ind[c]);
	sprintf(vname,"chbch%d_decode_in",chbch_ind[c]);
	
	write_output(fname,vname,
		     &PHY_vars->chbch_data[chbch_ind[c]].demod_data[0],
		     2*NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_CHBCH_SYMBOLS,1,4);
#endif // DEBUG_PHY
#endif // USER_MODE
	
	
	// 7. Viterbi Decoding
	
	//  msg("[openair][PHY][CHBCH %d] frame %d: Decoding -> Viterbi, num_bytes %d,%p,%p\n",chbch_ind,frame,mac_xface->mac_tch->bch_rx[chbch_ind].size,mac_xface->mac_tch->bch_rx[chbch_ind].data,PHY_vars->chbch_data[chbch_ind].demod_data);
	
	chbch_pdu  = PHY_vars->chbch_data[chbch_ind[c]].demod_pdu;
	chbch_size_bytes = chbch_size_bits>>3;
	
	
	
	Zero_Buffer(chbch_pdu,
		    chbch_size_bytes+8);
	
	
	
#ifndef USER_MODE
	time_in = openair_get_mbox();
#endif //USER_MODE
	
	//  printf("chbch_size_bits = %d\n",chbch_size_bits);
	
	phy_viterbi_dot11_sse2(PHY_vars->chbch_data[chbch_ind[c]].demod_data,
			       chbch_pdu,
			       chbch_size_bits);
	
	
	
	
	
	
	// descramble data
	for (i=0;
	     i<chbch_size_bytes-4;
	     i++) {
	  chbch_pdu[i] = chbch_pdu[i] ^ scrambling_sequence[i+chbch_ind[c]];
	  
	  //	  printf("%d:%d\n",i,chbch_pdu[i]);
	  // store the needed part for the MAC
	  if (i<chbch_pdu_length_bytes)
	    chbch_mac_pdu[chbch_ind[c] - 1][i] = chbch_pdu[i];
	}

	oldcrc= *((unsigned int *)(&chbch_pdu[chbch_size_bytes-4]));
	oldcrc&=0x00ffffff;
	
	crc = crc24(chbch_pdu,
		    (chbch_size_bytes-4)<<3)>>8;

#ifdef DEBUG_PHY    
	if ((mac_xface->frame % 100) == 0) {
	  msg("[PHY][CODING][CHBCH %d] Received CRC : %x\n",chbch_ind[c],oldcrc);
	  msg("[PHY][CODING][CHBCH %d] Computed CRC : %x\n",chbch_ind[c],crc);
	}
#endif
	
#ifdef USER_MODE
#ifdef DEBUG_PHY    
	write_output("scrambler.m","scrambling_seq",
		     (void*)scrambling_sequence,
		     1024,
		     1,5);
	
#endif //DEBUG_PHY
#endif //USER_MODE

#ifdef DEBUG_PHY
#ifndef USER_MODE
	time_out = openair_get_mbox();
	if ((mac_xface->frame % 100) == 0)
	  msg("[PHY][CODING][CHBCH %d] Frame %d: chbch channel decoding : time_in %d,time_out %d\n",chbch_ind[c],mac_xface->frame,time_in,time_out);
#endif //USER_MODE
#endif
	
	ret[c] = (crc==oldcrc) ? 0 : -1;
	
      }

      _mm_empty();
}

int phy_chbch_phase_comp(struct complex16 *Rchsch, 
			 struct complex16 *Rsymb, 
			 int chbch_ind, 
			 int nb_antennas_tx, 
			 struct complex16 *perror_out, 
			 unsigned char do_rotate) {
  __m64 *Rchsch64;
  register __m64 mm0,mm1;
  struct complex16 perror;
  int i2,ind,ind64,aatx,norm=0;
  __m64 temp64;
  short *Rsymb64_ptr=NULL,*perror64_ptr=NULL;

  if (NUMBER_OF_CHBCH_PILOTS) {
    perror.r = 0;
    perror.i = 0;

    Rchsch64 = (__m64 *)Rchsch;

    // inner product of received CHSCH in pilot positions and received symbol
    mm1 = _m_pxor(mm1,mm1);

    ind64 = FIRST_CARRIER_OFFSET+(chbch_ind-1)*NUMBER_OF_CHBCH_PILOTS/2*NUMBER_OF_CARRIERS_PER_GROUP;
    ind = ind64<<1;

    for (i2=0;i2<NUMBER_OF_CHBCH_PILOTS;i2+=nb_antennas_tx) {

      if (ind != 0) // skip DC carrier
	for (aatx=0;aatx<nb_antennas_tx;aatx++) {
	  Rsymb64_ptr = (short*)&Rsymb_conj64;
	  Rsymb64_ptr[0] = Rsymb[ind].r;
	  Rsymb64_ptr[1] = Rsymb[ind].i;
	  Rsymb64_ptr[2] = -Rsymb[ind].i;
	  Rsymb64_ptr[3] = Rsymb[ind].r;
	     
	  mm0 = _mm_madd_pi16(Rchsch64[ind64],Rsymb_conj64);
	  mm1 = _mm_add_pi32(mm0,mm1);
	    
#ifdef DEBUG_PHY
#ifdef USER_MODE
	  temp64 = mm0;
	  msg("[OPENAIR][PHY][CHBCH PHASE EST]Ant %d pilot %d (%d,%d): RX p (%d,%d), RX s (%d,%d)\n",
	      aatx,i2,ind64,ind,Rchsch[ind].r,Rchsch[ind].i,
	      Rsymb[ind].r,Rsymb[ind].i);
#endif //USER_MODE
#endif //DEBUG_PHY
	  ind+=2;
	  ind64++;
	}
      else
	ind64+=nb_antennas_tx;

	
      ind64+=(NUMBER_OF_CARRIERS_PER_GROUP-nb_antennas_tx);
      if (ind64>=NUMBER_OF_OFDM_CARRIERS)
	ind64-=NUMBER_OF_OFDM_CARRIERS;
      ind=ind64<<1;
    }


    //PHY_vars->chbch_data[chbch_ind].perror.r >>= LOG2_NUMBER_OF_CHBCH_PILOTS;
    //PHY_vars->chbch_data[chbch_ind].perror.i >>= LOG2_NUMBER_OF_CHBCH_PILOTS;

    perror64 = _mm_srai_pi32(mm1,PERROR_SHIFT+LOG2_NUMBER_OF_CHBCH_PILOTS);
    perror64_ptr = (short*)&perror64;
    perror.r = perror64_ptr[0];  
    perror.i = perror64_ptr[2];

    norm = iSqrt((int)perror.r*perror.r + perror.i*perror.i);
    //printf("norm %d (%d,%d)\n",norm,perror.r,perror.i);
    // bring perror to unit circle with 8 bits of precision
    if (norm>0) {
      //perror.r <<= 5;
      //perror.i <<= 5;
      //perror.r /= norm;
      //perror.i /= norm;
      perror.r = (short)((((int) perror.r) << 5)/norm);
      perror.i = (short)((((int) perror.i) << 5)/norm);
    }

    //printf("perror.r = %d, perror.i=%d\n",perror.r,perror.i);
    *perror_out = perror;

    // Apply rotation

    if (do_rotate == 1) 
      rotate_cpx_vector((short *)Rsymb,
			(short *)&perror,
			(short *)Rsymb,
			NUMBER_OF_OFDM_CARRIERS,
			5,
			0);

  }
  return(norm);
}

#else //EXPRESSMIMO_TARGET

int phy_decode_chbch(unsigned char chbch_ind,
                     unsigned char nb_antennas_rx,
                     unsigned char nb_antennas_tx,
                     unsigned char *chbch_mac_pdu,
                     unsigned int chbch_pdu_length_bytes) { 
}

#endif //EXPRESSMIMO_TARGET
/** @} */

#ifndef USER_MODE
EXPORT_SYMBOL(phy_generate_chbch);
EXPORT_SYMBOL(phy_decode_chbch);
#endif // USER_MODE

