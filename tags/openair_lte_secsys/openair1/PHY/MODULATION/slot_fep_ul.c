#include "PHY/defs.h"
#include "defs.h"
//#define DEBUG_FEP

int slot_fep_ul(LTE_DL_FRAME_PARMS *frame_parms,
		LTE_eNB_COMMON *eNb_common_vars,
		unsigned char l,
		unsigned char Ns,
		unsigned char eNb_id,
		int no_prefix) {
#ifdef DEBUG_FEP
  char fname[40], vname[40];
#endif
  unsigned char aa;
  unsigned char symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within sub-frame
  unsigned int nb_prefix_samples = (no_prefix ? 0 : frame_parms->nb_prefix_samples);
  unsigned int offset = (frame_parms->ofdm_symbol_size + nb_prefix_samples) * frame_parms->symbols_per_tti * (Ns>>1);

  if (l<0 || l>=7-frame_parms->Ncp) {
    msg("slot_fep: l must be between 0 and %d\n",7-frame_parms->Ncp);
    return(-1);
  }
  if (Ns<0 || Ns>=20) {
    msg("slot_fep: Ns must be between 0 and 19\n");
    return(-1);
  }

#ifdef DEBUG_FEP
  msg("slot_fep: offset %d, symbol %d, nb_prefix_samples %d\n",offset, symbol, nb_prefix_samples);
#endif

  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {

#ifdef DEBUG_FEP
    if ((Ns==5) && (eNb_id==0) && (aa==0)) 
      write_output("eNb_rx.m","rxs",&eNb_common_vars->rxdata[0][0][nb_prefix_samples + (frame_parms->ofdm_symbol_size+nb_prefix_samples)*symbol+offset],((frame_parms->ofdm_symbol_size+nb_prefix_samples)*6),1,1);
    if ((Ns==7) && (eNb_id==0) && (symbol==11)) 
      write_output("eNb_rx.m","rxs",&eNb_common_vars->rxdata[0][aa][nb_prefix_samples + (frame_parms->ofdm_symbol_size+nb_prefix_samples)*symbol+offset],(frame_parms->ofdm_symbol_size),1,1);
#endif

    fft((short *)&eNb_common_vars->rxdata[eNb_id][aa][nb_prefix_samples + (frame_parms->ofdm_symbol_size+nb_prefix_samples)*symbol+offset],
	(short*)&eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
	frame_parms->twiddle_fft,
	frame_parms->rev,
	frame_parms->log2_symbol_size,
	frame_parms->log2_symbol_size>>1,
	0);
  }

  if ((l==(6-frame_parms->Ncp)) && (Ns%2==1)) {
    for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
#ifdef DEBUG_FEP
      msg("Channel estimation eNb %d, aarx %d, %p, %p, %p\n",eNb_id,aa,
	  &eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
	  eNb_common_vars->srs,
	  eNb_common_vars->srs_ch_estimates[eNb_id][aa]);
#endif

      //write_output("eNb_rxF.m","rxF",&eNb_common_vars->rxdataF[0][aa][2*frame_parms->ofdm_symbol_size*symbol],2*(frame_parms->ofdm_symbol_size),2,1);
      //write_output("eNb_srs.m","srs_eNb",eNb_common_vars->srs,(frame_parms->ofdm_symbol_size),1,1);

      mult_cpx_vector_norep((short*) &eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
			    (short*) eNb_common_vars->srs,
			    (short*) eNb_common_vars->srs_ch_estimates[eNb_id][aa],
			    frame_parms->ofdm_symbol_size,
			    15);
#ifdef DEBUG_FEP
	sprintf(fname,"eNB_id%d_an%d_srs_ch_est.m",eNb_id,aa);
	sprintf(vname,"eNB%d_%d_srs_ch_est",eNb_id,aa);
	write_output(fname,vname,eNb_common_vars->srs_ch_estimates[eNb_id][aa],frame_parms->ofdm_symbol_size,1,1);
#endif
    }
  }

#ifdef DEBUG_FEP
  msg("slot_fep: done\n");
#endif
  return(0);
}
