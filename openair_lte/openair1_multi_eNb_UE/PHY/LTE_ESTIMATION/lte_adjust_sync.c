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

void lte_adjust_synch(LTE_DL_FRAME_PARMS *frame_parms,
		      PHY_VARS_UE *phy_vars_ue,
		      unsigned char eNb_id,
		      unsigned char clear,
		      short coef)
{


  int temp, i, aa, max_val = 0, max_pos = 0;
  int offset,diff;
  short Re,Im,ncoef;

  ncoef = 32767 - coef;
  offset = phy_vars_ue->rx_offset;

#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: rx_offset (before) = %d\n",mac_xface->frame,offset);
#endif //DEBUG_PHY

  // do ifft of channel estimate
  for (aa=0;aa<frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx;aa++) {
    fft((short*) &phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNb_id][aa][LTE_CE_OFFSET],
	(short*) phy_vars_ue->lte_ue_common_vars.dl_ch_estimates_time[aa],
	frame_parms->twiddle_ifft,
	frame_parms->rev,
	frame_parms->log2_symbol_size,
	frame_parms->log2_symbol_size/2,
	0);
  }

  // we only use channel estimates from tx antenna 0 here
  for (i = 0; i < frame_parms->nb_prefix_samples; i++) {
    temp = 0;
    for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
      Re = ((s16*)phy_vars_ue->lte_ue_common_vars.dl_ch_estimates_time[aa])[(i<<2)];
      Im = ((s16*)phy_vars_ue->lte_ue_common_vars.dl_ch_estimates_time[aa])[1+(i<<2)];
      temp += (Re*Re/2) + (Im*Im/2);
    }
    if (temp > max_val) {
      max_pos = i;
      max_val = temp;
    }
  }

  // filter position to reduce jitter
  if (clear == 1)
    max_pos_fil = max_pos;
  else
    max_pos_fil = ((max_pos_fil * coef) + (max_pos * ncoef)) >> 15;


  diff = max_pos_fil - frame_parms->nb_prefix_samples/8;

  if ( diff > SYNCH_HYST )
    offset++;
  else if (diff < -SYNCH_HYST)
    offset--;
    
  if ( offset < 0 )
    offset += FRAME_LENGTH_COMPLEX_SAMPLES;

  if ( offset >= FRAME_LENGTH_COMPLEX_SAMPLES )
    offset -= FRAME_LENGTH_COMPLEX_SAMPLES;

   phy_vars_ue->rx_offset = offset;


#ifdef DEBUG_PHY
 if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: rx_offset (after) = %d : max_pos = %d,max_pos_fil = %d\n",mac_xface->frame,phy_vars_ue->rx_offset,max_pos,max_pos_fil);
#endif //DEBUG_PHY

#ifndef USER_MODE
#ifndef PHY_EMUL
#ifndef NOCARD_TEST
#ifndef PLATON
    pci_interface->frame_offset = offset;
  //  openair_dma(ADJUST_SYNCH);
#endif //PLATON
#endif //PHY_EMUL
#endif // NOCARD_TEST
#endif // PHY_EMUL

}
