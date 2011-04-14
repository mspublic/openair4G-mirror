// Author: Florian Kaltenberger
// This file is based on phy_adjust_sync with the difference, that it does 
// not actually adjust the offset, but returns it as a parameter
// Date: 11.4.2008


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

 
int phy_calc_timing_offset(int clear, int sch_index, short coef, SCH_t sch_type, int* max_pos_fil)
{

  //static int max_pos_fil = 0;
  //static int first_run = 1;
  int temp, i, aa;
  //int offset,diff;
  int mean_val = 0, max_val = 0, max_pos = 0;
  short Re,Im,ncoef;

  ncoef = 32767 - coef;

  if (sch_type == CHSCH) {
    for (i = (NUMBER_OF_OFDM_CARRIERS-CYCLIC_PREFIX_LENGTH);
	 i < NUMBER_OF_OFDM_CARRIERS;
	 i++) {
      temp = 0;
      for (aa=0;aa<NB_ANTENNAS_RX;aa++) {
	Re = ((s16*)PHY_vars->chsch_data[sch_index].channel[aa])[(i<<2)];
	Im = ((s16*)PHY_vars->chsch_data[sch_index].channel[aa])[1+(i<<2)];
      
    
	temp += (Re*Re/2) + (Im*Im/2);
      }
      if (temp > max_val) {
	max_pos = i;
	max_val = temp;
      }
      mean_val = mean_val + temp;
    }
  }
  else {
    for (i = (NUMBER_OF_OFDM_CARRIERS-CYCLIC_PREFIX_LENGTH);
	 i < NUMBER_OF_OFDM_CARRIERS;
	 i++) {
      temp = 0;
      for (aa=0;aa<NB_ANTENNAS_RX;aa++) {
	Re = ((s16*)PHY_vars->sch_data[sch_index].channel[aa])[(i<<2)];
	Im = ((s16*)PHY_vars->sch_data[sch_index].channel[aa])[1+(i<<2)];
	
	
	temp += (Re*Re/2) + (Im*Im/2);
      }
      if (temp > max_val) {
	max_pos = i;
	max_val = temp;
      }
      mean_val = mean_val + temp;
    }
  }
  mean_val = mean_val/(NUMBER_OF_OFDM_CARRIERS-CYCLIC_PREFIX_LENGTH);

  // see if there was actually a peak
  if (max_val < 10*mean_val)
    return(-1); //leave max_pos_fil untouched!!!
  else { 
    // filter position to reduce jitter
    if (clear == 1) 
      *max_pos_fil = max_pos;
    else 
      *max_pos_fil = ((*max_pos_fil * coef) + (max_pos * ncoef)) >> 15;
  }

#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d, sch_index %d: max_pos = %d, max_val = %d, mean_val = %d, max_pos_fil = %d\n",mac_xface->frame,sch_index,max_pos,max_val,mean_val,*max_pos_fil);
#endif

  //*timing_offset = max_pos_fil - (NUMBER_OF_OFDM_CARRIERS - (7*CYCLIC_PREFIX_LENGTH/8));

  return(0);
}


