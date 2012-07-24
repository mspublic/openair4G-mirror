#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif 

#define DEBUG_PHY

// Adjust location synchronization point to account for drift
// The adjustment is performed once per frame based on the
// last channel estimate of the receiver

void lte_adjust_synch(LTE_DL_FRAME_PARMS *frame_parms,
		      PHY_VARS_UE *phy_vars_ue,
		      unsigned char eNB_id,
		      unsigned char clear,
		      short coef)
{

  static int max_pos_fil = 0;
  int temp, i, aa, max_val = 0, max_pos = 0;
  int diff;
  short Re,Im,ncoef;

  ncoef = 32767 - coef;

#ifdef DEBUG_PHY
  if (phy_vars_ue->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: rx_offset (before) = %d\n",phy_vars_ue->frame,phy_vars_ue->rx_offset);
#endif //DEBUG_PHY


  // we only use channel estimates from tx antenna 0 here
  for (i = 0; i < frame_parms->nb_prefix_samples; i++) {
    temp = 0;
    for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
      Re = ((s16*)phy_vars_ue->lte_ue_common_vars.dl_ch_estimates_time[eNB_id][aa])[(i<<2)];
      Im = ((s16*)phy_vars_ue->lte_ue_common_vars.dl_ch_estimates_time[eNB_id][aa])[1+(i<<2)];
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


  diff = max_pos_fil - 16; //frame_parms->nb_prefix_samples/8;

  if ( diff > SYNCH_HYST )
    phy_vars_ue->rx_offset++;
  else if (diff < -SYNCH_HYST)
    phy_vars_ue->rx_offset--;
    
  if ( phy_vars_ue->rx_offset < 0 )
    phy_vars_ue->rx_offset += FRAME_LENGTH_COMPLEX_SAMPLES;

  if ( phy_vars_ue->rx_offset >= FRAME_LENGTH_COMPLEX_SAMPLES )
    phy_vars_ue->rx_offset -= FRAME_LENGTH_COMPLEX_SAMPLES;



#ifdef DEBUG_PHY
  if (phy_vars_ue->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: rx_offset (after) = %d : max_pos = %d,max_pos_fil = %d\n",phy_vars_ue->frame,phy_vars_ue->rx_offset,max_pos,max_pos_fil);
#endif //DEBUG_PHY

#ifdef CBMIMO1
  pci_interface[0]->frame_offset = phy_vars_ue->rx_offset;
#endif

}

int max_val;

#undef DEBUG_PHY

int lte_est_timing_advance(LTE_DL_FRAME_PARMS *frame_parms,
			   LTE_eNB_SRS *lte_eNb_srs,
			   unsigned int  *eNB_id,
			   unsigned char clear,
			   unsigned char number_of_cards,
			   short coef)

{

  static int max_pos_fil2 = 0;
  int temp, i, aa, max_pos = 0,ind;
  short Re,Im,ncoef;
#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[100],vname[100];
#endif
#endif

  ncoef = 32768 - coef;

  for (ind=0;ind<number_of_cards;ind++) {

    if (ind==0)
      max_val=0;


    for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
      // do ifft of channel estimate
      fft((short*) &lte_eNb_srs->srs_ch_estimates[ind][aa][0],
	  (short*) lte_eNb_srs->srs_ch_estimates_time[ind][aa],
	  frame_parms->twiddle_ifft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size/2,
	  0);

#ifdef USER_MODE
#ifdef DEBUG_PHY
      sprintf(fname,"srs_ch_estimates_time_%d%d.m",ind,aa);
      sprintf(vname,"srs_time_%d%d",ind,aa);
      write_output(fname,vname,lte_eNb_srs->srs_ch_estimates_time[ind][aa],frame_parms->ofdm_symbol_size*2,2,1);
#endif
#endif
    }

    // we only use channel estimates from tx antenna 0 here
    // remember we fixed the SRS to use only every second subcarriers
    for (i = 0; i < frame_parms->ofdm_symbol_size/2; i++) {
      temp = 0;
      for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
	Re = ((s16*)lte_eNb_srs->srs_ch_estimates_time[ind][aa])[(i<<2)];
	Im = ((s16*)lte_eNb_srs->srs_ch_estimates_time[ind][aa])[1+(i<<2)];
	temp += (Re*Re/2) + (Im*Im/2);
      }
      if (temp > max_val) {
	max_pos = i; 
	max_val = temp;
	*eNB_id = ind;
      }
    }
  }

  // filter position to reduce jitter
  if (clear == 1)
    max_pos_fil2 = max_pos;
  else
    max_pos_fil2 = ((max_pos_fil2 * coef) + (max_pos * ncoef)) >> 15;
  
#ifdef DEBUG_PHY
  if (mac_xface->frame%100 == 0)
    msg("[PHY][Adjust Sync] frame %d: max_pos = %d, max_pos_fil = %d\n",mac_xface->frame,max_pos,max_pos_fil2);
#endif //DEBUG_PHY
  
  return(max_pos_fil2);
}
