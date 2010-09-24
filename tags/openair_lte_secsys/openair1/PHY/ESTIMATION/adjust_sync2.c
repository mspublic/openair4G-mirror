#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"

#ifndef PLATON
#ifndef USER_MODE
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif //USER_MODE
#endif //PLATON

// Adjust location synchronization point to account for drift
// The adjustment is performed once per frame based on the
// last channel estimate of the receiver


void phy_adjust_synch_multi_CH(unsigned char clear, short coef, SCH_t sch_type)
{

  static int max_pos_fil[NUMBER_OF_CHSCH_SYMBOLS_MAX] = {0,0,0,0};
  int   sch_idx;
  int   peaks_found = 0;
  int   first_peak = 0;
  int   diff;


#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: rx_offset (before) = %d\n",mac_xface->frame,PHY_vars->rx_vars[0].offset);
#endif //DEBUG_PHY

  for (sch_idx = 1; sch_idx < NUMBER_OF_CHSCH_SYMBOLS_MAX; sch_idx++) {
    if (phy_calc_timing_offset(clear, sch_idx, coef, sch_type, max_pos_fil+sch_idx) == 0) {
      peaks_found += 1;
      if (sch_idx == 1) {
	first_peak = max_pos_fil[sch_idx];
	break;
      }
      if (peaks_found==1)
	first_peak = max_pos_fil[sch_idx];
      else if (max_pos_fil[sch_idx] < first_peak)
	first_peak = max_pos_fil[sch_idx];
    }
  }
  
  if (peaks_found>0)
    diff = first_peak - (NUMBER_OF_OFDM_CARRIERS - (7*CYCLIC_PREFIX_LENGTH/8));
  else 
    diff = 0;

  if ( diff > SYNCH_HYST )
    PHY_vars->rx_vars[0].offset++;
  else if (diff < -SYNCH_HYST)
    PHY_vars->rx_vars[0].offset--;
    
  if ( PHY_vars->rx_vars[0].offset < 0 )
    PHY_vars->rx_vars[0].offset += FRAME_LENGTH_COMPLEX_SAMPLES;

#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: peaks_found = %d, first_peak = %d, diff = %d, rx_offset (after) = %d\n",mac_xface->frame, peaks_found, first_peak, diff, PHY_vars->rx_vars[0].offset);
#endif //DEBUG_PHY

#ifndef USER_MODE
#ifndef PHY_EMUL
#ifndef NOCARD_TEST
#ifndef PLATON
    pci_interface->frame_offset = PHY_vars->rx_vars[0].offset;
#endif // PLATON
#endif // PHY_EMUL
#endif // NOCARD_TEST
#endif // PHY_EMUL

}


int phy_adjust_sync_CH2(int clear, int sch_index, short coef, SCH_t sch_type)
{

  static int first_peak_fil = 0;
  int *channel2;
  int i, aa;
  int diff;
  int mean_val = 0;
  int first_peak = -1;
  short ncoef;

  ncoef = 32767 - coef;

#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: rx_offset (before) = %d\n",mac_xface->frame,PHY_vars->rx_vars[0].offset);
#endif //DEBUG_PHY

  if (sch_type == CHSCH) {
    channel2 = PHY_vars->chsch_data[sch_index].mag_channel;
    for (aa=0;aa<NB_ANTENNAS_RX;aa++) {
      mult_cpx_vector_h_add32((s16*)PHY_vars->chsch_data[sch_index].channel[aa], 
			      (s16*)PHY_vars->chsch_data[sch_index].channel[aa], 
			      (s16*)PHY_vars->chsch_data[sch_index].mag_channel, 
			      NUMBER_OF_OFDM_CARRIERS, 
			      1);
    }

    for (i = (NUMBER_OF_OFDM_CARRIERS-CYCLIC_PREFIX_LENGTH);
	 i < NUMBER_OF_OFDM_CARRIERS;
	 i++) {
      mean_val = mean_val + PHY_vars->chsch_data[sch_index].mag_channel[i*2];
    }
  }
  else {
    channel2 = PHY_vars->sch_data[sch_index].mag_channel;
    for (aa=0;aa<NB_ANTENNAS_RX;aa++) {
      mult_cpx_vector_h_add32((s16*)PHY_vars->sch_data[sch_index].channel[aa], 
			      (s16*)PHY_vars->sch_data[sch_index].channel[aa], 
			      (s16*)PHY_vars->sch_data[sch_index].mag_channel, 
			      NUMBER_OF_OFDM_CARRIERS, 
			      1);
    }

    for (i = (NUMBER_OF_OFDM_CARRIERS-CYCLIC_PREFIX_LENGTH);
	 i < NUMBER_OF_OFDM_CARRIERS;
	 i++) {
      mean_val = mean_val + PHY_vars->sch_data[sch_index].mag_channel[i*2];
    }
  }
  mean_val = mean_val/(NUMBER_OF_OFDM_CARRIERS-CYCLIC_PREFIX_LENGTH);

#ifdef DEBUG_PHY
	write_output("channel2.m","chan2",
		     (short *)&PHY_vars->chsch_data[sch_index].mag_channel[0],2*NUMBER_OF_OFDM_CARRIERS,2,2);

#endif

  for (i = (NUMBER_OF_OFDM_CARRIERS-CYCLIC_PREFIX_LENGTH);
       i < NUMBER_OF_OFDM_CARRIERS;
       i++) {
    
    if (channel2[2*i] > 10*mean_val){
      first_peak = i;
      break;
    }
  }    

  if (first_peak == -1)  //no peak was found
    return(-1);

  // filter position to reduce jitter
  if (clear == 1) 
    first_peak_fil = first_peak;
  else 
    first_peak_fil = ((first_peak_fil * coef) + (first_peak * ncoef)) >> 15;


#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d, sch_index %d: first_peak = %d, max_val = %d, mean_val = %d, max_pos_fil = %d\n",mac_xface->frame,sch_index,first_peak,channel2[2*first_peak],mean_val,first_peak_fil);
#endif

  diff = first_peak_fil - (NUMBER_OF_OFDM_CARRIERS - (7*CYCLIC_PREFIX_LENGTH/8));

  if ( diff > SYNCH_HYST )
    PHY_vars->rx_vars[0].offset++;
  else if (diff < -SYNCH_HYST)
    PHY_vars->rx_vars[0].offset--;
    
  if ( PHY_vars->rx_vars[0].offset < 0 )
    PHY_vars->rx_vars[0].offset += FRAME_LENGTH_COMPLEX_SAMPLES;

#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: first_peak = %d, diff = %d, rx_offset (after) = %d\n",mac_xface->frame, first_peak, diff, PHY_vars->rx_vars[0].offset);
#endif //DEBUG_PHY

#ifndef USER_MODE
#ifndef PHY_EMUL
#ifndef NOCARD_TEST
#ifndef PLATON
    pci_interface->frame_offset = PHY_vars->rx_vars[0].offset;
#endif // PLATON
#endif // PHY_EMUL
#endif // NOCARD_TEST
#endif // PHY_EMUL

    return(0);

}
