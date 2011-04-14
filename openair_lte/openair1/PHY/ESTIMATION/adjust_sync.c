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

static int      max_pos_fil = 0;

void
phy_adjust_synch(unsigned char clear,int sch_index,short coef, SCH_t sch_type)
{


  int             temp, i, aa, max_val = 0, max_pos = 0;
  int offset,diff;
  short Re,Im,ncoef;

  ncoef = 32767 - coef;

  //#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: rx_offset (before) = %d\n",mac_xface->frame,PHY_vars->rx_vars[0].offset);
  //#endif //DEBUG_PHY

  //BUGFIX fk+rk 31.7.2008 the offset is implicit in the loop below. it should no longer be used  
  //offset = (NUMBER_OF_OFDM_CARRIERS-CYCLIC_PREFIX_LENGTH)<<1;
  
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
    }
  }


  // filter position to reduce jitter
  if (clear == 1)
    max_pos_fil = max_pos;
  else
    max_pos_fil = ((max_pos_fil * coef) + (max_pos * ncoef)) >> 15;


  // update start of frame offset, position maximum at peak_location
  //  PHY_vars->rx_vars[0].offset += (max_pos_fil - (NUMBER_OF_OFDM_CARRIERS - (CYCLIC_PREFIX_LENGTH/2)));
  //  if (PHY_vars->rx_vars[0].offset < 0)
  //    PHY_vars->rx_vars[0].offset += FRAME_LENGTH_COMPLEX_SAMPLES;
  
  diff = max_pos_fil - (NUMBER_OF_OFDM_CARRIERS - (7*CYCLIC_PREFIX_LENGTH/8));

  if ( diff > SYNCH_HYST )
    PHY_vars->rx_vars[0].offset++;
  else if (diff < -SYNCH_HYST)
    PHY_vars->rx_vars[0].offset--;
    
  //  frame_offset_adjusted += (max_pos_fil - (NUMBER_OF_OFDM_CARRIERS - (CYCLIC_PREFIX_LENGTH/2)));
  if (     PHY_vars->rx_vars[0].offset < 0 )
        PHY_vars->rx_vars[0].offset += FRAME_LENGTH_COMPLEX_SAMPLES;

  //#ifdef DEBUG_PHY
 if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: rx_offset (after) = %d : max_pos = %d,max_pos_fil = %d\n",mac_xface->frame,PHY_vars->rx_vars[0].offset,max_pos,max_pos_fil);
 //#endif //DEBUG_PHY

#ifndef USER_MODE
#ifndef PHY_EMUL
#ifndef NOCARD_TEST
#ifndef PLATON
    pci_interface->frame_offset = PHY_vars->rx_vars[0].offset;
  //  openair_dma(ADJUST_SYNCH);
#endif //PLATON
#endif //PHY_EMUL
#endif // NOCARD_TEST
#endif // PHY_EMUL

}
