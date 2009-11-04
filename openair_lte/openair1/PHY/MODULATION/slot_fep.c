#include "PHY/defs.h"
#include "defs.h"

void slot_fep(LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned char l,
	      unsigned char Ns,
	      int **rxdata,
	      int **rxdataF,
	      int **dl_ch_estimates,
	      int offset) {
 
  unsigned char aa,symbol = l+((7-frame_parms->Ncp)*(Ns&1));

  //#ifdef DEBUG_PHY
    msg("slot_fep: offset %d, symbol %d, prefix %d\n",offset, symbol,frame_parms->nb_prefix_samples);
  //#endif
  
  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
    fft((short *)&rxdata[aa][frame_parms->nb_prefix_samples + (frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)*symbol+offset],
	(short*)&rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	frame_parms->twiddle_fft,
	frame_parms->rev,
	frame_parms->log2_symbol_size,
	frame_parms->log2_symbol_size>>1,
	0);
  }

  if ((l==0) || (l==(4-frame_parms->Ncp))) {
    for (aa=0;aa<frame_parms->nb_antennas_tx;aa++)
      lte_dl_channel_estimation(dl_ch_estimates,
				rxdataF,
				frame_parms,
				Ns,
				aa,
				l,
				symbol);
    

  }

}
