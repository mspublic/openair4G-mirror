/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/dlsch_demodulation.c
 * \brief Top-level routines for demodulating the PDSCH physical channel from 36-211, V8.6 2009-03
 * \author R. Knopp, F. Kaltenberger,A. Bhamri, S. Aubert
 * \date 2011
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr,ankit.bhamri@eurecom.fr,sebastien.aubert@eurecom.fr
 * \note
 * \warning
 */

#ifdef __SSE2__
#include <emmintrin.h>
#include <xmmintrin.h>
#endif
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"


#ifndef __SSE3__
__m128i zero;//,tmp_over_sqrt_10,tmp_sum_4_over_sqrt_10,tmp_sign,tmp_sign_3_over_sqrt_10;
//#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_abs_epi16(xmmx) _mm_add_epi16(_mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx))),_mm_srli_epi16(_mm_cmpgt_epi16(zero,(xmmx)),15))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmy)))
#endif

#ifndef USER_MODE
#define NOCYGWIN_STATIC static
#else
#define NOCYGWIN_STATIC 
#endif

//#define DEBUG_PHY 1
__m128i mmtmpD0,mmtmpD1,mmtmpD2,mmtmpD3,QAM_amp128,QAM_amp128b,avg128D;
int avg[4];
int offset_mumimo_llr_drange[29]={8,7,7,7,6,6,6,5,4,3,6,5,5,3,2,2,2,3,3,3,3,2,2,0,0,0,0,0,0};
int rx_pdsch(PHY_VARS_UE *phy_vars_ue,
             PDSCH_t type,
             unsigned char eNB_id,
             unsigned char eNB_id_i, //if this == phy_vars_ue->n_connected_eNB, we assume MU interference
             u8 subframe,
             unsigned char symbol,
             unsigned char first_symbol_flag,
             unsigned char dual_stream_flag,
             unsigned char i_mod) {
  
  LTE_UE_COMMON *lte_ue_common_vars  = phy_vars_ue->lte_ue_common_vars[eNB_id];
  LTE_UE_PDSCH **lte_ue_pdsch_vars;
  LTE_DL_FRAME_PARMS *frame_parms    = phy_vars_ue->lte_frame_parms[eNB_id];
  PHY_MEASUREMENTS *phy_measurements = &phy_vars_ue->PHY_measurements;
  LTE_UE_DLSCH_t   **dlsch_ue;

  unsigned char aatx,aarx,symbol_mod,harq_pid0;    
  unsigned short nb_rb;
  int avgs, rb;  
  
  switch (type) {
  case SI_PDSCH:
    lte_ue_pdsch_vars = phy_vars_ue->lte_ue_pdsch_vars_SI;
    dlsch_ue          = &phy_vars_ue->dlsch_ue_SI[eNB_id];
    break;
  case RA_PDSCH:
    lte_ue_pdsch_vars = phy_vars_ue->lte_ue_pdsch_vars_ra;
    dlsch_ue          = &phy_vars_ue->dlsch_ue_ra[eNB_id];
    break;
  case CO_PDSCH:
    lte_ue_pdsch_vars = phy_vars_ue->lte_ue_pdsch_vars_co;
    dlsch_ue          = phy_vars_ue->dlsch_ue_co[0]; // To be indexed
    break;
  case PDSCH:
    lte_ue_pdsch_vars = phy_vars_ue->lte_ue_pdsch_vars;
    dlsch_ue          = phy_vars_ue->dlsch_ue[eNB_id];
    break;
  case PMCH:
    //msg("[PHY][UE %d][FATAL] Frame %d subframe %d: PMCH not supported yet\n",phy_vars_ue->frame,subframe,type);
    mac_xface->macphy_exit("");
    return(-1);
    break;
  default:
    //msg("[PHY][UE %d][FATAL] Frame %d subframe %d: Unknown PDSCH format %d\n",phy_vars_ue->frame,subframe,type);
    mac_xface->macphy_exit("");
    return(-1);
    break;
  }
  
  if (eNB_id > 2) {
    msg("dlsch_demodulation.c: Illegal eNB_id %d\n",eNB_id);
    return(-1);
  }
    
  if (!lte_ue_common_vars) {
    msg("dlsch_demodulation.c: Null lte_ue_common_vars\n");
    return(-1);
  }

  if (!dlsch_ue[0]) {
    msg("dlsch_demodulation.c: Null dlsch_ue pointer\n");
    return(-1);
  }

  if (!lte_ue_pdsch_vars) {
    msg("dlsch_demodulation.c: Null lte_ue_pdsch_vars pointer\n");
    return(-1);
  }
    
  if (!frame_parms) {
    msg("dlsch_demodulation.c: Null lte_frame_parms\n");
    return(-1);
  }
  //  printf("rx_dlsch : eNB_id %d, eNB_id_i %d, dual_stream_flag %d\n",eNB_id,eNB_id_i,dual_stream_flag); 
  //  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  /*
    if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
    pilots=1;
    else 
    pilots=0;
  */

  harq_pid0 = dlsch_ue[0]->current_harq_pid;
  
  if (frame_parms->nb_antennas_tx_eNB>1) {
#ifdef DEBUG_DLSCH_MOD     
    msg("dlsch: using pmi %x (%p), rb_alloc %x\n",pmi2hex_2Ar1(dlsch_ue[0]->pmi_alloc),dlsch_ue[0],dlsch_ue[0]->rb_alloc[0]);
#endif
    nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				   lte_ue_common_vars->dl_ch_estimates[0],
				   lte_ue_pdsch_vars[eNB_id]->rxdataF_ext,
				   lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
				   dlsch_ue[0]->pmi_alloc,
				   lte_ue_pdsch_vars[eNB_id]->pmi_ext,
				   dlsch_ue[0]->rb_alloc,
				   symbol,
				   subframe,
				   frame_parms);

    if (dual_stream_flag==1) {
      if (eNB_id_i<phy_vars_ue->n_connected_eNB)
	nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				       lte_ue_common_vars->dl_ch_estimates[eNB_id_i],
				       lte_ue_pdsch_vars[eNB_id_i]->rxdataF_ext,
				       lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				       dlsch_ue[0]->pmi_alloc,
				       lte_ue_pdsch_vars[eNB_id_i]->pmi_ext,
				       dlsch_ue[0]->rb_alloc,
				       symbol,
				       subframe,
				       frame_parms);
      else 
	nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				       lte_ue_common_vars->dl_ch_estimates[0],
				       lte_ue_pdsch_vars[eNB_id_i]->rxdataF_ext,
				       lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				       dlsch_ue[0]->pmi_alloc,
				       lte_ue_pdsch_vars[eNB_id_i]->pmi_ext,
				       dlsch_ue[0]->rb_alloc,
				       symbol,
				       subframe,
				       frame_parms);
    }
  } // if n_tx>1
  else {     
    nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
				     lte_ue_common_vars->dl_ch_estimates[0],
				     lte_ue_pdsch_vars[eNB_id]->rxdataF_ext,
				     lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
				     dlsch_ue[0]->pmi_alloc,
				     lte_ue_pdsch_vars[eNB_id]->pmi_ext,
				     dlsch_ue[0]->rb_alloc,
				     symbol,
				     subframe,
				     frame_parms);
        
    if (dual_stream_flag==1) {
      if (eNB_id_i<phy_vars_ue->n_connected_eNB)
	nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
					 lte_ue_common_vars->dl_ch_estimates[eNB_id_i],
					 lte_ue_pdsch_vars[eNB_id_i]->rxdataF_ext,
					 lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext,    
					 dlsch_ue[0]->pmi_alloc,
					 lte_ue_pdsch_vars[eNB_id_i]->pmi_ext,
					 dlsch_ue[0]->rb_alloc,
					 symbol,
					 subframe,
					 frame_parms);
      else 
	nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
					 lte_ue_common_vars->dl_ch_estimates[0],
					 lte_ue_pdsch_vars[eNB_id_i]->rxdataF_ext,
					 lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext,    
					 dlsch_ue[0]->pmi_alloc,
					 lte_ue_pdsch_vars[eNB_id_i]->pmi_ext,
					 dlsch_ue[0]->rb_alloc,
					 symbol,
					 subframe,
					 frame_parms);
    }
  } //else n_tx>1
  
  if (nb_rb==0) {
    msg("dlsch_modulation.c: nb_rb=0\n");
    return(-1);
  }

    // DL power control: Scaling of Channel estimates for PDSCH
    dlsch_scale_channel(lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
                        frame_parms,
                        dlsch_ue,
                        symbol,
                        nb_rb);
    
  if (first_symbol_flag==1) {
    dlsch_channel_level(lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
			frame_parms,
			avg,
			symbol,
			nb_rb);
#ifdef DEBUG_PHY
    msg("[DLSCH] avg[0] %d\n",avg[0]);
#endif
      
    // the channel gain should be the effective gain of precoding + channel
    // however lets be more conservative and set maxh = nb_tx*nb_rx*max(h_i)
    // in case of precoding we add an additional factor of two for the precoding gain
    avgs = 0;
    for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++)
      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
	avgs = cmax(avgs,avg[(aatx<<1)+aarx]);
    //	avgs = cmax(avgs,avg[(aarx<<1)+aatx]);
        
    
    lte_ue_pdsch_vars[eNB_id]->log2_maxh = (log2_approx(avgs)/2) + 2
      + log2_approx(frame_parms->nb_antennas_tx_eNB-1) //-1 because log2_approx counts the number of bits
      + log2_approx(frame_parms->nb_antennas_rx-1);

    if ((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode>=UNIFORM_PRECODING11) &&
	(dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode< DUALSTREAM_UNIFORM_PRECODING1) &&
	(dlsch_ue[0]->dl_power_off==1)) // we are in TM 6
      lte_ue_pdsch_vars[eNB_id]->log2_maxh++;

    // this version here applies the factor .5 also to the extra terms. however, it does not work so well as the one above
    /* K = Nb_rx         in TM1 
       Nb_tx*Nb_rx   in TM2,4,5
       Nb_tx^2*Nb_rx in TM6 */
    /*
      K = frame_parms->nb_antennas_rx*frame_parms->nb_antennas_tx_eNB; //that also covers TM1 since Nb_tx=1
      if ((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode>=UNIFORM_PRECODING11) &&
      (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode< DUALSTREAM_UNIFORM_PRECODING1) &&
      (dlsch_ue[0]->dl_power_off==1)) // we are in TM 6
      K *= frame_parms->nb_antennas_tx_eNB;

      lte_ue_pdsch_vars[eNB_id]->log2_maxh = (log2_approx(K*avgs)/2);
    */

#ifdef DEBUG_PHY
    msg("[DLSCH] log2_maxh = %d (%d,%d)\n",lte_ue_pdsch_vars[eNB_id]->log2_maxh,avg[0],avgs);
    msg("[DLSCH] mimo_mode = %d\n", dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode);
#endif
  }
  aatx = frame_parms->nb_antennas_tx_eNB;
  aarx = frame_parms->nb_antennas_rx;

  if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<UNIFORM_PRECODING11) {// no precoding

    dlsch_channel_compensation(lte_ue_pdsch_vars[eNB_id]->rxdataF_ext,
			       lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
			       lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
			       lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,
			       lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
			       (aatx>1) ? lte_ue_pdsch_vars[eNB_id]->rho : NULL,
			       frame_parms,
			       symbol,
			       first_symbol_flag,
			       get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
			       nb_rb,
			       lte_ue_pdsch_vars[eNB_id]->log2_maxh,
			       phy_measurements); // log2_maxh+I0_shift
#ifdef DEBUG_PHY
    if (symbol==5)
      write_output("rxF_comp_d.m","rxF_c_d",&lte_ue_pdsch_vars[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
#endif
      
    if ((dual_stream_flag==1) && 
	(eNB_id_i<phy_vars_ue->n_connected_eNB)) {
      // get MF output for interfering stream
      dlsch_channel_compensation(lte_ue_pdsch_vars[eNB_id_i]->rxdataF_ext,
				 lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				 lte_ue_pdsch_vars[eNB_id_i]->dl_ch_mag,
				 lte_ue_pdsch_vars[eNB_id_i]->dl_ch_magb,
				 lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp,
				 (aatx>1) ? lte_ue_pdsch_vars[eNB_id_i]->rho : NULL,
				 frame_parms,
				 symbol,
				 first_symbol_flag,
				 i_mod,
				 nb_rb,
				 lte_ue_pdsch_vars[eNB_id]->log2_maxh,
				 phy_measurements); // log2_maxh+I0_shift
#ifdef DEBUG_PHY
      if (symbol == 5) {
	write_output("rxF_comp_d.m","rxF_c_d",&lte_ue_pdsch_vars[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
	write_output("rxF_comp_i.m","rxF_c_i",&lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);     
      }
#endif 
	
      // compute correlation between signal and interference channels
      dlsch_dual_stream_correlation(frame_parms,
				    symbol,
				    nb_rb,
				    lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
				    lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				    lte_ue_pdsch_vars[eNB_id]->dl_ch_rho_ext,
				    lte_ue_pdsch_vars[eNB_id]->log2_maxh);
    }
  }
  else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<DUALSTREAM_UNIFORM_PRECODING1) {// single-layer precoding
    //    printf("Channel compensation for precoding\n");
    //    if ((dual_stream_flag==1) && (eNB_id_i==NUMBER_OF_CONNECTED_eNB_MAX)) {
    if ((dual_stream_flag==1) && (eNB_id_i==phy_vars_ue->n_connected_eNB)) {

        // Scale the channel estimates for interfering stream
        dlsch_scale_channel(lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext,
                            frame_parms,
                            dlsch_ue,
                            symbol,
                            nb_rb);      

      /* compute new log2_maxh for effective channel */
      if (first_symbol_flag==1) {
	// effective channel of desired user is always stronger than interfering eff. channel
	dlsch_channel_level_prec(lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext, frame_parms, lte_ue_pdsch_vars[eNB_id]->pmi_ext,	avg, symbol, nb_rb);
	

    avg[0] = log2_approx(avg[0]) - 13 + offset_mumimo_llr_drange[dlsch_ue[0]->harq_processes[harq_pid0]->mcs];
	
	lte_ue_pdsch_vars[eNB_id]->log2_maxh = cmax(avg[0],0);
	//printf("log1_maxh =%d\n",lte_ue_pdsch_vars[eNB_id]->log2_maxh);
      }      

      dlsch_channel_compensation_prec(lte_ue_pdsch_vars[eNB_id]->rxdataF_ext, lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext, lte_ue_pdsch_vars[eNB_id]->dl_ch_mag, lte_ue_pdsch_vars[eNB_id]->dl_ch_magb, lte_ue_pdsch_vars[eNB_id]->rxdataF_comp, lte_ue_pdsch_vars[eNB_id]->pmi_ext, frame_parms, phy_measurements, eNB_id, symbol, get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs), nb_rb, lte_ue_pdsch_vars[eNB_id]->log2_maxh, dlsch_ue[0]->dl_power_off);

      // if interference source is MU interference, assume opposite precoder was used at eNB

      // calculate opposite PMI
      for (rb=0;rb<nb_rb;rb++) {
	switch(lte_ue_pdsch_vars[eNB_id]->pmi_ext[rb]) {
	case 0:
	  lte_ue_pdsch_vars[eNB_id_i]->pmi_ext[rb]=1;
	  break;
	case 1:
	  lte_ue_pdsch_vars[eNB_id_i]->pmi_ext[rb]=0;
	  break;
	case 2:
	  lte_ue_pdsch_vars[eNB_id_i]->pmi_ext[rb]=3;
	  break;
	case 3:
	  lte_ue_pdsch_vars[eNB_id_i]->pmi_ext[rb]=2;
	  break;
	}
	//	if (rb==0)
	//	  printf("pmi %d, pmi_i %d\n",lte_ue_pdsch_vars[eNB_id]->pmi_ext[rb],lte_ue_pdsch_vars[eNB_id_i]->pmi_ext[rb]);
	
      }

      dlsch_channel_compensation_prec(lte_ue_pdsch_vars[eNB_id_i]->rxdataF_ext, lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext, lte_ue_pdsch_vars[eNB_id_i]->dl_ch_mag, lte_ue_pdsch_vars[eNB_id_i]->dl_ch_magb, lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp, lte_ue_pdsch_vars[eNB_id_i]->pmi_ext, frame_parms, phy_measurements, eNB_id_i, symbol, i_mod, nb_rb, lte_ue_pdsch_vars[eNB_id]->log2_maxh, dlsch_ue[0]->dl_power_off);
        
#ifdef DEBUG_PHY
      if (symbol==5) {
	write_output("rxF_comp_d.m","rxF_c_d",&lte_ue_pdsch_vars[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
	write_output("rxF_comp_i.m","rxF_c_i",&lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);    
      }
#endif  

      dlsch_dual_stream_correlation(frame_parms, symbol, nb_rb, lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext, lte_ue_pdsch_vars[eNB_id_i]->dl_ch_estimates_ext, lte_ue_pdsch_vars[eNB_id]->dl_ch_rho_ext, lte_ue_pdsch_vars[eNB_id]->log2_maxh);

    }
    else {
      dlsch_channel_compensation_prec(lte_ue_pdsch_vars[eNB_id]->rxdataF_ext,
				      lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
				      lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
				      lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,
				      lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
				      lte_ue_pdsch_vars[eNB_id]->pmi_ext,
				      frame_parms,
				      phy_measurements,
				      eNB_id,
				      symbol,
				      get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
				      nb_rb,
				      lte_ue_pdsch_vars[eNB_id]->log2_maxh,
				      1);
    }
  }

  //  printf("MRC\n");
  if (frame_parms->nb_antennas_rx > 1)
    dlsch_detection_mrc(frame_parms,
			lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
			lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp,
			lte_ue_pdsch_vars[eNB_id]->rho,
			lte_ue_pdsch_vars[eNB_id]->dl_ch_rho_ext,
			lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
			lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,
			lte_ue_pdsch_vars[eNB_id_i]->dl_ch_mag,
			lte_ue_pdsch_vars[eNB_id_i]->dl_ch_magb,
			symbol,
			nb_rb,
			dual_stream_flag); 

  //  printf("Combining");
  // Single-layer transmission formats
  if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<DUALSTREAM_UNIFORM_PRECODING1) {
    if ((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == SISO) ||
	((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode >= UNIFORM_PRECODING11) &&
	 (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode <= PUSCH_PRECODING0)))
      {}//dlsch_siso(frame_parms,lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp,symbol,nb_rb);
    else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == ALAMOUTI)
      dlsch_alamouti(frame_parms,lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,symbol,nb_rb);
    else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == ANTCYCLING)
      dlsch_antcyc(frame_parms,lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,symbol,nb_rb);
    else {
      msg("dlsch_rx: Unknown MIMO mode\n");
      return (-1);
    }

    //    printf("LLR");

    switch (get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs)) {
    case 2 : 
      if (dual_stream_flag == 0)
	dlsch_qpsk_llr(frame_parms,
		       lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
		       lte_ue_pdsch_vars[eNB_id]->llr[0],
		       symbol,first_symbol_flag,nb_rb,
		       adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
		       lte_ue_pdsch_vars[eNB_id]->llr128);
      else if (i_mod == 2) {
          dlsch_qpsk_qpsk_llr(frame_parms,
                lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
                lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp,
                lte_ue_pdsch_vars[eNB_id]->dl_ch_rho_ext,
                lte_ue_pdsch_vars[eNB_id]->llr[0],
                symbol,first_symbol_flag,nb_rb,
                adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
                lte_ue_pdsch_vars[eNB_id]->llr128);}
      else if (i_mod == 4) { 
          dlsch_qpsk_16qam_llr(frame_parms,
			    lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
			    lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp,
                lte_ue_pdsch_vars[eNB_id_i]->dl_ch_mag,
			    lte_ue_pdsch_vars[eNB_id]->dl_ch_rho_ext,
			    lte_ue_pdsch_vars[eNB_id]->llr[0],
			    symbol,first_symbol_flag,nb_rb,
			    adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
                lte_ue_pdsch_vars[eNB_id]->llr128);}
      else {
	msg("rx_dlsch.c : IC receiver only implemented for 4QAM-4QAM and 4QAM=16QAM\n");
	return(-1);
      }
      
      break;
    case 4 :
      if (dual_stream_flag == 0)
	dlsch_16qam_llr(frame_parms,
			lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
			lte_ue_pdsch_vars[eNB_id]->llr[0],
			lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
			symbol,first_symbol_flag,nb_rb,
			adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,4,subframe,symbol),
			lte_ue_pdsch_vars[eNB_id]->llr128);
      else if (i_mod == 2) {
        dlsch_16qam_qpsk_llr(frame_parms,
                             lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
                             lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp,
                             lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
                             lte_ue_pdsch_vars[eNB_id]->dl_ch_rho_ext,
                             lte_ue_pdsch_vars[eNB_id]->llr[0],
                             symbol,first_symbol_flag,nb_rb,
                             adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
                             lte_ue_pdsch_vars[eNB_id]->llr128);}
      else if (i_mod == 4) {
          dlsch_16qam_16qam_llr(frame_parms,
			      lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
			      lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp,
			      lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
			      lte_ue_pdsch_vars[eNB_id_i]->dl_ch_mag,
			      lte_ue_pdsch_vars[eNB_id]->dl_ch_rho_ext,
			      lte_ue_pdsch_vars[eNB_id]->llr[0],
			      symbol,first_symbol_flag,nb_rb,
			      adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
                  lte_ue_pdsch_vars[eNB_id]->llr128);}
      else {
          msg("rx_dlsch.c : IC receiver only implemented for 16QAM-4QAM and 16QAM-16QAM\n");
          return(-1);
      }
      break;
    case 6 :
      if (dual_stream_flag == 0)
          dlsch_64qam_llr(frame_parms,
                          lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
                          lte_ue_pdsch_vars[eNB_id]->llr[0],
                          lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
                          lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,
                          symbol,first_symbol_flag,nb_rb,
                          adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,6,subframe,symbol),
                          lte_ue_pdsch_vars[eNB_id]->llr128);
      else if (i_mod == 2) {
          msg("rx_dlsch.c : IC receiver only implemented for 64QAM-64QAM\n");
          return(-1);
      }
      else if (i_mod == 4) {
          msg("rx_dlsch.c : IC receiver only implemented for 64QAM-64QAM\n");
          return(-1);
      }
      else {	  
          dlsch_64qam_64qam_llr(frame_parms,
				lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
				lte_ue_pdsch_vars[eNB_id_i]->rxdataF_comp,
				lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
				lte_ue_pdsch_vars[eNB_id_i]->dl_ch_mag,
				lte_ue_pdsch_vars[eNB_id]->dl_ch_rho_ext,
				lte_ue_pdsch_vars[eNB_id]->llr[0],
				symbol,first_symbol_flag,nb_rb,
				adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
				lte_ue_pdsch_vars[eNB_id]->llr128);
	}
      break;
    default:
        msg("rx_dlsch.c : Unknown mod_order!!!!\n");
        return(-1);
        break;
    }
  } // single-layer transmission
  else  {
    msg("rx_dlsch.c : Dualstream not yet implemented\n");
    return(-1);
  }
  
  return(0);    
}

//==============================================================================================
// Pre-processing for LLR computation
//==============================================================================================

void dlsch_channel_compensation(int **rxdataF_ext,
                                int **dl_ch_estimates_ext,
                                int **dl_ch_mag,
                                int **dl_ch_magb,
                                int **rxdataF_comp,
                                int **rho,
                                LTE_DL_FRAME_PARMS *frame_parms,
                                unsigned char symbol,
                                u8 first_symbol_flag,
                                unsigned char mod_order,
                                unsigned short nb_rb,
                                unsigned char output_shift,
                                PHY_MEASUREMENTS *phy_measurements) {

  unsigned short rb;
  unsigned char aatx,aarx,symbol_mod,pilots=0;
  __m128i *dl_ch128,*dl_ch128_2,*dl_ch_mag128,*dl_ch_mag128b,*rxdataF128,*rxdataF_comp128,*rho128;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

#ifndef __SSE3__
  zero = _mm_xor_si128(zero,zero);
#endif

  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp))) {
      
    if (frame_parms->mode1_flag==1) // 10 out of 12 so don't reduce size    
      nb_rb=1+(5*nb_rb/6);
    else  
      pilots=1;    
  }

  for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++) {
    if (mod_order == 4) {
      QAM_amp128 = _mm_set1_epi16(QAM16_n1);  // 2/sqrt(10)
      QAM_amp128b = _mm_xor_si128(QAM_amp128b,QAM_amp128b);
    }    
    else if (mod_order == 6) {
      QAM_amp128  = _mm_set1_epi16(QAM64_n1); // 
      QAM_amp128b = _mm_set1_epi16(QAM64_n2);
    }
    
    //    printf("comp: rxdataF_comp %p, symbol %d\n",rxdataF_comp[0],symbol);

    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

      dl_ch128          = (__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch_mag128      = (__m128i *)&dl_ch_mag[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch_mag128b     = (__m128i *)&dl_ch_magb[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
      rxdataF_comp128   = (__m128i *)&rxdataF_comp[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];


      for (rb=0;rb<nb_rb;rb++) {
	if (mod_order>2) {  
	  // get channel amplitude if not QPSK
                
	  mmtmpD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128[0]);
	  mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
                
	  mmtmpD1 = _mm_madd_epi16(dl_ch128[1],dl_ch128[1]);
	  mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
                
	  mmtmpD0 = _mm_packs_epi32(mmtmpD0,mmtmpD1);
                
	  // store channel magnitude here in a new field of dlsch
                
	  dl_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpD0,mmtmpD0);
	  dl_ch_mag128b[0] = dl_ch_mag128[0];
	  dl_ch_mag128[0] = _mm_mulhi_epi16(dl_ch_mag128[0],QAM_amp128);
	  dl_ch_mag128[0] = _mm_slli_epi16(dl_ch_mag128[0],1);
                
	  dl_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpD0,mmtmpD0);
	  dl_ch_mag128b[1] = dl_ch_mag128[1];
	  dl_ch_mag128[1] = _mm_mulhi_epi16(dl_ch_mag128[1],QAM_amp128);
	  dl_ch_mag128[1] = _mm_slli_epi16(dl_ch_mag128[1],1);
                
	  if (pilots==0) {
	    mmtmpD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128[2]);
	    mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	    mmtmpD1 = _mm_packs_epi32(mmtmpD0,mmtmpD0);
                    
	    dl_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpD1,mmtmpD1);
	    dl_ch_mag128b[2] = dl_ch_mag128[2];
                    
	    dl_ch_mag128[2] = _mm_mulhi_epi16(dl_ch_mag128[2],QAM_amp128);
	    dl_ch_mag128[2] = _mm_slli_epi16(dl_ch_mag128[2],1);	  
	  }
                
	  dl_ch_mag128b[0] = _mm_mulhi_epi16(dl_ch_mag128b[0],QAM_amp128b);
	  dl_ch_mag128b[0] = _mm_slli_epi16(dl_ch_mag128b[0],1);
                
                
	  dl_ch_mag128b[1] = _mm_mulhi_epi16(dl_ch_mag128b[1],QAM_amp128b);
	  dl_ch_mag128b[1] = _mm_slli_epi16(dl_ch_mag128b[1],1);
                
	  if (pilots==0) {
	    dl_ch_mag128b[2] = _mm_mulhi_epi16(dl_ch_mag128b[2],QAM_amp128b);
	    dl_ch_mag128b[2] = _mm_slli_epi16(dl_ch_mag128b[2],1);	  
	  }
	}
	
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[0],rxdataF128[0]);
	//	print_ints("re",&mmtmpD0);
            
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
	//	print_ints("im",&mmtmpD1);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[0]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	//	print_ints("re(shift)",&mmtmpD0);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	//	print_ints("im(shift)",&mmtmpD1);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	//       	print_ints("c0",&mmtmpD2);
	//	print_ints("c1",&mmtmpD3);
	rxdataF_comp128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	//	print_shorts("rx:",rxdataF128);
	//	print_shorts("ch:",dl_ch128);
	//	print_shorts("pack:",rxdataF_comp128);
            
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[1],rxdataF128[1]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[1]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
            
	rxdataF_comp128[1] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	//	print_shorts("rx:",rxdataF128+1);
	//	print_shorts("ch:",dl_ch128+1);
	//	print_shorts("pack:",rxdataF_comp128+1);	
            
	if (pilots==0) {
	  // multiply by conjugated channel
	  mmtmpD0 = _mm_madd_epi16(dl_ch128[2],rxdataF128[2]);
	  // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	  mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
	  mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	  mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	  mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[2]);
	  // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	  mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	  mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	  mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	  mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
                
	  rxdataF_comp128[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	  //	print_shorts("rx:",rxdataF128+2);
	  //	print_shorts("ch:",dl_ch128+2);
	  //      	print_shorts("pack:",rxdataF_comp128+2);
                
	  dl_ch128+=3;
	  dl_ch_mag128+=3;
	  dl_ch_mag128b+=3;
	  rxdataF128+=3;
	  rxdataF_comp128+=3;
	}
	else { // we have a smaller PDSCH in symbols with pilots so skip last group of 4 REs and increment less
	  dl_ch128+=2;
	  dl_ch_mag128+=2;
	  dl_ch_mag128b+=2;
	  rxdataF128+=2;
	  rxdataF_comp128+=2;
	}
            
      }
    }
  }
  
  if (rho) {
      
      
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      rho128        = (__m128i *)&rho[aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch128      = (__m128i *)&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch128_2    = (__m128i *)&dl_ch_estimates_ext[2+aarx][symbol*frame_parms->N_RB_DL*12];
          
      for (rb=0;rb<nb_rb;rb++) {
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128_2[0]);
	//	print_ints("re",&mmtmpD0);
              
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
	//	print_ints("im",&mmtmpD1);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128_2[0]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	//	print_ints("re(shift)",&mmtmpD0);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	//	print_ints("im(shift)",&mmtmpD1);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	//       	print_ints("c0",&mmtmpD2);
	//	print_ints("c1",&mmtmpD3);
	rho128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
              
	//print_shorts("rx:",dl_ch128_2);
	//print_shorts("ch:",dl_ch128);
	//print_shorts("pack:",rho128);
              
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[1],dl_ch128_2[1]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128_2[1]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);

	
	rho128[1] =_mm_packs_epi32(mmtmpD2,mmtmpD3);
	//print_shorts("rx:",dl_ch128_2+1);
	//print_shorts("ch:",dl_ch128+1);
	//print_shorts("pack:",rho128+1);	
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128_2[2]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128_2[2]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
              
	rho128[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	//print_shorts("rx:",dl_ch128_2+2);
	//print_shorts("ch:",dl_ch128+2);
	//print_shorts("pack:",rho128+2);
              
	dl_ch128+=3;
	dl_ch128_2+=3;
	rho128+=3;
              
      }	
          
      if (first_symbol_flag==1) {
	phy_measurements->rx_correlation[0][aarx] = signal_energy(&rho[aarx][symbol*frame_parms->N_RB_DL*12],rb*12);
      }           
    }      
  }

  _mm_empty();
  _m_empty();
}     

void prec2A_128(unsigned char pmi,__m128i *ch0,__m128i *ch1) {
  
    __m128i amp;
    amp = _mm_set1_epi16(ONE_OVER_SQRT2_Q15);

  switch (pmi) {
        
  case 0 :   // +1 +1
    //    print_shorts("phase 0 :ch0",ch0);
    //    print_shorts("phase 0 :ch1",ch1);
    ch0[0] = _mm_adds_epi16(ch0[0],ch1[0]);   
    break;
  case 1 :   // +1 -1
    //    print_shorts("phase 1 :ch0",ch0);
    //    print_shorts("phase 1 :ch1",ch1);
    ch0[0] = _mm_subs_epi16(ch0[0],ch1[0]);
    //    print_shorts("phase 1 :ch0-ch1",ch0);
    break;
  case 2 :   // +1 +j
    ch1[0] = _mm_sign_epi16(ch1[0],*(__m128i*)&conjugate[0]);
    ch1[0] = _mm_shufflelo_epi16(ch1[0],_MM_SHUFFLE(2,3,0,1));
    ch1[0] = _mm_shufflehi_epi16(ch1[0],_MM_SHUFFLE(2,3,0,1));
    ch0[0] = _mm_subs_epi16(ch0[0],ch1[0]);
        
    break;   // +1 -j
  case 3 :
    ch1[0] = _mm_sign_epi16(ch1[0],*(__m128i*)&conjugate[0]);
    ch1[0] = _mm_shufflelo_epi16(ch1[0],_MM_SHUFFLE(2,3,0,1));
    ch1[0] = _mm_shufflehi_epi16(ch1[0],_MM_SHUFFLE(2,3,0,1));
    ch0[0] = _mm_adds_epi16(ch0[0],ch1[0]);
    break;
  }

  ch0[0] = _mm_mulhi_epi16(ch0[0],amp);
  ch0[0] = _mm_slli_epi16(ch0[0],1);
    
  _mm_empty();
  _m_empty();
}

void dlsch_channel_compensation_prec(int **rxdataF_ext,
                                     int **dl_ch_estimates_ext,
                                     int **dl_ch_mag,
                                     int **dl_ch_magb,
                                     int **rxdataF_comp,
                                     unsigned char *pmi_ext,
                                     LTE_DL_FRAME_PARMS *frame_parms,
                                     PHY_MEASUREMENTS *phy_measurements,
                                     int eNB_id,
                                     unsigned char symbol,
                                     unsigned char mod_order,
                                     unsigned short nb_rb,
                                     unsigned char output_shift,
                                     unsigned char dl_power_off) {
  
  unsigned short rb,Nre;
  __m128i *dl_ch128_0,*dl_ch128_1,*dl_ch_mag128,*dl_ch_mag128b,*rxdataF128,*rxdataF_comp128;
  unsigned char aarx=0,symbol_mod,pilots=0;
  int precoded_signal_strength=0,rx_power_correction;
    
  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
    
  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
    pilots=1;

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;
    
#ifndef __SSE3__
  zero = _mm_xor_si128(zero,zero);
#endif

  //printf("comp prec: symbol %d, pilots %d\n",symbol, pilots);

  if (mod_order == 4) {
    QAM_amp128 = _mm_set1_epi16(QAM16_n1);
    QAM_amp128b = _mm_xor_si128(QAM_amp128b,QAM_amp128b);
  }
  else if (mod_order == 6) {
    QAM_amp128  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128b = _mm_set1_epi16(QAM64_n2);
  }
    
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
        
    dl_ch128_0          = (__m128i *)&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch128_1          = (__m128i *)&dl_ch_estimates_ext[2+aarx][symbol*frame_parms->N_RB_DL*12];
        
        
    dl_ch_mag128      = (__m128i *)&dl_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch_mag128b     = (__m128i *)&dl_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128   = (__m128i *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];
        

    for (rb=0;rb<nb_rb;rb++) {
      // combine TX channels using precoder from pmi
#ifdef DEBUG_DLSCH_DEMOD
      printf("mode 6 prec: rb %d, pmi->%d\n",rb,pmi_ext[rb]);
#endif            
      prec2A_128(pmi_ext[rb],&dl_ch128_0[0],&dl_ch128_1[0]);
      prec2A_128(pmi_ext[rb],&dl_ch128_0[1],&dl_ch128_1[1]);

      if (pilots==0) {
	prec2A_128(pmi_ext[rb],&dl_ch128_0[2],&dl_ch128_1[2]); 
      }

      if (mod_order>2) {  
	// get channel amplitude if not QPSK
	
	mmtmpD0 = _mm_madd_epi16(dl_ch128_0[0],dl_ch128_0[0]);	
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
                
	mmtmpD1 = _mm_madd_epi16(dl_ch128_0[1],dl_ch128_0[1]);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
                
	mmtmpD0 = _mm_packs_epi32(mmtmpD0,mmtmpD1);
                
	dl_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpD0,mmtmpD0);
	dl_ch_mag128b[0] = dl_ch_mag128[0];
	dl_ch_mag128[0] = _mm_mulhi_epi16(dl_ch_mag128[0],QAM_amp128);
	dl_ch_mag128[0] = _mm_slli_epi16(dl_ch_mag128[0],1);
                
	//print_shorts("dl_ch_mag128[0]=",&dl_ch_mag128[0]);
                
	dl_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpD0,mmtmpD0);
	dl_ch_mag128b[1] = dl_ch_mag128[1];
	dl_ch_mag128[1] = _mm_mulhi_epi16(dl_ch_mag128[1],QAM_amp128);
	dl_ch_mag128[1] = _mm_slli_epi16(dl_ch_mag128[1],1);
                
	if (pilots==0) {
	  mmtmpD0 = _mm_madd_epi16(dl_ch128_0[2],dl_ch128_0[2]);
	  mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
                    
	  mmtmpD1 = _mm_packs_epi32(mmtmpD0,mmtmpD0);
                    
	  dl_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpD1,mmtmpD1);
	  dl_ch_mag128b[2] = dl_ch_mag128[2];
                    
	  dl_ch_mag128[2] = _mm_mulhi_epi16(dl_ch_mag128[2],QAM_amp128);
	  dl_ch_mag128[2] = _mm_slli_epi16(dl_ch_mag128[2],1);	  
	}
                
	dl_ch_mag128b[0] = _mm_mulhi_epi16(dl_ch_mag128b[0],QAM_amp128b);
	dl_ch_mag128b[0] = _mm_slli_epi16(dl_ch_mag128b[0],1);
                
	//print_shorts("dl_ch_mag128b[0]=",&dl_ch_mag128b[0]);
                
	dl_ch_mag128b[1] = _mm_mulhi_epi16(dl_ch_mag128b[1],QAM_amp128b);
	dl_ch_mag128b[1] = _mm_slli_epi16(dl_ch_mag128b[1],1);
                
	if (pilots==0) {
	  dl_ch_mag128b[2] = _mm_mulhi_epi16(dl_ch_mag128b[2],QAM_amp128b);
	  dl_ch_mag128b[2] = _mm_slli_epi16(dl_ch_mag128b[2],1);	  
	}
      }

      // MF multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128_0[0],rxdataF128[0]);
      //	print_ints("re",&mmtmpD0);
            
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128_0[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpD1);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[0]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      //	print_ints("re(shift)",&mmtmpD0);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      //	print_ints("im(shift)",&mmtmpD1);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      //       	print_ints("c0",&mmtmpD2);
      //	print_ints("c1",&mmtmpD3);
      rxdataF_comp128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      //	print_shorts("rx:",rxdataF128);
      //	print_shorts("ch:",dl_ch128);
      //	print_shorts("pack:",rxdataF_comp128);
            
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128_0[1],rxdataF128[1]);
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128_0[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[1]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
            
      rxdataF_comp128[1] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      //	print_shorts("rx:",rxdataF128+1);
      //	print_shorts("ch:",dl_ch128+1);
      //	print_shorts("pack:",rxdataF_comp128+1);	
            
      if (pilots==0) {
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128_0[2],rxdataF128[2]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128_0[2],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[2]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
                
	rxdataF_comp128[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	//	print_shorts("rx:",rxdataF128+2);
	//	print_shorts("ch:",dl_ch128+2);
	//      	print_shorts("pack:",rxdataF_comp128+2);
                
	dl_ch128_0+=3;
	dl_ch128_1+=3;
	dl_ch_mag128+=3;
	dl_ch_mag128b+=3;
	rxdataF128+=3;
	rxdataF_comp128+=3;
      }
      else {
	dl_ch128_0+=2;
	dl_ch128_1+=2;
	dl_ch_mag128+=2;
	dl_ch_mag128b+=2;
	rxdataF128+=2;
	rxdataF_comp128+=2;
      }
    }
        
    Nre = (pilots==0) ? 12 : 8;
        
    precoded_signal_strength += ((signal_energy_nodc(&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*Nre],
						     (nb_rb*Nre))*rx_power_correction) - (phy_measurements->n0_power[aarx]));
  } // rx_antennas
    
  phy_measurements->precoded_cqi_dB[eNB_id][0] = dB_fixed2(precoded_signal_strength,phy_measurements->n0_power_tot);
	
  //printf("eNB_id %d, symbol %d: precoded CQI %d dB\n",eNB_id,symbol,
  //	 phy_measurements->precoded_cqi_dB[eNB_id][0]);
    
  _mm_empty();
  _m_empty();  
}     

void dlsch_dual_stream_correlation(LTE_DL_FRAME_PARMS *frame_parms,
                                   unsigned char symbol,
                                   unsigned short nb_rb,
                                   int **dl_ch_estimates_ext,
                                   int **dl_ch_estimates_ext_i,
                                   int **dl_ch_rho_ext,
                                   unsigned char output_shift) {
    
  unsigned short rb;
  __m128i *dl_ch128,*dl_ch128i,*dl_ch_rho128;
  unsigned char aarx,symbol_mod,pilots=0;
    
  //    printf("dlsch_dual_stream_correlation: symbol %d\n",symbol);

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
    
  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp))) {
    pilots=1;
  }
    
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
        
    dl_ch128          = (__m128i *)&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch128i         = (__m128i *)&dl_ch_estimates_ext_i[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch_rho128      = (__m128i *)&dl_ch_rho_ext[aarx][symbol*frame_parms->N_RB_DL*12];
        
        
    for (rb=0;rb<nb_rb;rb++) {
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128i[0]);
      //	print_ints("re",&mmtmpD0);
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpD1);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128i[0]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      //	print_ints("re(shift)",&mmtmpD0);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      //	print_ints("im(shift)",&mmtmpD1);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      //       	print_ints("c0",&mmtmpD2);
      //	print_ints("c1",&mmtmpD3);
      dl_ch_rho128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);      
            
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128[1],dl_ch128i[1]);
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128i[1]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      dl_ch_rho128[1] =_mm_packs_epi32(mmtmpD2,mmtmpD3);
            
      if (pilots==0) {  
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128i[2]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128i[2]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	dl_ch_rho128[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
                
	dl_ch128+=3;
	dl_ch128i+=3;
	dl_ch_rho128+=3;
      }
      else {      
	dl_ch128+=2;
	dl_ch128i+=2;
	dl_ch_rho128+=2;
      }
    }	        
  }
  
  _mm_empty();
  _m_empty();
}

void dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
                         int **rxdataF_comp,
                         int **rxdataF_comp_i,
                         int **rho,
                         int **rho_i,
                         int **dl_ch_mag,
                         int **dl_ch_magb,
                         int **dl_ch_mag_i,
                         int **dl_ch_magb_i,
                         unsigned char symbol,
                         unsigned short nb_rb,
                         unsigned char dual_stream_UE) {
    
  unsigned char aatx;
  int i;
  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1,*rxdataF_comp128_i0,*rxdataF_comp128_i1,*dl_ch_mag128_0,*dl_ch_mag128_1,*dl_ch_mag128_0b,*dl_ch_mag128_1b,*rho128_0,*rho128_1,*rho128_i0,*rho128_i1,*dl_ch_mag128_i0,*dl_ch_mag128_i1,*dl_ch_mag128_i0b,*dl_ch_mag128_i1b;

  if (frame_parms->nb_antennas_rx>1) {
      
    for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++) {

      rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
      rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_0      = (__m128i *)&dl_ch_mag[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_1      = (__m128i *)&dl_ch_mag[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_0b     = (__m128i *)&dl_ch_magb[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_1b     = (__m128i *)&dl_ch_magb[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  

      // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
      for (i=0;i<nb_rb*3;i++) {
	rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
	dl_ch_mag128_0[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0[i],1),_mm_srai_epi16(dl_ch_mag128_1[i],1));
	dl_ch_mag128_0b[i]   = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0b[i],1),_mm_srai_epi16(dl_ch_mag128_1b[i],1));
      }
    }

    if (rho) {
      rho128_0 = (__m128i *) &rho[0][symbol*frame_parms->N_RB_DL*12];
      rho128_1 = (__m128i *) &rho[1][symbol*frame_parms->N_RB_DL*12];
      for (i=0;i<nb_rb*3;i++) {
	rho128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rho128_0[i],1),_mm_srai_epi16(rho128_1[i],1));
      }
    }


    if (dual_stream_UE == 1) {
      rho128_i0 = (__m128i *) &rho_i[0][symbol*frame_parms->N_RB_DL*12];
      rho128_i1 = (__m128i *) &rho_i[1][symbol*frame_parms->N_RB_DL*12];
      rxdataF_comp128_i0   = (__m128i *)&rxdataF_comp_i[0][symbol*frame_parms->N_RB_DL*12];  
      rxdataF_comp128_i1   = (__m128i *)&rxdataF_comp_i[1][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_i0      = (__m128i *)&dl_ch_mag_i[0][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_i1      = (__m128i *)&dl_ch_mag_i[1][symbol*frame_parms->N_RB_DL*12]; 
      dl_ch_mag128_i0b     = (__m128i *)&dl_ch_magb_i[0][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_i1b     = (__m128i *)&dl_ch_magb_i[1][symbol*frame_parms->N_RB_DL*12];
      for (i=0;i<nb_rb*3;i++) {
	rxdataF_comp128_i0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_i0[i],1),_mm_srai_epi16(rxdataF_comp128_i1[i],1));
	rho128_i0[i]           = _mm_adds_epi16(_mm_srai_epi16(rho128_i0[i],1),_mm_srai_epi16(rho128_i1[i],1));
            
	dl_ch_mag128_i0[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_i0[i],1),_mm_srai_epi16(dl_ch_mag128_i1[i],1));
	dl_ch_mag128_i0b[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_i0b[i],1),_mm_srai_epi16(dl_ch_mag128_i1b[i],1));
      }
    }
  }
  _mm_empty();
  _m_empty();
}

void dlsch_scale_channel(int **dl_ch_estimates_ext,
                         LTE_DL_FRAME_PARMS *frame_parms,
                         LTE_UE_DLSCH_t **dlsch_ue,
                         u8 symbol,
                         unsigned short nb_rb){

    short rb, ch_amp;
    unsigned char aatx,aarx,pilots=0,symbol_mod;
    __m128i *dl_ch128, ch_amp128;    
    
    symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

    if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
        pilots=1;
    
    // Determine scaling amplitude based the symbol
    ch_amp = ((pilots) ? (dlsch_ue[0]->sqrt_rho_b) : (dlsch_ue[0]->sqrt_rho_a));
    
    //    msg("Scaling PDSCH Chest in OFDM symbol %d by %d\n",symbol_mod,ch_amp);

    ch_amp128 = _mm_set1_epi16(ch_amp); // Q3.13

        for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++) {
            for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

                dl_ch128=(__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
                
                for (rb=0;rb<nb_rb;rb++) {
                    dl_ch128[0] = _mm_mulhi_epi16(dl_ch128[0],ch_amp128);
                    dl_ch128[0] = _mm_slli_epi16(dl_ch128[0],3);

                    dl_ch128[1] = _mm_mulhi_epi16(dl_ch128[1],ch_amp128);
                    dl_ch128[1] = _mm_slli_epi16(dl_ch128[1],3);

                    if (pilots) {
                        dl_ch128+=2;
                    } else {
                        dl_ch128[2] = _mm_mulhi_epi16(dl_ch128[2],ch_amp128);
                        dl_ch128[2] = _mm_slli_epi16(dl_ch128[2],3);
                        dl_ch128+=3;
                    }	
                }                
            }
        }
}

//compute average channel_level on each (TX,RX) antenna pair
void dlsch_channel_level(int **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int *avg,
			 u8 symbol,
			 unsigned short nb_rb){

  short rb;
  unsigned char aatx,aarx,nre=12,symbol_mod;
  __m128i *dl_ch128;
  
  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  for (aatx=0;aatx<frame_parms->nb_antennas_tx_eNB;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      //clear average level
      avg128D = _mm_xor_si128(avg128D,avg128D);
      // 5 is always a symbol with no pilots for both normal and extended prefix

      dl_ch128=(__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];

      for (rb=0;rb<nb_rb;rb++) {
	//	printf("rb %d : ",rb);
	//	print_shorts("ch",&dl_ch128[0]);
	avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[0],dl_ch128[0]));
	avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[1],dl_ch128[1]));
	if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==0)) {
	  dl_ch128+=2;	
	}
	else {
	  avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[2],dl_ch128[2]));
	  dl_ch128+=3;	
	}
	/*
	  if (rb==0) {
	  print_shorts("dl_ch128",&dl_ch128[0]);
	  print_shorts("dl_ch128",&dl_ch128[1]);
	  print_shorts("dl_ch128",&dl_ch128[2]);
	  }
	*/
      }

      if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==0))
	nre=8;
      else if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==1))
	nre=10;
      else
	nre=12;

      avg[(aatx<<1)+aarx] = (((int*)&avg128D)[0] + 
			     ((int*)&avg128D)[1] + 
			     ((int*)&avg128D)[2] + 
			     ((int*)&avg128D)[3])/(nb_rb*nre);

      //            printf("Channel level : %d\n",avg[(aatx<<1)+aarx]);
    }
  _mm_empty();
  _m_empty();

}

//compute average channel_level of effective (precoded) channel
void dlsch_channel_level_prec(int **dl_ch_estimates_ext,
                              LTE_DL_FRAME_PARMS *frame_parms,
                              unsigned char *pmi_ext,
                              int *avg,
                              u8 symbol,
                              unsigned short nb_rb){

  short rb;
  unsigned char aarx,nre=12,symbol_mod;
  __m128i *dl_ch128_0,*dl_ch128_1, dl_ch128_0_tmp, dl_ch128_1_tmp;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  //clear average level
  avg128D = _mm_xor_si128(avg128D,avg128D);
  avg[0] = 0;
  avg[1] = 0;
  // 5 is always a symbol with no pilots for both normal and extended prefix

  if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==0))
    nre=8;
  else if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==1))
    nre=10;
  else
    nre=12;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    dl_ch128_0 = (__m128i *)&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch128_1 = (__m128i *)&dl_ch_estimates_ext[2+aarx][symbol*frame_parms->N_RB_DL*12];

    for (rb=0;rb<nb_rb;rb++) {

      dl_ch128_0_tmp = _mm_load_si128(&dl_ch128_0[0]);
      dl_ch128_1_tmp = _mm_load_si128(&dl_ch128_1[0]);

      prec2A_128(pmi_ext[rb],&dl_ch128_0_tmp,&dl_ch128_1_tmp);
      mmtmpD0 = _mm_madd_epi16(dl_ch128_0_tmp,dl_ch128_0_tmp);          
      avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128_0_tmp,dl_ch128_0_tmp));

      dl_ch128_0_tmp = _mm_load_si128(&dl_ch128_0[1]);
      dl_ch128_1_tmp = _mm_load_si128(&dl_ch128_1[1]);          

      prec2A_128(pmi_ext[rb],&dl_ch128_0_tmp,&dl_ch128_1_tmp);
      mmtmpD1 = _mm_madd_epi16(dl_ch128_0_tmp,dl_ch128_0_tmp);          
      avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128_0_tmp,dl_ch128_0_tmp));

      if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==0)) {
	dl_ch128_0+=2;	
	dl_ch128_1+=2;
      }
      else {
	dl_ch128_0_tmp = _mm_load_si128(&dl_ch128_0[2]);
	dl_ch128_1_tmp = _mm_load_si128(&dl_ch128_1[2]);          

	prec2A_128(pmi_ext[rb],&dl_ch128_0_tmp,&dl_ch128_1_tmp);
	mmtmpD2 = _mm_madd_epi16(dl_ch128_0_tmp,dl_ch128_0_tmp);
	avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128_0_tmp,dl_ch128_0_tmp));

	dl_ch128_0+=3;	
	dl_ch128_1+=3;
      }          
    }
    avg[aarx] = (((int*)&avg128D)[0])/(nb_rb*nre) + 
      (((int*)&avg128D)[1])/(nb_rb*nre) + 
      (((int*)&avg128D)[2])/(nb_rb*nre) + 
      (((int*)&avg128D)[3])/(nb_rb*nre);
  }
  // choose maximum of the 2 effective channels
  avg[0] = cmax(avg[0],avg[1]);

  _mm_empty();
  _m_empty();
}
  

void dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
                    int **rxdataF_comp,
                    int **dl_ch_mag,
                    int **dl_ch_magb,
                    unsigned char symbol,
                    unsigned short nb_rb) {


  short *rxF0,*rxF1;
  __m128i *ch_mag0,*ch_mag1,*ch_mag0b,*ch_mag1b;
  unsigned char rb,re;
  int jj = (symbol*frame_parms->N_RB_DL*12);
  u8 symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  u8 pilots = ((symbol_mod==0)||(symbol_mod==(4-frame_parms->Ncp))) ? 1 : 0;


  //    printf("Doing alamouti!\n");
  rxF0     = (short*)&rxdataF_comp[0][jj];  //tx antenna 0  h0*y
  rxF1     = (short*)&rxdataF_comp[2][jj];  //tx antenna 1  h1*y
  ch_mag0 = (__m128i *)&dl_ch_mag[0][jj];
  ch_mag1 = (__m128i *)&dl_ch_mag[2][jj];
  ch_mag0b = (__m128i *)&dl_ch_magb[0][jj];
  ch_mag1b = (__m128i *)&dl_ch_magb[2][jj];
  
  for (rb=0;rb<nb_rb;rb++) {

    for (re=0;re<((pilots==0)?12:8);re+=2) {

      // Alamouti RX combining
      
      rxF0[0] = rxF0[0] + rxF1[2];
      rxF0[1] = rxF0[1] - rxF1[3];

      rxF0[2] = rxF0[2] - rxF1[0];
      rxF0[3] = rxF0[3] + rxF1[1];
 
      rxF0+=4;
      rxF1+=4;

    }

    // compute levels for 16QAM or 64 QAM llr unit
    ch_mag0[0] = _mm_adds_epi16(ch_mag0[0],ch_mag1[0]);
    ch_mag0[1] = _mm_adds_epi16(ch_mag0[1],ch_mag1[1]);

    ch_mag0b[0] = _mm_adds_epi16(ch_mag0b[0],ch_mag1b[0]);
    ch_mag0b[1] = _mm_adds_epi16(ch_mag0b[1],ch_mag1b[1]);

    if (pilots==0) {
      ch_mag0[2] = _mm_adds_epi16(ch_mag0[2],ch_mag1[2]);
      ch_mag0b[2] = _mm_adds_epi16(ch_mag0b[2],ch_mag1b[2]);

      ch_mag0+=3;
      ch_mag1+=3;
      ch_mag0b+=3;
      ch_mag1b+=3;
    }
    else {
      ch_mag0+=2;
      ch_mag1+=2;
      ch_mag0b+=2;
      ch_mag1b+=2;
    }
  }

  _mm_empty();
  _m_empty();  
}

void dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
                  int **rxdataF_comp,
                  int **dl_ch_mag,
                  int **dl_ch_magb,
                  unsigned char symbol,
                  unsigned short nb_rb) {

  unsigned char rb,re;
  int jj=1+(symbol*frame_parms->N_RB_DL*12);
    
  //  printf("Doing antcyc rx\n");
  for (rb=0;rb<nb_rb;rb++) {
    for (re=0;re<12;re+=2) {
      rxdataF_comp[0][jj] = rxdataF_comp[2][jj];  //copy odd carriers from tx antenna 1
      dl_ch_mag[0][jj]    = dl_ch_mag[2][jj];
      dl_ch_magb[0][jj]   = dl_ch_magb[2][jj];
      jj+=2;
    }
  }  
}

//==============================================================================================
// Extraction functions
//==============================================================================================

unsigned short dlsch_extract_rbs_single(int **rxdataF,
                                        int **dl_ch_estimates,
                                        int **rxdataF_ext,
                                        int **dl_ch_estimates_ext,
                                        unsigned short pmi,
                                        unsigned char *pmi_ext,
                                        unsigned int *rb_alloc,
                                        unsigned char symbol,
                                        unsigned char subframe,
                                        LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=0;
  unsigned char rb_alloc_ind;
  unsigned char i,aarx,l,nsymb,skip_half=0,sss_symb,pss_symb=0;
  int *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;



  unsigned char symbol_mod,pilots=0,j=0,poffset=0;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  pilots = ((symbol_mod==0)||(symbol_mod==(4-frame_parms->Ncp))) ? 1 : 0;
  l=symbol;
  nsymb = (frame_parms->Ncp==0) ? 14:12;

  if (frame_parms->frame_type == 1) {  // TDD
    sss_symb = nsymb-1;
    pss_symb = 2;
  }
  else {
    sss_symb = (nsymb>>1)-2;
    pss_symb = (nsymb>>1)-1;
  }
  
  if (symbol_mod==(4-frame_parms->Ncp))
    poffset=3;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];

    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
    
    if ((frame_parms->N_RB_DL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	  //dl_ch0++; 
	}
	
	if (rb_alloc_ind==1) {
	  *pmi_ext = (pmi>>((rb>>2)<<1))&3;
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  /*	  
		  printf("rb %d\n",rb);
		  for (i=0;i<12;i++)
		  printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
		  printf("\n");
	  */
	  for (i=0;i<12;i++) {
	    rxF_ext[i]=rxF[i<<1];
	    //	      printf("%d : (%d,%d)\n",(rxF+(2*i)-&rxdataF[(aatx<<1)+aarx][( (symbol*(frame_parms->ofdm_symbol_size)))*2])/2,
	    //     ((short*)&rxF[i<<1])[0],((short*)&rxF[i<<1])[0]);
	  }
	  nb_rb++;
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
	}
	dl_ch0+=12;
	rxF+=24;

      }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {
	//	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);
	skip_half=0;
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;

	if (rb_alloc_ind==1)
	  nb_rb++;

	// PBCH
	if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4))) {
	  rb_alloc_ind = 0;
	}
	//PBCH subframe 0, symbols nsymb>>1 ... nsymb>>1 + 3
	if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=1;
	else if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=2;
	
	//SSS

	if (((subframe==0)||(subframe==5)) && 
	    (rb>((frame_parms->N_RB_DL>>1)-3)) && 
	    (rb<((frame_parms->N_RB_DL>>1)+3)) && 
	    (l==sss_symb) ) {
	  rb_alloc_ind = 0;
	}
	//SSS 
	if (((subframe==0)||(subframe==5)) && 
	    (rb==((frame_parms->N_RB_DL>>1)-3)) && 
	    (l==sss_symb))
	  skip_half=1;
	else if (((subframe==0)||(subframe==5)) && 
		 (rb==((frame_parms->N_RB_DL>>1)+3)) && 
		 (l==sss_symb))
	  skip_half=2;

	//PSS in subframe 0/5 if FDD
	if (frame_parms->frame_type == 0) {  //FDD
	  if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
	
	if ((frame_parms->frame_type == 1) &&
	    (subframe==6)){  //TDD Subframe 6
	  if ((rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  if ((rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if ((rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}

	if (rb_alloc_ind==1) {
	  /*	  	  
			  printf("rb %d\n",rb);
			  for (i=0;i<12;i++)
			  printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
			  printf("\n");
	  */
	  if (pilots==0) {
	    //	    	    printf("Extracting w/o pilots (symbol %d, rb %d, skip_half %d)\n",l,rb,skip_half);
	    if (skip_half==1) {
	      memcpy(dl_ch0_ext,dl_ch0,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=6;
	      rxF_ext+=6;
	    }
	    else if (skip_half==2) {
	      memcpy(dl_ch0_ext,dl_ch0+6,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[(i+6)<<1];
	      dl_ch0_ext+=6;
	      rxF_ext+=6;
	    }
	    else {
	      memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	      for (i=0;i<12;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=12;
	      rxF_ext+=12;
	    }
	  }
	  else {
	    //	    	    printf("Extracting with pilots (symbol %d, rb %d, skip_half %d)\n",l,rb,skip_half);
	    j=0;
	    if (skip_half==1) {
	      for (i=0;i<6;i++) {
		if (i!=((frame_parms->nushift+poffset)%6)) {
		  rxF_ext[j]=rxF[i<<1];
		  //		  		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i];
		}
	      }
	      dl_ch0_ext+=5;
	      rxF_ext+=5;
	    }
	    else if (skip_half==2) {
	      for (i=0;i<6;i++) {
		if (i!=((frame_parms->nushift+poffset)%6)) {
		  rxF_ext[j]=rxF[(i+6)<<1];
		  //		  		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i+6];
		}
	      }
	      dl_ch0_ext+=5;
	      rxF_ext+=5;
	    }
	    else {
	      for (i=0;i<12;i++) {
		if ((i!=(frame_parms->nushift+poffset)) &&
		    (i!=((frame_parms->nushift+poffset+6)%12))) {
		  rxF_ext[j]=rxF[i<<1];
		  //		  		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i];
		  
		}
	      }
	      dl_ch0_ext+=10;
	      rxF_ext+=10;
	    }
	  }
	}	    
	dl_ch0+=12;
	rxF+=24;
      } // first half loop


      // Do middle RB (around DC)
      if (rb < 32)
	rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
      else if (rb < 64)
	rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
      else if (rb < 96)
	rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
      else if (rb < 100)
	rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
      else
	rb_alloc_ind = 0;

      if (rb_alloc_ind==1)
	nb_rb++;

      // PBCH
      if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4))) {
	rb_alloc_ind = 0;
      }
      //SSS
      if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb) ) {
	rb_alloc_ind = 0;
      }
      if (frame_parms->frame_type == 0) {
	//PSS
	if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	  rb_alloc_ind = 0;
	}
      }

      if ((frame_parms->frame_type == 1) &&
	  (subframe==6)){
	//PSS
	if ((rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	  rb_alloc_ind = 0;
	}
      }
      //	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);      
      //      printf("DC rb %d (%p)\n",rb,rxF);
      if (rb_alloc_ind==1) {
	if (pilots==0) {
	  for (i=0;i<6;i++) {
	    dl_ch0_ext[i]=dl_ch0[i];
	    rxF_ext[i]=rxF[i<<1];
	  }
	  rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	  for (;i<12;i++) {
	    dl_ch0_ext[i]=dl_ch0[i];
	    rxF_ext[i]=rxF[(1+i-6)<<1];
	  }
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
	}
	else { // pilots==1
	  j=0;
	  for (i=0;i<6;i++) {
	    if (i!=((frame_parms->nushift+poffset)%6)) {
	      dl_ch0_ext[j]=dl_ch0[i];
	      rxF_ext[j++]=rxF[i<<1];
	      //	           	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	    }
	  }
	  rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	  for (;i<12;i++) {
	    if (i!=((frame_parms->nushift+6+poffset)%12)) {
	      dl_ch0_ext[j]=dl_ch0[i];
	      rxF_ext[j++]=rxF[(1+i-6)<<1];
	      //	            	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	    }
	  }
	  dl_ch0_ext+=10;
	  rxF_ext+=10;
	} // symbol_mod==0
      } // rballoc==1
      else {
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
      }
      dl_ch0+=12;
      rxF+=14;
      rb++;
      
      for (;rb<frame_parms->N_RB_DL;rb++) {
	//	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);	
	//	printf("rb %d (%p)\n",rb,rxF);
	skip_half=0;
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;

	if (rb_alloc_ind==1)
	  nb_rb++;

	// PBCH
	if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=nsymb>>1) && (l<((nsymb>>1) + 4))) {
	  rb_alloc_ind = 0;
	}
	//PBCH subframe 0, symbols nsymb>>1 ... nsymb>>1 + 3
	if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=1;
	else if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=2;
	//SSS
	if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb) ) {
	  rb_alloc_ind = 0;
	}
	//SSS 
	if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==sss_symb))
	  skip_half=1;
	else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb))
	  skip_half=2;
      
	if (frame_parms->frame_type == 0) {
	  //PSS
	  if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  //PSS 
	  if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}

	if ((frame_parms->frame_type == 1) &&
	    (subframe==6)){  //TDD Subframe 6
	  if ((rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  if ((rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if ((rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
	
	if (rb_alloc_ind==1) {
	  /*
  	    printf("rb %d\n",rb);
	    for (i=0;i<12;i++)
	    printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
	    printf("\n");
	  */
	  if (pilots==0) {
	    //	    	    printf("Extracting w/o pilots (symbol %d, rb %d, skip_half %d)\n",l,rb,skip_half);
	    if (skip_half==1) {
	      memcpy(dl_ch0_ext,dl_ch0,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=6;
	      rxF_ext+=6;

	    }
	    else if (skip_half==2) {
	      memcpy(dl_ch0_ext,dl_ch0+6,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[(i+6)<<1];
	      dl_ch0_ext+=6;
	      rxF_ext+=6;

	    }
	    else {
	      memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	      for (i=0;i<12;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=12;
	      rxF_ext+=12;
	    }
	  }
	  else {
	    //	    	    printf("Extracting with pilots (symbol %d, rb %d, skip_half %d)\n",l,rb,skip_half);
	    j=0;
	    if (skip_half==1) {
	      for (i=0;i<6;i++) {
		if (i!=((frame_parms->nushift+poffset)%6)) {
		  rxF_ext[j]=rxF[i<<1];
		  //		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i];
		}
	      }
	      dl_ch0_ext+=5;
	      rxF_ext+=5;
	    }
	    else if (skip_half==2) {
	      for (i=0;i<6;i++) {
		if (i!=((frame_parms->nushift+poffset)%6)) {
		  rxF_ext[j]=rxF[(i+6)<<1];
		  //		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i+6];
		}
	      }
	      dl_ch0_ext+=5;
	      rxF_ext+=5;
	    }
	    else {
	      for (i=0;i<12;i++) {
		if ((i!=(frame_parms->nushift+poffset)) &&
		    (i!=((frame_parms->nushift+poffset+6)%12))) {
		  rxF_ext[j]=rxF[i<<1];
		  //		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i];
		}
	      }
	      dl_ch0_ext+=10;
	      rxF_ext+=10;
	    }
	  } // pilots=0
	}
	dl_ch0+=12;
	rxF+=24;

      }
    }
  }

  _mm_empty();
  _m_empty();

  return(nb_rb/frame_parms->nb_antennas_rx);
}

unsigned short dlsch_extract_rbs_dual(int **rxdataF,
                                      int **dl_ch_estimates,
                                      int **rxdataF_ext,
                                      int **dl_ch_estimates_ext,
                                      unsigned short pmi,
                                      unsigned char *pmi_ext,
                                      unsigned int *rb_alloc,
                                      unsigned char symbol,
                                      unsigned char subframe,
                                      LTE_DL_FRAME_PARMS *frame_parms) {
    

  unsigned short rb,nb_rb=0;
  unsigned char rb_alloc_ind,skip_half=0,sss_symb,pss_symb=0,nsymb,l;
  unsigned char i,aarx;
  int *dl_ch0,*dl_ch0_ext,*dl_ch1,*dl_ch1_ext,*rxF,*rxF_ext;
  unsigned char symbol_mod,pilots=0,j=0;
  unsigned char *pmi_loc;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  //  printf("extract_rbs: symbol_mod %d\n",symbol_mod);

  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
    pilots=1;
  nsymb = (frame_parms->Ncp==0) ? 14:12;
  l=symbol;

  if (frame_parms->frame_type == 1) {  // TDD
    sss_symb = nsymb-1;
    pss_symb = 2;
  }
  else {
    sss_symb = (nsymb>>1)-2;
    pss_symb = (nsymb>>1)-1;
  }

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    dl_ch1     = &dl_ch_estimates[2+aarx][5+(symbol*(frame_parms->ofdm_symbol_size))];
    dl_ch1_ext = &dl_ch_estimates_ext[2+aarx][symbol*(frame_parms->N_RB_DL*12)];
    pmi_loc = pmi_ext;

    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];


    if ((frame_parms->N_RB_DL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	skip_half=0;
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	  //dl_ch0++;
	  //dl_ch1++;
	}

	// PBCH
	if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4))) {
	  rb_alloc_ind = 0;
	}
	//PBCH subframe 0, symbols nsymb>>1 ... nsymb>>1 + 3
	if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=1;
	else if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=2;
	
	//SSS

	if (((subframe==0)||(subframe==5)) && 
	    (rb>((frame_parms->N_RB_DL>>1)-3)) && 
	    (rb<((frame_parms->N_RB_DL>>1)+3)) && 
	    (l==sss_symb) ) {
	  rb_alloc_ind = 0;
	}
	//SSS 
	if (((subframe==0)||(subframe==5)) && 
	    (rb==((frame_parms->N_RB_DL>>1)-3)) && 
	    (l==sss_symb))
	  skip_half=1;
	else if (((subframe==0)||(subframe==5)) && 
		 (rb==((frame_parms->N_RB_DL>>1)+3)) && 
		 (l==sss_symb))
	  skip_half=2;

	//PSS in subframe 0/5 if FDD
	if (frame_parms->frame_type == 0) {  //FDD
	  if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
	
	if ((frame_parms->frame_type == 1) &&
	    (subframe==6)){  //TDD Subframe 6
	  if ((rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  if ((rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if ((rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
	
	if (rb_alloc_ind==1) {


	  *pmi_loc = (pmi>>((rb>>2)<<1))&3;
	  //	  printf("rb %d: sb %d : pmi %d\n",rb,rb>>2,*pmi_loc);

	  pmi_loc++;


	  if (pilots == 0) {

	    memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	    memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	    /*
	      printf("rb %d\n",rb);
	      for (i=0;i<12;i++)
	      printf("(%d %d)",((short *)dl_ch)[i<<1],((short*)dl_ch)[1+(i<<1)]);
	      printf("\n");*/
	    
	    for (i=0;i<12;i++) {
	      rxF_ext[i]=rxF[i<<1];
	      //	      printf("%d : (%d,%d)\n",(rxF+(2*i)-&rxdataF[(aatx<<1)+aarx][( (symbol*(frame_parms->ofdm_symbol_size)))*2])/2,
	      //     ((short*)&rxF[i<<1])[0],((short*)&rxF[i<<1])[0]);
	    }
	    nb_rb++;
	    dl_ch0_ext+=12;
	    dl_ch1_ext+=12;
	    rxF_ext+=12;
	  }
	  else {
	    j=0;
	    for (i=0;i<12;i++) {
	      if ((i!=frame_parms->nushift) &&
		  (i!=frame_parms->nushift+3) &&
		  (i!=frame_parms->nushift+6) &&
		  (i!=((frame_parms->nushift+9)%12))) {
		rxF_ext[j]=rxF[i<<1];
		//	      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		dl_ch0_ext[j]=dl_ch0[i];
		dl_ch1_ext[j++]=dl_ch1[i];
	      }
	    }
	    nb_rb++;
	    dl_ch0_ext+=8;
	    dl_ch1_ext+=8;
	    rxF_ext+=8;
	  } // pilots==1
	}
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;

      }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {
	skip_half=0;
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;

	if (rb_alloc_ind == 1)
	  nb_rb++;
	// PBCH
	if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4))) {
	  rb_alloc_ind = 0;
	}
	//PBCH subframe 0, symbols nsymb>>1 ... nsymb>>1 + 3
	if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=1;
	else if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=2;
	
	//SSS

	if (((subframe==0)||(subframe==5)) && 
	    (rb>((frame_parms->N_RB_DL>>1)-3)) && 
	    (rb<((frame_parms->N_RB_DL>>1)+3)) && 
	    (l==sss_symb) ) {
	  rb_alloc_ind = 0;
	}
	//SSS 
	if (((subframe==0)||(subframe==5)) && 
	    (rb==((frame_parms->N_RB_DL>>1)-3)) && 
	    (l==sss_symb))
	  skip_half=1;
	else if (((subframe==0)||(subframe==5)) && 
		 (rb==((frame_parms->N_RB_DL>>1)+3)) && 
		 (l==sss_symb))
	  skip_half=2;
	
	//PSS in subframe 0/5 if FDD
	if (frame_parms->frame_type == 0) {  //FDD
	  if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
	
	if ((frame_parms->frame_type == 1) &&
	    (subframe==6)){  //TDD Subframe 6
	  if ((rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  if ((rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if ((rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
	//	printf("rb %d: alloc %d skip_half %d (rxF %p, rxF_ext %p)\n",rb,rb_alloc_ind,skip_half,rxF,rxF_ext);

	if (rb_alloc_ind==1) {

	  *pmi_loc = (pmi>>((rb>>2)<<1))&3;
	  //	  printf("symbol_mod %d (pilots %d) rb %d, sb %d, pmi %d (pmi_loc %p,rxF %p, ch00 %p, ch01 %p, rxF_ext %p dl_ch0_ext %p dl_ch1_ext %p)\n",symbol_mod,pilots,rb,rb>>2,*pmi_loc,pmi_loc,rxF,dl_ch0, dl_ch1, rxF_ext,dl_ch0_ext,dl_ch1_ext);
	  
	  pmi_loc++;
	  if (pilots==0) {
	    if (skip_half==1) {
	      memcpy(dl_ch0_ext,dl_ch0,6*sizeof(int));
	      memcpy(dl_ch1_ext,dl_ch1,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=6;
	      dl_ch1_ext+=6;
	      rxF_ext+=6;
	    }
	    else if (skip_half==2) {
	      memcpy(dl_ch0_ext,dl_ch0+6,6*sizeof(int));
	      memcpy(dl_ch1_ext,dl_ch1+6,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[(i+6)<<1];
	      dl_ch0_ext+=6;
	      dl_ch1_ext+=6;
	      rxF_ext+=6;
	    }
	    else {
	      memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	      memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	      for (i=0;i<12;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=12;
	      dl_ch1_ext+=12;
	      rxF_ext+=12;
	    }
	  }
	  else { // pilots=1
	    j=0;
	    if (skip_half==1) {
	      for (i=0;i<6;i++) {
 		if ((i!=frame_parms->nushift) &&
		    (i!=((frame_parms->nushift+3)%6))) {
		  
		  rxF_ext[j]=rxF[i<<1];
		  //	  printf("(pilots,skip1)extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j]=dl_ch0[i];
		  dl_ch1_ext[j++]=dl_ch1[i];
		}
	      }
	      dl_ch0_ext+=4;
	      dl_ch1_ext+=4;
	      rxF_ext+=4;
	    }
	    else if (skip_half==2) {
	      for (i=0;i<6;i++) {
 		if ((i!=frame_parms->nushift) &&
		    (i!=((frame_parms->nushift+3)%6))) {
		  rxF_ext[j]=rxF[(i+6)<<1];
		  //	      printf("(pilots,skip2)extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j]=dl_ch0[i+6];
		  dl_ch1_ext[j++]=dl_ch1[i+6];
		}
		
		dl_ch0_ext+=4;
		dl_ch1_ext+=4;
		rxF_ext+=4;
	      }
	    }
	    else {
	      for (i=0;i<12;i++) {
		if ((i!=frame_parms->nushift) &&
		    (i!=frame_parms->nushift+3) &&
		    (i!=frame_parms->nushift+6) &&
		    (i!=((frame_parms->nushift+9)%12))) {
		  rxF_ext[j]=rxF[i<<1];
		  //	  printf("(pilots)extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  
		  dl_ch0_ext[j]  =dl_ch0[i];
		  dl_ch1_ext[j++]=dl_ch1[i];
		  
		  //			      printf("extract rb %d, re %d => ch0 (%d,%d) ch1 (%d,%d)\n",rb,i,
		  //				     *(short *)&dl_ch0_ext[j-1],*(1+(short*)&dl_ch0_ext[j-1]),
		  //				     *(short *)&dl_ch1_ext[j-1],*(1+(short*)&dl_ch1_ext[j-1]));
		}
	      }
	      dl_ch0_ext+=8;
	      dl_ch1_ext+=8;
	      rxF_ext+=8;		
	    }
	  }
	}
      
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;
      }
    
    
      // Do middle RB (around DC)
      if (rb < 32)
	rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
      else if (rb < 64)
	rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
      else if (rb < 96)
	rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
      else if (rb < 100)
	rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
      else
	rb_alloc_ind = 0;
      
      if (rb_alloc_ind==1)
	nb_rb++;

      
      // PBCH
      if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4))) {
	rb_alloc_ind = 0;
      }
      //SSS
      if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb) ) {
	rb_alloc_ind = 0;
      }
      if (frame_parms->frame_type == 0) {
	//PSS
	if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	  rb_alloc_ind = 0;
	}
      }
      
      if ((frame_parms->frame_type == 1) &&
	  (subframe==6)){
	//PSS
	if ((rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	  rb_alloc_ind = 0;
	}
      }
      
      
      if (rb_alloc_ind==1) {
	
	*pmi_loc = (pmi>>((rb>>2)<<1))&3;
	//	printf("rb %d, sb %d, pmi %d (pmi_loc %p,rxF %p, ch00 %p, ch01 %p, rxF_ext %p dl_ch0_ext %p dl_ch1_ext %p)\n",rb,rb>>2,*pmi_loc,pmi_loc,rxF,dl_ch0, dl_ch1, rxF_ext,dl_ch0_ext,dl_ch1_ext);
	pmi_loc++;
	
	if (pilots==0) {
	  for (i=0;i<6;i++) {
	    dl_ch0_ext[i]=dl_ch0[i];
	    dl_ch1_ext[i]=dl_ch1[i];
	    rxF_ext[i]=rxF[i<<1];
	  }
	  rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	  for (;i<12;i++) {
	    dl_ch0_ext[i]=dl_ch0[i];
	    dl_ch1_ext[i]=dl_ch1[i];
	    rxF_ext[i]=rxF[(1+i-6)<<1];
	  }
	  dl_ch0_ext+=12;
	  dl_ch1_ext+=12;
	  rxF_ext+=12;	
	}
	else {  // pilots==1
	  j=0;
	  for (i=0;i<6;i++) {
	    if ((i!=frame_parms->nushift) &&
		(i!=((frame_parms->nushift+3)%6))){
	      dl_ch0_ext[j]=dl_ch0[i];
	      dl_ch1_ext[j]=dl_ch1[i];
	      rxF_ext[j++]=rxF[i<<1];
	      //     	      printf("(pilots center)extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	      //	      printf("extract rb %d, re %d => ch0 (%d,%d) ch1 (%d,%d)\n",rb,i,
	      //		     *(short *)&dl_ch0_ext[j-1],*(1+(short*)&dl_ch0_ext[j-1]),
	      //		     *(short *)&dl_ch1_ext[j-1],*(1+(short*)&dl_ch1_ext[j-1]));
	    }
	  }
	  rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	  for (;i<12;i++) {
	    if ((i!=((frame_parms->nushift+6)%12)) &&
		(i!=((frame_parms->nushift+9)%12))){
	      dl_ch0_ext[j]=dl_ch0[i];
	      dl_ch1_ext[j]=dl_ch1[i];
	      rxF_ext[j++]=rxF[(1+i-6)<<1];
	      //     	      printf("(pilots center )extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	    }
	  }
	  dl_ch0_ext+=8;
	  dl_ch1_ext+=8;
	  rxF_ext+=8; 
	}
	
	
      }
      else {
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
      }
      dl_ch0+=12;
      dl_ch1+=12;
      rxF+=14;
      rb++;
      
      for (;rb<frame_parms->N_RB_DL;rb++) {
	skip_half=0;
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	if (rb_alloc_ind==1)
	  nb_rb++;
	
	// PBCH
	if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4))) {
	  rb_alloc_ind = 0;
	}
	//PBCH subframe 0, symbols nsymb>>1 ... nsymb>>1 + 3
	if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=1;
	else if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l>=(nsymb>>1)) && (l<((nsymb>>1) + 4)))
	  skip_half=2;

	//SSS
	if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb) ) {
	  rb_alloc_ind = 0;
	}
	//SSS 
	if (((subframe==0)||(subframe==5)) && 
	    (rb==((frame_parms->N_RB_DL>>1)-3)) && 
	    (l==sss_symb))
	  skip_half=1;
	else if (((subframe==0)||(subframe==5)) && 
		 (rb==((frame_parms->N_RB_DL>>1)+3)) && 
		 (l==sss_symb))
	  skip_half=2;

	if (frame_parms->frame_type == 0) {
	  //PSS
	  if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;

	  }
	  if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
	
	if ((frame_parms->frame_type == 1) &&
	    (subframe==6)){
	  //PSS
	  if ((rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  if ((rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if ((rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
      	
	if (rb_alloc_ind==1) {
	  
	  *pmi_loc = (pmi>>((rb>>2)<<1))&3;
	  //  	  printf("rb %d, sb %d, pmi %d (pmi_loc %p,rxF %p, ch00 %p, ch01 %p, rxF_ext %p dl_ch0_ext %p dl_ch1_ext %p)\n",rb,rb>>2,*pmi_loc,pmi_loc,rxF,dl_ch0, dl_ch1, rxF_ext,dl_ch0_ext,dl_ch1_ext);
	  
	  pmi_loc++;
	  
	  if (pilots==0) {
	    if (skip_half==1) {
	      memcpy(dl_ch0_ext,dl_ch0,6*sizeof(int));
	      memcpy(dl_ch1_ext,dl_ch1,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=6;
	      dl_ch1_ext+=6;
	      rxF_ext+=6;
	      
	    }
	    else if (skip_half==2) {
	      memcpy(dl_ch0_ext,dl_ch0+6,6*sizeof(int));
	      memcpy(dl_ch1_ext,dl_ch1+6,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[(i+6)<<1];
	      dl_ch0_ext+=6;
	      dl_ch1_ext+=6;
	      rxF_ext+=6;
	      
	    }
	    else {
	      memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	      memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	      for (i=0;i<12;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=12;
	      dl_ch1_ext+=12;
	      rxF_ext+=12;
	    }
	  }
	  else {
	    j=0;
	    if (skip_half==1) {
	      for (i=0;i<6;i++) {
		if ((i!=(frame_parms->nushift)) &&
		    (i!=((frame_parms->nushift+3)%6))){
		  rxF_ext[j]=rxF[i<<1];
		  //  		  printf("(skip1,pilots)extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j]=dl_ch0[i];
		  dl_ch1_ext[j++]=dl_ch1[i];
		}
	      }
	      dl_ch0_ext+=4;
	      dl_ch1_ext+=4;
	      rxF_ext+=4;
	    }
	    else if (skip_half==2) {
	      for (i=0;i<6;i++) {
		if ((i!=(frame_parms->nushift))  &&
		    (i!=((frame_parms->nushift+3)%6))){
		  rxF_ext[j]=rxF[(i+6)<<1];
		  //  		  printf("(skip2,pilots)extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j]=dl_ch0[i+6];
		  dl_ch1_ext[j++]=dl_ch1[i+6];
		}
	      }
	      dl_ch0_ext+=4;
	      dl_ch1_ext+=4;
	      rxF_ext+=4;
	    }
	    else {
	      for (i=0;i<12;i++) {
		if ((i!=frame_parms->nushift) &&
		    (i!=frame_parms->nushift+3) &&
		    (i!=frame_parms->nushift+6) &&
		    (i!=((frame_parms->nushift+9)%12))) {
		  rxF_ext[j]=rxF[i<<1];
		  //	printf("(pilots)extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j]=dl_ch0[i];
		  dl_ch1_ext[j++]=dl_ch1[i];
		  //			      printf("extract rb %d, re %d => ch0 (%d,%d) ch1 (%d,%d)\n",rb,i,
		  //				     *(short *)&dl_ch0_ext[j-1],*(1+(short*)&dl_ch0_ext[j-1]),
		  //				     *(short *)&dl_ch1_ext[j-1],*(1+(short*)&dl_ch1_ext[j-1]));
		}
	      }
	      dl_ch0_ext+=8;
	      dl_ch1_ext+=8;
	      rxF_ext+=8;
	      
	    }
	  }
	}
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;
      }
    }
  }
  
  
  _mm_empty();
  _m_empty();
  
  return(nb_rb/frame_parms->nb_antennas_rx);
}

//==============================================================================================
// Auxiliary functions
//==============================================================================================

#ifdef USER_MODE

void dump_dlsch2(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u16 coded_bits_per_codeword) {

  unsigned int nsymb = (phy_vars_ue->lte_frame_parms[eNB_id]->Ncp == 0) ? 14 : 12;
  char fname[32],vname[32];
    
  sprintf(fname,"dlsch%d_rxF_ext0.m",eNB_id);
  sprintf(vname,"dl%d_rxF_ext0",eNB_id);
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->rxdataF_ext[0],300*nsymb,1,1);
  sprintf(fname,"dlsch%d_ch_ext00.m",eNB_id);
  sprintf(vname,"dl%d_ch_ext00",eNB_id);
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch%d_ch_ext01.m","dl01_ch0_ext",lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext[1],300*nsymb,1,1);
    write_output("dlsch%d_ch_ext10.m","dl10_ch0_ext",lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext[2],300*nsymb,1,1);
    write_output("dlsch%d_ch_ext11.m","dl11_ch0_ext",lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext[3],300*nsymb,1,1);
    write_output("dlsch%d_rho.m","dl_rho",lte_ue_pdsch_vars[eNB_id]->rho[0],300*nsymb,1,1);
  */
  sprintf(fname,"dlsch%d_rxF_comp0.m",eNB_id);
  sprintf(vname,"dl%d_rxF_comp0",eNB_id);
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->rxdataF_comp[0],300*nsymb,1,1);
  sprintf(fname,"dlsch%d_rxF_llr.m",eNB_id);
  sprintf(vname,"dl%d_llr",eNB_id);
  write_output(fname,vname, phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],coded_bits_per_codeword,1,0);
  sprintf(fname,"dlsch%d_mag1.m",eNB_id);
  sprintf(vname,"dl%d_mag1",eNB_id);
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,300*nsymb,1,1);
  sprintf(fname,"dlsch%d_mag2.m",eNB_id);
  sprintf(vname,"dl%d_mag2",eNB_id);
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,300*nsymb,1,1);
}
#endif

#ifdef DEBUG_DLSCH_DEMOD

void print_bytes(char *s,__m128i *x) {

  char *tempb = (char *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7],
         tempb[8],tempb[9],tempb[10],tempb[11],tempb[12],tempb[13],tempb[14],tempb[15]
         );

}

void print_shorts(char *s,__m128i *x) {

  short *tempb = (short *)x;
  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7]);

}

void print_shorts2(char *s,__m64 *x) {

  short *tempb = (short *)x;
  printf("%s  : %d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3]);

}

void print_ints(char *s,__m128i *x) {

  int *tempb = (int *)x;
  printf("%s  : %d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3]);

}
#endif
