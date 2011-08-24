/*!\brief MRBCH Encoding and Decoding*/
///
/// Calls underlying channel coding and modulation for MRBCH
///

#ifndef USER_MODE
#define __NO_VERSION__



//#ifdef RTAI_ENABLED
//#include <rtai.h>
//#include <rtai_posix.h>
//#include <rtai_fifos.h>
//#endif
#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif //CBMIMO1

#else // USER_MODE
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#endif // USER_MODE

#include "PHY/defs.h"
#include "PHY/types.h"
#include "PHY/extern.h"
#include "PHY/TOOLS/defs.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "defs.h"
#include "extern.h"

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif

#ifndef EXPRESSMIMO_TARGET
#include "xmmintrin.h"
#endif //EXPRESSMIMO_TARGET

#ifndef USER_MODE 
#include <linux/crc32.h>
#else
#define crc32(X,Y,Z) 0
#endif //
//#define crc32(X,Y,Z) 0

// precompute interleaving for mrbch


#ifndef USER_MODE
#define openair_get_mbox() (*(unsigned int *)mbox)
#endif //USER_MODE
#ifdef USER_MODE
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif //USER_MODE

#ifdef OPENAIR2
void phy_generate_mrbch_top(unsigned char sch_index) {

  unsigned char i; //,j;

  
  for(i=0;i<NB_REQ_MAX;i++) {
    /* 
   if (((mac_xface->frame/5) % 20) == 0)
      msg("[PHY][CODING] Frame %d: Ind %d(%p), Active %d, Type %d\n",
	  mac_xface->frame,
	  i,
	  &Macphy_ind_table.Macphy_ind_table_entry[i].Macphy_data_ind,
	  Macphy_ind_table.Macphy_ind_table_entry[i].Active,
	  (Macphy_ind_table.Macphy_ind_table_entry[i].Active == 1)?
	  Macphy_ind_table.Macphy_ind_table_entry[i].Macphy_data_ind.Pdu_type:-1);
    */

    if(Macphy_req_table[0].Macphy_req_table_entry[i].Active){
      if (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == MRBCH) {

	
	Macphy_req_table[0].Macphy_req_table_entry[i].Active=0;
	//	Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_req.Phy_Resources_Entry->Active=0;
	Macphy_req_table[0].Macphy_req_cnt--;
	/*
	if ((mac_xface->frame % 100) == 0) {
	  for (j=0;j<136;j+=4)
	    msg("[PHY][CODING] MRBCH_PDU(%d) %x : %x %x %x %x\n",sizeof(MRBCH_PDU),j,
		((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Mrbch_pdu)[j],
		((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Mrbch_pdu)[j+1],
		((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Mrbch_pdu)[j+2],
		((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Mrbch_pdu)[j+3]);
	}
	*/

	//	if (((mac_xface->frame/5) % 20) == 0)
	//	  msg("[PHY][CODING] Frame %d: Calling generate mrbch\n",mac_xface->frame);

	phy_generate_mrbch(sch_index,
			   0,
			   NB_ANTENNAS_TX,
			   (unsigned char *)Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Mrbch_pdu);
      }
    }
  }

}

#endif // OPENAIR2

unsigned char phy_generate_mrbch(unsigned char sch_index,
				 unsigned char extension_switch,
				 unsigned char nb_antennas_tx,
				 unsigned char *mrbch_pdu) {

  unsigned int crc;
  int i,j,n,ii,jj,jjj,off,aa;
  short pilot_ind;

//  int ant_index;
  unsigned char *bch_data = PHY_vars->mrbch_data[0].encoded_data[0];
  short gain_lin;
  int *fft_input[nb_antennas_tx];
  short *fft_input16[nb_antennas_tx];

#ifdef USER_MODE
#ifdef DEBUG_PHY
  short data;
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif // USER_MODE

  
  unsigned int mrbch_size_bits,mrbch_size_bytes;
  unsigned char mrbch_power;
  unsigned int tx_energy;

#ifdef DEBUG_PHY
  msg("[openair][PHY][CODING] Frame %d: Generate MRBCH (%d,%d,%d) for %d antennas\n",
      mac_xface->frame,
      NUMBER_OF_MRBCH_SYMBOLS,
      SYMBOL_OFFSET_MRBCH,
      SAMPLE_OFFSET_MRBCH,
      nb_antennas_tx);
#endif // DEBUG_PHY
  // Encode data

  mrbch_size_bits = (NUMBER_OF_USEFUL_CARRIERS-NUMBER_OF_MRBCH_PILOTS)*NUMBER_OF_MRBCH_SYMBOLS;
  mrbch_size_bytes = mrbch_size_bits>>3;

  //msg("[openair][PHY][CODING] Frame %d: MRBCH size (bytes) = %d \n",mac_xface->frame, mrbch_size_bytes);

  // scramble data
  for (i=0;
       i<mrbch_size_bytes;
       i++)
    PHY_vars->mrbch_data[0].tx_pdu[0][i] = mrbch_pdu[i]^scrambling_sequence[i];
  

  crc = crc24(PHY_vars->mrbch_data[0].tx_pdu[0],
 	      (mrbch_size_bytes-4)<<3);

  
  // Get crc
  *(unsigned int *)&PHY_vars->mrbch_data[0].tx_pdu[0][mrbch_size_bytes-4] = crc>>8;



  ccodedot11_encode(mrbch_size_bytes,
		    PHY_vars->mrbch_data[0].tx_pdu[0],
		    PHY_vars->mrbch_data[0].encoded_data[0],
		    0);

#ifdef DEBUG_PHY
#ifdef USER_MODE
  write_output("mrbch_encoded_output.m","mrbch_encoded_out",
	       PHY_vars->mrbch_data[0].encoded_data[0],
	       2*NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_MRBCH_SYMBOLS,
	       1,
	       4);

#endif //USER_MODE
#endif //DEBUG_PHY
  // Set up FFT Input buffer
  for (i=0;i<nb_antennas_tx;i++) {
    fft_input[i]   = PHY_vars->mrbch_data[0].fft_input[i];
    fft_input16[i] = (short *)fft_input[i];
    // clear FFT input buffer (for zero carriers)
    Zero_Buffer((void *)fft_input[i],
		NUMBER_OF_OFDM_CARRIERS_BYTES * NUMBER_OF_MRBCH_SYMBOLS);

  }

  //  printf("NUMBER_OF_OFDM_CARRIERS_BYTES = %d (%p,%p)\n",NUMBER_OF_OFDM_CARRIERS_BYTES,fft_input,beacon_pilot);




  gain_lin = (MRSCH_AMP * ONE_OVER_SQRT2_Q15*nb_antennas_tx)>>15;

  
  // QPSK MODULATION
  jj=0;
  jjj=0;
  for (n=0;
       n<NUMBER_OF_MRBCH_SYMBOLS;
       n++) {

    pilot_ind = 0;
    off = n<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS);

    for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j+=nb_antennas_tx) {
      ii = FIRST_CARRIER_OFFSET+j;
      //      jjj = 0;//j % (nb_antennas_tx);// Comment/Uncomment for space-diversity
      for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
	  
	//	  ii = (FIRST_CARRIER_OFFSET + (i*NUMBER_OF_CARRIERS_PER_GROUP) + j)% NUMBER_OF_OFDM_CARRIERS;
	//	printf("jj:%d,jjj:%d,ii:%d\n",jj,jjj,ii);
	
	for (aa=0;aa<nb_antennas_tx;aa++) {
	  if (pilot_ind == NUMBER_OF_MRBCH_PILOTS) {
	    fft_input16[aa][off+((aa+ii)<<1)]   = (bch_data[jj]==0)                 ? (-gain_lin) : gain_lin;
	    fft_input16[aa][1+off+((aa+ii)<<1)] = (bch_data[jj+mrbch_size_bits]==0) ? (-gain_lin) : gain_lin;
	    jj++;
	  }
	  else {  // This is for pilots
	      fft_input[aa][(off>>1)+ii+aa] = 
		PHY_vars->sch_data[sch_index].SCH_f_tx[aa][ii+aa];
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


  tx_energy = 0;
  for (i=0;i<nb_antennas_tx;i++) {

    PHY_ofdm_mod(fft_input[i],
		 (int *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[((extension_switch == 1) ? SAMPLE_OFFSET_MRBCH : SAMPLE_OFFSET_MRBCH_NO_PREFIX)],   
		 LOG2_NUMBER_OF_OFDM_CARRIERS,
		 NUMBER_OF_MRBCH_SYMBOLS,
		 CYCLIC_PREFIX_LENGTH,
		 twiddle_ifft,
		 rev,
		 (extension_switch == 1) ? EXTENSION_TYPE : NONE
		 );
    tx_energy += signal_energy(&PHY_vars->tx_vars[i].TX_DMA_BUFFER[((extension_switch == 1) ? SAMPLE_OFFSET_MRBCH : SAMPLE_OFFSET_MRBCH_NO_PREFIX)],(NUMBER_OF_MRBCH_SYMBOLS)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
  }

  mrbch_power = dB_fixed(tx_energy);

#ifdef BIT8_TXMUX
  bit8_txmux(((extension_switch == 1) ? OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES : OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)*(NUMBER_OF_MRBCH_SYMBOLS),
	     (extension_switch == 1) ? SAMPLE_OFFSET_MRBCH : SAMPLE_OFFSET_MRBCH_NO_PREFIX );
#endif //BIT8_TXMUX





#ifdef DEBUG_PHY
#ifdef USER_MODE  
  for (i=0;i<nb_antennas_tx;i++) {
    sprintf(fname,"mrbch%d_data.m",i);
    sprintf(vname,"mrbch%d_dat",i);
    write_output(fname,vname,
		 (short *)fft_input[i],
		 NUMBER_OF_MRBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,
		 1,
		 1);
    sprintf(fname,"mrbch%d_sig.m",i);
    sprintf(vname,"mrbch%d",i);    
    write_output(fname,vname,
		 (short *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[(extension_switch == 1) ? SAMPLE_OFFSET_MRBCH :SAMPLE_OFFSET_MRBCH_NO_PREFIX],
		 ((extension_switch == 1) ? OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES: OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)*(NUMBER_OF_MRBCH_SYMBOLS),
		 1,
		 1);
  }
#endif //
#endif // DEBUG_PHY			 



#ifdef DEBUG_PHY
  if ((mac_xface->frame % 100) == 0)
    msg("[OPENAIR][PHY][MRBCH] Power %d, crc %x\n",mrbch_power,crc);
#endif DEBUG_PHY
#ifdef BIT8_TXMUX
  mrbch_power-=BIT8_TX_SHIFT_DB;
#endif

  return(mrbch_power);
}




#ifdef OPENAIR2
void phy_decode_mrbch_top(unsigned char sch_index) {

  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry;
  MACPHY_REQ_ENTRY_KEY Search_key;
  int crc_status;

  Search_key.Key_type=PDU_TYPE_KEY;
  Search_key.Key.Pdu_type=MRBCH;
  //  msg("[PHY][CODING][MRBCH] Searching for MRBCH request\n");
  Macphy_data_req_entry = find_data_req_entry(0,&Search_key);

  if (Macphy_data_req_entry) {// IF not there is a pb
    phy_channel_estimation_top(PHY_vars->rx_vars[0].offset,
			       SYMBOL_OFFSET_MRSCH,
			       0,
			       sch_index,
			       NB_ANTENNAS_RX,
			       SCH);

    crc_status = phy_decode_mrbch(sch_index,
#ifdef BIT8_RXDMUX
				  1,
#endif 
				  NB_ANTENNAS_RX,
				  NB_ANTENNAS_TXRX,
				  (unsigned char*)Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Mrbch_pdu,
				  sizeof(MRBCH_PDU));

#ifndef USER_MODE    
    if (crc_status == -1) {
      PHY_vars->mrbch_data[0].pdu_errors++;
      PHY_vars->mrbch_data[0].pdu_errors_conseq++;
    }
    else
      PHY_vars->mrbch_data[0].pdu_errors_conseq=0;

    if (PHY_vars->mrbch_data[0].pdu_errors_conseq >= 1000) {
      msg("[PHY][MRBCH] Frame %d : consecutive error count reached, resynchronizing\n",mac_xface->frame);
#ifndef USER_MODE
      openair_daq_vars.mode=openair_NOT_SYNCHED;
      openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
      openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1

      openair_daq_vars.synch_wait_cnt=0;
      openair_daq_vars.sched_cnt=-1;
#endif //USER_MODE
      mac_xface->frame = -1;
    }

    if (mac_xface->frame % 100 == 0)
      msg("[PHY][MRBCH] Frame %d : MRBCH error count = %d\n",
	  mac_xface->frame,
	  PHY_vars->mrbch_data[0].pdu_errors);

#endif

    //     = PHY_vars->mrbch_data[mrbch_ind].demod_pdu;
    Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0] = crc_status;
    //    msg("[PHY][CODING][MRBCH] MACPHY_req_table_entry.MRBCH_PDU=%p,CRC status %d\n",
    //          Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Mrbch_pdu,
    //          Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
    mac_xface->macphy_data_ind(0,
			       &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,
			       MRBCH,
			       0);
    Macphy_req_table[0].Macphy_req_cnt--;
    Macphy_data_req_entry->Active=0;
  }
}

#endif //OPENAIR2


#ifndef EXPRESSMIMO_TARGET

static __m64 perror64 __attribute__ ((aligned(16)));
static __m64 Rsymb_conj64 __attribute__ ((aligned(16)));
 
int phy_decode_mrbch(unsigned char sch_index,
		     unsigned char nb_antennas_rx,
		     unsigned char nb_antennas_tx,
		     unsigned char *mrbch_mac_pdu,
		     unsigned int mrbch_pdu_length_bytes) { 
  

  unsigned short i,j=0,i2,n,ii,aa,iii,jj;
  
  int *input;
  char *demod_data;
  
  unsigned int oldcrc,crc,rxp[nb_antennas_rx];
  
  
  unsigned int avg,maxh,minh;
  
  unsigned char log2_avg,log2_maxh,log2_perror_amp=0;
  
  short stat;
  
  struct complex16 *Rmrsch,*Rsymb,perror[4][8];
  __m64 *Rmrsch64;
  register __m64 mm0,mm1;
  
  int ind,ind64;
  int chr,chi,norm;
  int rx_energy[nb_antennas_rx];
  int n0_energy[nb_antennas_rx];
  unsigned char *mrbch_pdu;
  unsigned int mrbch_size_bits=((NUMBER_OF_USEFUL_CARRIERS-NUMBER_OF_MRBCH_PILOTS)*NUMBER_OF_MRBCH_SYMBOLS);
  unsigned int mrbch_size_bytes;
  unsigned int time_in,time_out;

  unsigned int off,amp_shift,pilot_ind,aatx;
  char *Isymb2;

  __m128i *Csymb[nb_antennas_rx],*Isymb,temp;
  register __m128i xmm0,xmm1,xmm2,xmm3,xmm4;

#ifdef USER_MODE
  char fname[40],vname[40];
#endif //USER_MODE
  
#ifdef DEBUG_PHY
  msg("[openair][PHY][CODING] Decode MRBCH (%d), rx_offset %d\n",
      NUMBER_OF_MRBCH_SYMBOLS,PHY_vars->rx_vars[0].offset);
  
  
#endif // DEBUG_PHY 



  for (i=0;i<nb_antennas_rx;i++) {

#ifndef USER_MODE
    if (openair_daq_vars.mode == openair_NOT_SYNCHED) {

      rx_energy[i] = signal_energy((int *)&PHY_vars->rx_vars[i].RX_DMA_BUFFER[SAMPLE_OFFSET_MRBCH+PHY_vars->rx_vars[0].offset+CYCLIC_PREFIX_LENGTH],
				   OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);

      n0_energy[i] = signal_energy((int *)&PHY_vars->rx_vars[i].RX_DMA_BUFFER[PHY_vars->rx_vars[0].offset+CYCLIC_PREFIX_LENGTH+(TX_RX_SWITCH_SYMBOL+1) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
				   OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);

    }
    else {  
      
      
      rx_energy[i] = signal_energy((int *)&PHY_vars->rx_vars[i].RX_DMA_BUFFER[SAMPLE_OFFSET_MRBCH_NO_PREFIX],
				   OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX);
      n0_energy[i] = signal_energy((int *)&PHY_vars->rx_vars[i].RX_DMA_BUFFER[(TX_RX_SWITCH_SYMBOL+1) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX], OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX);
      
    }
#else //USER_MODE
    rx_energy[i] = signal_energy((int *)&PHY_vars->rx_vars[i].RX_DMA_BUFFER[PHY_vars->rx_vars[0].offset+SAMPLE_OFFSET_MRBCH+CYCLIC_PREFIX_LENGTH],
				 OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
		
    n0_energy[i] = signal_energy((int *)&PHY_vars->rx_vars[i].RX_DMA_BUFFER[PHY_vars->rx_vars[0].offset+CYCLIC_PREFIX_LENGTH + (TX_RX_SWITCH_SYMBOL+1) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
				 OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    
#endif //USER_MODE

    rxp[i] = (unsigned int)dB_fixed(rx_energy[i]);
    
    PHY_vars->PHY_measurements.rx_power_dB[0][i] = (short) dB_fixed(rx_energy[i]); 

    PHY_vars->PHY_measurements.rx_rssi_dBm[0][i] =     
      PHY_vars->PHY_measurements.rx_power_dB[0][i] -
      PHY_vars->rx_vars[0].rx_total_gain_dB;
    PHY_vars->PHY_measurements.n0_power_dB[0][i] = (short) dB_fixed(n0_energy[i]);

    PHY_vars->PHY_measurements.n0_power[0][i] = n0_energy[i];
    PHY_vars->PHY_measurements.rx_power[0][i] = rx_energy[i];
  }


#ifdef BIT8_RXDEMUX   // Deinterleave and IQflip the two 8-bit antenna streams CBMIMO1 software modem
    
  if (rxdemux_done == 0)
    bit8_rxdemux(OFDM_SYMBOL_SIZE_SAMPLES*(NUMBER_OF_MRBCH_SYMBOLS),SAMPLE_OFFSET_MRBCH);
  
#endif //


#ifndef USER_MODE
#ifdef DEBUG_PHY
  if (mac_xface->frame % 100 == 0 || openair_daq_vars.mode == openair_NOT_SYNCHED) {
	

	  msg("[openair][PHY][MRBCH] frame %d: MODE %d, slot_count %d : Offset = %d\n",

	      mac_xface->frame,
	      openair_daq_vars.mode,
	      openair_daq_vars.slot_count,
	      PHY_vars->rx_vars[0].offset);


	  for (i=0;i<nb_antennas_rx;i++) 
	    {
	      msg("[openair][PHY][MRBCH] frame %d, antenna %d: MRBCH Signal Power = %d dBm (digital = %d dB, rx_gain = %d dB, energy = %d), N0 Power = %d (%d dB)\n",
		  mac_xface->frame,
		  i,
		  PHY_vars->PHY_measurements.rx_rssi_dBm[0][i],
		  rxp[i],
		  PHY_vars->rx_vars[0].rx_total_gain_dB,
		  rx_energy[i],
		  n0_energy[i],
		  PHY_vars->PHY_measurements.n0_power_dB[0][i]
		  );
	    }

  }
#endif //DEBUG_PHY
#endif //USER_MODE

  /* Channel estimation is done outside here
#ifndef USER_MODE
  time_in = openair_get_mbox();

  if (openair_daq_vars.mode == openair_NOT_SYNCHED)
    phy_channel_estimation_top(PHY_vars->rx_vars[0].offset,0,0,0,nb_antennas_rx,CHSCH);
  else {



    phy_channel_estimation_top(0,0,
#ifdef HW_PREFIX_REMOVAL
			       1
#else
			       0
#endif //HW_PREFIX_REMOVAL			  
			      ,0,
			       nb_antennas_rx,
			       CHSCH );

    time_out = openair_get_mbox();
    if ((mac_xface->frame % 100) == 0)
      msg("[PHY][CODING][MRBCH] Frame %d: mrbch channel estimation : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);

  }
#else //USER_MODE
  phy_channel_estimation_top(PHY_vars->rx_vars[0].offset,0,0,0,nb_antennas_rx,CHSCH);

#endif //USER_MODE

#ifndef USER_MODE
  time_in = openair_get_mbox();
#endif //USER_MODE
  */



  for (aa = 0 ; aa < nb_antennas_rx ; aa++) {


    for (i=0;
	 i<(NUMBER_OF_MRBCH_SYMBOLS);
	 i++ ){
      
#ifndef USER_MODE      
      if (openair_daq_vars.mode == openair_NOT_SYNCHED) { 
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(PHY_vars->rx_vars[0].offset +
						      ((i+SYMBOL_OFFSET_MRBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)+
						      (i+1+SYMBOL_OFFSET_MRBCH)*CYCLIC_PREFIX_LENGTH)%FRAME_LENGTH_SAMPLES];
      }
      else {
#ifdef HW_PREFIX_REMOVAL
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((i+SYMBOL_OFFSET_MRBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)];
#else
	input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((i+SYMBOL_OFFSET_MRBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS) +
						     (i+SYMBOL_OFFSET_MRBCH+1)*CYCLIC_PREFIX_LENGTH];
#endif //HW_PREFIX_REMOVAL
      }
#else //USER_MODE

      input = &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(PHY_vars->rx_vars[0].offset +
						    ((i+SYMBOL_OFFSET_MRBCH)<<LOG2_NUMBER_OF_OFDM_CARRIERS)+
						    (i+1+SYMBOL_OFFSET_MRBCH)*CYCLIC_PREFIX_LENGTH)%FRAME_LENGTH_SAMPLES];
      
#endif //USER_MODE

      //    msg("[openair][PHY][MRBCH %d] frame %d: Decoding -> FFT %d\n",mrbch_ind,frame,i);

      //      dump_mrbch_pilots(0);

      //      msg("mrbch: calling fft %d,%d\n",aa,i);
      //      msg("mrbch: calling fft input %x,%x,%x,%x,%d,%d \n",
      //	  &input[0],
      //	  (short *)&PHY_vars->mrbch_data[mrbch_ind].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
      //	  (short *)twiddle_fft,
      //	  rev,
      //	  LOG2_NUMBER_OF_OFDM_CARRIERS,
      //	  3);

      	
      fft((short *)&input[0],
	  (short *)&PHY_vars->mrbch_data[0].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
	  (short *)twiddle_fft,
	  rev,
	  LOG2_NUMBER_OF_OFDM_CARRIERS,
	  3,
	  0);
      

      
      // Phase error compensation


      if (NUMBER_OF_MRBCH_PILOTS) {


	//	msg("mrbch: doing phase compensation %d,%d\n",aa,i);
      
	
	perror[aa][i].r = 0;
	perror[aa][i].i = 0;

	Rmrsch = (struct complex16 *)&PHY_vars->sch_data[sch_index].rx_sig_f[aa][0];	
	Rsymb  = (struct complex16 *)&PHY_vars->mrbch_data[0].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];

	Rmrsch64 = (__m64 *)&PHY_vars->sch_data[sch_index].rx_sig_f[aa][0];	


	// inner product of received SCH in pilot positions and received symbol
	mm1 = _m_pxor(mm1,mm1);

	ind64 = FIRST_CARRIER_OFFSET;
	ind = ind64<<1;

	for (i2=0;i2<NUMBER_OF_MRBCH_PILOTS;i2+=nb_antennas_tx) {

	  for (aatx=0;aatx<nb_antennas_tx;aatx++) {
	    ((short *)&Rsymb_conj64)[0] = Rsymb[ind].r;
	    ((short *)&Rsymb_conj64)[1] = Rsymb[ind].i;
	    ((short *)&Rsymb_conj64)[2] = -Rsymb[ind].i;
	    ((short *)&Rsymb_conj64)[3] = Rsymb[ind].r;
	    
	    mm0 = _mm_madd_pi16(Rmrsch64[ind64],Rsymb_conj64);
	    mm1 = _mm_add_pi32(mm0,mm1);
	    
#ifdef DEBUG_PHY
#ifdef USER_MODE
	    msg("[OPENAIR][PHY][MRBCH DEMOD]Ant %d symbol %d (%p), pilot %d (%d): RX p (%d,%d), RX s (%d,%d)\n",
		aa,i,Rsymb,i2,ind64,Rmrsch[ind].r,Rmrsch[ind].i,
		Rsymb[ind].r,Rsymb[ind].i);
#endif //USER_MODE
#endif //DEBUG_PHY
	    //	  perror.r += ( ((Rchsch[ind].r*Rsymb[ind].r)>>PERROR_SHIFT) + ((Rchsch[ind].i*Rsymb[ind].i)>>PERROR_SHIFT) );
	    //	  perror.i += ( ((Rchsch[ind].i*Rsymb[ind].r)>>PERROR_SHIFT) - ((Rchsch[ind].r*Rsymb[ind].i)>>PERROR_SHIFT) );
	    // MMX version
	    ind+=2;
	    ind64++;
	  }
	  //	  msg("mm0 = %d,%d\n",((int *)&mm0)[0],((short *)&mm0)[1]);
	
	  ind64+=(NUMBER_OF_CARRIERS_PER_GROUP-nb_antennas_tx);
	  if (ind64>=NUMBER_OF_OFDM_CARRIERS)
	    ind64-=NUMBER_OF_OFDM_CARRIERS;
	  ind=ind64<<1;
	}


	//perror.r >>= LOG2_NUMBER_OF_MRBCH_PILOTS;
	//perror.i >>= LOG2_NUMBER_OF_MRBCH_PILOTS;

	perror64 = _mm_srai_pi32(mm1,PERROR_SHIFT+LOG2_NUMBER_OF_MRBCH_PILOTS);
	perror[aa][i].r = ((short *)&perror64)[0];  
	perror[aa][i].i = ((short *)&perror64)[2];  
	norm = iSqrt((int)perror[aa][i].r*perror[aa][i].r + (int)perror[aa][i].i*perror[aa][i].i);
	// bring perror to unit circle with 8 bits of precision
	if (norm>0) {
	  perror[aa][i].r <<= 5;
	  perror[aa][i].i <<= 5;
	  perror[aa][i].r /= norm;
	  perror[aa][i].i /= norm;
	}

#ifdef DEBUG_PHY
#ifdef USER_MODE
	
	msg("[OPENAIR][PHY][MRBCH DEMOD] Ant %d : symbol %d, norm = %d perror = (%d,%d)\n",aa,i,norm,perror[aa][i].r,perror[aa][i].i);
#endif //USER_MODE
#endif //DEBUG_PHY
	//	msg("perror64 = %d,%d\n",((short *)&perror64)[0],((short *)&perror64)[2]);
	

      }
    }






#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    msg("NUMBER_OF_MRBCH_SYMBOLS+NUMBER_OF_CHSCH_SYMBOLS = %d\n",NUMBER_OF_MRBCH_SYMBOLS+NUMBER_OF_CHSCH_SYMBOLS);
    msg("NUMBER_OF_OFDM_CARRIERS = %d\n",NUMBER_OF_OFDM_CARRIERS);
    sprintf(fname,"mrbch_rxsigF%d.m",aa);
    sprintf(vname,"mrbch_rxF%d",aa);

    write_output(fname,vname,
		 (short *)&PHY_vars->mrbch_data[0].rx_sig_f[aa][0],2*(NUMBER_OF_MRBCH_SYMBOLS)*NUMBER_OF_OFDM_CARRIERS,2,1);
    write_output("chsch_conj_F.m","chsch_conjF",
		 (short *)&PHY_vars->sch_data[sch_index].SCH_conj_f[0],2*NUMBER_OF_SCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);
    
    sprintf(fname,"mrbch_channelF%d.m",aa);
    sprintf(vname,"mrbch_chanF%d",aa);
    
    
    write_output(fname,vname,
		 (short *)&PHY_vars->sch_data[sch_index].channel_f[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
    
    sprintf(fname,"mrbch_channel%d.m",aa);
    sprintf(vname,"mrbch_chan%d",aa);
    
    write_output(fname,vname,
		 (short *)&PHY_vars->sch_data[sch_index].channel[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
    
    sprintf(fname,"mrbch_channelF%d.m",aa);
    sprintf(vname,"mrbch_chanF%d",aa);
    
    
    write_output(fname,vname,
		 (short *)&PHY_vars->sch_data[sch_index].channel_f[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);
    
    sprintf(fname,"mrbch_channel%d.m",aa);
    sprintf(vname,"mrbch_chan%d",aa);
    
    write_output(fname,vname,
		 (short *)&PHY_vars->sch_data[sch_index].channel[aa][0],2*NUMBER_OF_OFDM_CARRIERS,2,1);

#endif // DEBUG_PHY
#endif // USER_MODE

  }

  // check if phase estimate is to weak
#ifndef USER_MODE
  if (norm < 2) {
    return(-1);
  }
#endif //USER_MODE

  // Apply rotation

  //  msg("mrbch: applying rotation\n");

  if (NUMBER_OF_MRBCH_PILOTS) {
    for (aa=0;aa<nb_antennas_rx;aa++) {
      for (i=0;
	   i<(NUMBER_OF_MRBCH_SYMBOLS);
	   i++ ){
	
	
	Rsymb  = (struct complex16 *)&PHY_vars->mrbch_data[0].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
	
	//            msg("mrbch: calling rotate_cpx_vector: %x,%x,%x,%d,%d\n",
	//		(short *)Rsymb,(short *)&perror[aa][i],(short *)Rsymb,NUMBER_OF_OFDM_CARRIERS,log2_perror_amp);
	
	rotate_cpx_vector((short *)Rsymb,(short *)&perror[aa][i],(short *)Rsymb,NUMBER_OF_OFDM_CARRIERS,5,0);
	
      }
    }
  }




#ifndef USER_MODE  
  time_out = openair_get_mbox();
  /*
  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CODING][MRBCH] Frame %d: mrbch transform+CPOC : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);
  */
#endif //USER_MODE




  // Channel Compensation
  

  //  msg("mrbch: computing average channel strength\n");
  //  msg("[openair][PHY][MRBCH %d] frame %d: Decoding -> Channel Estimation 4\n",mrbch_ind,frame);
  avg = 0;
  maxh = 0;

#ifndef USER_MODE
  time_in = openair_get_mbox();
#endif //USER_MODE

  for (aa=0;aa<nb_antennas_rx;aa++) {
    for (i=0;i<NUMBER_OF_USEFUL_CARRIERS;i++) {
      ii = (FIRST_CARRIER_OFFSET + i)% NUMBER_OF_OFDM_CARRIERS;
      chr = (unsigned int)((short*)PHY_vars->sch_data[sch_index].channel_f[aa])[0+(ii<<2)];  // real-part
      chi = (unsigned int)((short*)PHY_vars->sch_data[sch_index].channel_f[aa])[1+(ii<<2)];  // -imag-part
      avg += chr*chr + chi*chi;
      //      maxh = cmax(maxh,chr*chr + chi*chi); 
      //      minh = cmin(minh,chr*chr + chi*chi); 
      
      /*
      ((short*)PHY_vars->chsch_data[mrbch_ind].channel_f[aa])[2+(ii<<2)] = - ((short*)PHY_vars->chsch_data[mrbch_ind].channel_f[aa])[1+(ii<<2)];
      ((short*)PHY_vars->chsch_data[mrbch_ind].channel_f[aa])[3+(ii<<2)] = ((short*)PHY_vars->chsch_data[mrbch_ind].channel_f[aa])[0+(ii<<2)];   // conjugate channel response

      
      if (i<16)
	msg("%d %d %d %d\n",
	    ((short*)PHY_vars->chsch_data[mrbch_ind].channel_f[aa])[0+(ii<<2)],
	    ((short*)PHY_vars->chsch_data[mrbch_ind].channel_f[aa])[1+(ii<<2)],
	    ((short*)PHY_vars->chsch_data[mrbch_ind].channel_f[aa])[2+(ii<<2)],
	    ((short*)PHY_vars->chsch_data[mrbch_ind].channel_f[aa])[3+(ii<<2)]);
      */

    }
  }

  //  msg("mrbch:avg=%d\n",avg);
  
  
  // find maximum bit position of I/Q components for rescaling
  avg/=(nb_antennas_rx * NUMBER_OF_USEFUL_CARRIERS);


  log2_maxh = log2_approx(avg) - 9;
  
  
  //  msg("mrbch:avg=%d,log2maxh-8=%d\n",avg,log2_maxh);
#ifndef USER_MODE
  if (log2_maxh < 1) {
    return(-1);
  }
#endif //USER_MODE

  //  return(-1);

  //  msg("mrbch: applying channel comp\n");

  for (aa=0;aa<nb_antennas_rx;aa++){
    for (i=0;
	 i<NUMBER_OF_MRBCH_SYMBOLS;
	 i++ ){
      
#ifdef USER_MODE
#ifdef DEBUG_PHY
      msg("[openair][PHY][MRBCH] Ant %d Compensating symbol %d\n",aa,i);
#endif //DEBUG_PHY
#endif //USER_MODE

      
      mult_cpx_vector((short *)&PHY_vars->mrbch_data[0].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
		      (short *)&PHY_vars->sch_data[sch_index].channel_matched_filter_f[aa][0],
		      (short *)&PHY_vars->mrbch_data[0].rx_sig_f2[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
		      NUMBER_OF_OFDM_CARRIERS,
		      log2_maxh);   // This is the approximate square-root of the average energy
     

    }
    
    //    msg("[openair][PHY][MRBCH %d] frame %d: Decoding -> Deinterleaving\n",mrbch_ind,frame);


#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    
    sprintf(fname,"mrbch_demod_output%d.m",aa);
    sprintf(vname,"mrbch_demod_out%d",aa);
    
    write_output(fname,vname,
		 &PHY_vars->mrbch_data[0].rx_sig_f2[aa][0],2*NUMBER_OF_MRBCH_SYMBOLS*NUMBER_OF_OFDM_CARRIERS,2,1);
#endif // DEBUG_PHY
#endif // USER_MODE  
  }


    




#ifndef USER_MODE
  time_out = openair_get_mbox();

  /*
  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CODING][MRBCH] Frame %d: mrbch channel compensation : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);

  */

  
  time_in = openair_get_mbox();
#endif //USER_MODE

  // Deinterleaving + Rescaling


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

  Isymb = (__m128i *)PHY_vars->mrbch_data[0].rx_sig_f4;
  iii=0;
  for (n=0;
       n<NUMBER_OF_MRBCH_SYMBOLS;
       n++) {

    for (aa=0;aa<nb_antennas_rx;aa++) {
      Csymb[aa] = (__m128i *)&PHY_vars->mrbch_data[0].rx_sig_f2[aa][(n)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
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
      //      print_shorts(((__m128i *)&PHY_vars->mrbch_data[mrbch_ind].rx_sig_f2[0][4*ii+(n<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS))])[0],"Csymb[0][ii]");

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

  demod_data = PHY_vars->mrbch_data[0].demod_data;
  jj=0;


  for (n=0;
       n<NUMBER_OF_MRBCH_SYMBOLS;
       n++) {

  // Frequency Deinterleaving  
    Isymb2 = (char*)&PHY_vars->mrbch_data[0].rx_sig_f4[(n*NUMBER_OF_USEFUL_CARRIERS)];
    pilot_ind = 0;

    for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j+=nb_antennas_tx) {
      ii = j;
      for (i = 0 ; i<NUMBER_OF_FREQUENCY_GROUPS ; i++) {
	
	// This is the offset for the jth carrier in the ith group
	//	ii = (FIRST_CARRIER_OFFSET + (i*NUMBER_OF_CARRIERS_PER_GROUP) + j)% NUMBER_OF_OFDM_CARRIERS;
	//	printf("jj:%d(%d),jj+size:%d(%d),ii:%d\n",jj,Isymb2[(ii<<1)],jj+mrbch_size_bits,Isymb2[1+(ii<<1)],ii);
	for (aa=0;aa<nb_antennas_tx;aa++) {
	  if (pilot_ind == NUMBER_OF_MRBCH_PILOTS) {
	    demod_data[jj]                 = Isymb2[((aa+ii)<<1)]>>4;    // Real component
	    demod_data[jj+mrbch_size_bits] = Isymb2[((aa+ii)<<1)+1]>>4;  // Imaginary components
	    jj++;
	  }
	  else
	    pilot_ind++;
	}
	
	ii+=NUMBER_OF_CARRIERS_PER_GROUP;
      } // groups
    } // carriers per group
  }
  
#ifndef USER_MODE
  time_out = openair_get_mbox();
  /*
  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CODING][MRBCH] Frame %d: mrbch deinterleaving : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);
  */

#endif //USER_MODE

#ifdef USER_MODE  
#ifdef DEBUG_PHY  
  write_output("mrbch_decode_input.m","decode_in",
	       &PHY_vars->mrbch_data[0].demod_data[0],
	       2*NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_MRBCH_SYMBOLS,1,4);
#endif // DEBUG_PHY
#endif // USER_MODE  

  // Viterbi Decoding


  //  msg("[openair][PHY][MRBCH %d] frame %d: Decoding -> Viterbi, num_bytes %d,%p,%p\n",mrbch_ind,frame,mac_xface->mac_tch->bch_rx[mrbch_ind].size,mac_xface->mac_tch->bch_rx[mrbch_ind].data,PHY_vars->mrbch_data[mrbch_ind].demod_data);

  mrbch_pdu  = PHY_vars->mrbch_data[0].demod_pdu;
  mrbch_size_bytes = mrbch_size_bits>>3;



  Zero_Buffer(mrbch_pdu,
	      mrbch_size_bytes+8);



#ifndef USER_MODE
  time_in = openair_get_mbox();
#endif //USER_MODE

  //  printf("mrbch_size_bits = %d\n",mrbch_size_bits);

  phy_viterbi_dot11_sse2(PHY_vars->mrbch_data[0].demod_data,
			 mrbch_pdu,
			 mrbch_size_bits);


#ifndef USER_MODE
  time_out = openair_get_mbox();
#endif // USER_MODE


  oldcrc= *((unsigned int *)(&mrbch_pdu[mrbch_size_bytes-4]));
  oldcrc&=0x00ffffff;

  crc = crc24(mrbch_pdu,
	      (mrbch_size_bytes-4)<<3)>>8;
  
  
  
#ifdef DEBUG_PHY    
  msg("Received CRC : %x\n",oldcrc);
  msg("Computed CRC : %x\n",crc);
#endif // DEBUG_PHY
  
  // descramble data
  for (i=0;
       i<mrbch_size_bytes-4;
       i++) {
    mrbch_pdu[i] = mrbch_pdu[i] ^ scrambling_sequence[i];

    //            printf("%d:%d\n",i,mrbch_pdu[i]);
    // store the needed part for the MAC
    if (i<mrbch_pdu_length_bytes)
      mrbch_mac_pdu[i] = mrbch_pdu[i];
  }

#ifdef USER_MODE
#ifdef DEBUG_PHY    
      write_output("scrambler.m","scrambling_seq",
		   (void*)scrambling_sequence,
		   1024,
		   1,5);
 
#endif //DEBUG_PHY
#endif //USER_MODE

#ifndef USER_MODE
  time_out = openair_get_mbox();
  /*
  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CODING][MRBCH] Frame %d: mrbch channel decoding : time_in %d,time_out %d\n",mac_xface->frame,time_in,time_out);
  */
#endif //USER_MODE


  _mm_empty();

  return (crc==oldcrc) ? 0 : -1;
}

#else //EXPRESSMIMO_TARGET

int phy_decode_mrbch(unsigned char sch_index,
		     unsigned char nb_antennas_rx,
		     unsigned char nb_antennas_tx,
		     unsigned char *mrbch_mac_pdu,
		     unsigned int mrbch_pdu_length_bytes) { 
}


#endif //EXPRESSMIMO_TARGET

#ifndef USER_MODE
EXPORT_SYMBOL(phy_generate_mrbch);
EXPORT_SYMBOL(phy_decode_mrbch);
#endif // USER_MODE

