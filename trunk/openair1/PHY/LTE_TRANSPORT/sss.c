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

/*! \file PHY/LTE_TRANSPORT/sss.c
* \brief Top-level routines for generating and decoding the secondary synchronization signal (SSS) V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/
#include "PHY/defs.h"
#include "defs.h"
#include "PHY/extern.h"

//#define DEBUG_SSS

int generate_sss(mod_sym_t **txdataF,
		 short amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 unsigned short symbol,
		 unsigned short slot_offset) {

  u8 i,aa,Nsymb;
  short *d,k;
  u8 Nid2;
  u16 Nid1;


  Nid2 = frame_parms->Nid_cell % 3;
  Nid1 = frame_parms->Nid_cell/3;

  if (slot_offset < 3) 
    d = &d0_sss[62*(Nid2 + (Nid1*3))];
  else
    d = &d5_sss[62*(Nid2 + (Nid1*3))];

  Nsymb = (frame_parms->Ncp==0)?14:12;
  k = frame_parms->ofdm_symbol_size-3*12+5;
  for (i=0;i<62;i++) {
    //for (aa=0;aa<frame_parms->nb_antennas_tx;aa++) {
    aa=0;

      ((short*)txdataF[aa])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
			       symbol*frame_parms->ofdm_symbol_size + k)] = 
	(amp * d[i]); 
      ((short*)txdataF[aa])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
			       symbol*frame_parms->ofdm_symbol_size + k)+1] = 0;
      /*
      if (aa==0)
	printf("sss (slot %d, symbol %d): txdataF[%d] => (%d,%d)\n",slot_offset,symbol,
	       slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
	       symbol*frame_parms->ofdm_symbol_size + k,
	       (amp * d[i]),0);
      */
      //}
    k+=1;
    if (k >= frame_parms->ofdm_symbol_size) {
      k++;
      k-=frame_parms->ofdm_symbol_size;
    }
  }
  return(0);
}

int pss_ch_est(PHY_VARS_UE *phy_vars_ue,
	       s32 pss_ext[4][72],
	       s32 sss_ext[4][72])  {

  short *pss;
  short *pss_ext2,*sss_ext2,*sss_ext3,tmp_re,tmp_im,tmp_re2,tmp_im2;
  u8 aarx,i;
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_ue->lte_frame_parms;

  switch (phy_vars_ue->lte_ue_common_vars.eNb_id) {
    
  case 0:
    pss = &primary_synch0[10];
    break;
  case 1:
    pss = &primary_synch1[10];
    break;
  case 2:
    pss = &primary_synch2[10];
    break;
  default:
    pss = &primary_synch0[10];
    break;
  }

  sss_ext3 = (short*)&sss_ext[0][5];
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    sss_ext2 = (short*)&sss_ext[aarx][5];
    pss_ext2 = (short*)&pss_ext[aarx][5];
    for (i=0;i<62;i++) {

      // This is H*(PSS) = R* \cdot PSS
      tmp_re = (s16)(((pss_ext2[i<<1] * (s32)pss[i<<1])>>15)     + ((pss_ext2[1+(i<<1)] * (s32)pss[1+(i<<1)])>>15));
      tmp_im = (s16)(((pss_ext2[i<<1] * (s32)pss[1+(i<<1)])>>15) - ((pss_ext2[1+(i<<1)] * (s32)pss[(i<<1)])>>15));
      //      printf("H*(%d,%d) : (%d,%d)\n",aarx,i,tmp_re,tmp_im);
      // This is R(SSS) \cdot H*(PSS)
      tmp_re2 = (s16)(((tmp_re * (s32)sss_ext2[i<<1])>>15)     - ((tmp_im * (s32)sss_ext2[1+(i<<1)]>>15))); 
      tmp_im2 = (s16)(((tmp_re * (s32)sss_ext2[1+(i<<1)])>>15) + ((tmp_im * (s32)sss_ext2[(i<<1)]>>15)));
      //      printf("SSSi(%d,%d) : (%d,%d)\n",aarx,i,sss_ext2[i<<1],sss_ext2[1+(i<<1)]); 
      //      printf("SSSo(%d,%d) : (%d,%d)\n",aarx,i,tmp_re2,tmp_im2);
      // MRC on RX antennas
      if (aarx==0) {
	sss_ext3[i<<1]      = tmp_re2;
	sss_ext3[1+(i<<1)]  = tmp_im2;
      }
      else {
	sss_ext3[i<<1]      += tmp_re2;
	sss_ext3[1+(i<<1)]  += tmp_im2;
      }
    }
  }
  // sss_ext now contains the compensated SSS
  return(0);
}


int pss_sss_extract(PHY_VARS_UE *phy_vars_ue,
		    s32 pss_ext[4][72],
		    s32 sss_ext[4][72]) {

    
  
  u16 rb,nb_rb=6;
  u8 i,aarx;
  s32 *pss_rxF,*pss_rxF_ext;
  s32 *sss_rxF,*sss_rxF_ext;
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_ue->lte_frame_parms;

  int rx_offset = frame_parms->ofdm_symbol_size-3*12;
  u8 pss_symb,sss_symb;

  s32 **rxdataF =  phy_vars_ue->lte_ue_common_vars.rxdataF;

  if (frame_parms->frame_type == 0) {
    pss_symb = 6-frame_parms->Ncp;
    sss_symb = pss_symb-1;
  }
  else {
    pss_symb = 2;
    sss_symb = frame_parms->symbols_per_tti-1;
  }

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    //printf("extract_rbs: symbol_mod=%d, rx_offset=%d, ch_offset=%d\n",symbol_mod,
    //   (rx_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2,
    //   LTE_CE_OFFSET+ch_offset+(symbol_mod*(frame_parms->ofdm_symbol_size)));

    pss_rxF        = &rxdataF[aarx][(rx_offset + (pss_symb*(frame_parms->ofdm_symbol_size)))*2];
    pss_rxF_ext    = &pss_ext[aarx][0];
    sss_rxF        = &rxdataF[aarx][(rx_offset + (sss_symb*(frame_parms->ofdm_symbol_size)))*2];
    sss_rxF_ext    = &sss_ext[aarx][0];

    for (rb=0; rb<nb_rb; rb++) {
      // skip DC carrier
      if (rb==3) {
	sss_rxF       = &rxdataF[aarx][(1 + (sss_symb*(frame_parms->ofdm_symbol_size)))*2];
	pss_rxF       = &rxdataF[aarx][(1 + (pss_symb*(frame_parms->ofdm_symbol_size)))*2];
      }
      for (i=0;i<12;i++) {
	pss_rxF_ext[i]=pss_rxF[i<<1];
	sss_rxF_ext[i]=sss_rxF[i<<1];
      }
      pss_rxF+=24;
      pss_rxF_ext+=12;
      sss_rxF+=24;
      sss_rxF_ext+=12;
    }

  }
  
  return(0);
}


short phase_re[7] = {16383, 25101, 30791, 32767, 30791, 25101, 16383};
short phase_im[7] = {-28378, -21063, -11208, 0, 11207, 21062, 28377};


int rx_sss(PHY_VARS_UE *phy_vars_ue,s32 *tot_metric,u8 *flip_max,u8 *phase_max) {
  
  u8 i;
  s32 pss_ext[4][72];
  s32 sss0_ext[4][72],sss5_ext[4][72];
  u8 Nid2 = phy_vars_ue->lte_ue_common_vars.eNb_id;
  u8 flip,phase;
  u16 Nid1;
  s16 *sss0,*sss5;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  s32 metric;
  s16 *d0,*d5;

  if (phy_vars_ue->lte_frame_parms.frame_type == 0) { // FDD 
#ifdef DEBUG_SSS
    if (phy_vars_ue->lte_frame_parms.Ncp == 0)
      msg("[PHY][UE%d] Doing SSS for FDD Normal Prefix\n",phy_vars_ue->Mod_id);
    else
      msg("[PHY][UE%d] Doing SSS for FDD Extended Prefix\n",phy_vars_ue->Mod_id);
#endif
    // Do FFTs for SSS/PSS
    // SSS
    slot_fep(phy_vars_ue,
	     (frame_parms->symbols_per_tti/2)-2, // second to last symbol of 
	     0,                                  // slot 0
	     phy_vars_ue->rx_offset,
	     0);
    // PSS
    slot_fep(phy_vars_ue,
	     (frame_parms->symbols_per_tti/2)-1, // last symbol of
	     0,                                  // slot 0
	     phy_vars_ue->rx_offset,
	     0);
  }
  else {   // TDD
#ifdef DEBUG_SSS
    if (phy_vars_ue->lte_frame_parms.Ncp == 0)
      msg("[PHY][UE%d] Doing SSS for TDD Normal Prefix\n",phy_vars_ue->Mod_id);
    else
      msg("[PHY][UE%d] Doing SSS for TDD Extended Prefix\n",phy_vars_ue->Mod_id);
#endif
    // SSS
    slot_fep(phy_vars_ue,
	     (frame_parms->symbols_per_tti>>1)-1,  // last symbol of 
	     1,                                    // slot 1
	     phy_vars_ue->rx_offset,
	     0);
    // PSS
    slot_fep(phy_vars_ue,
	     2,                                   // symbol 2 of
	     2,                                   // slot 2
	     phy_vars_ue->rx_offset,
	     0);
  }

  pss_sss_extract(phy_vars_ue,
		  pss_ext,
		  sss0_ext);
  /*  
  write_output("rxsig0.m","rxs0",&phy_vars_ue->lte_ue_common_vars.rxdata[0][0],phy_vars_ue->lte_frame_parms.samples_per_tti,1,1);
  write_output("rxdataF0.m","rxF0",&phy_vars_ue->lte_ue_common_vars.rxdataF[0][0],2*14*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("pss_ext0.m","pssext0",pss_ext,72,1,1);
  write_output("sss0_ext0.m","sss0ext0",sss0_ext,72,1,1);
  */

    // get conjugated channel estimate from PSS (symbol 6), H* = R* \cdot PSS
    // and do channel estimation and compensation based on PSS

  pss_ch_est(phy_vars_ue,
	     pss_ext,
	     sss0_ext);
  
  //  write_output("sss0_comp0.m","sss0comp0",sss0_ext,72,1,1);

  if (phy_vars_ue->lte_frame_parms.frame_type == 0) { // FDD 

    // SSS
    slot_fep(phy_vars_ue,
	     (frame_parms->symbols_per_tti/2)-2,
	     10,
	     phy_vars_ue->rx_offset,
	     0);
    // PSS
    slot_fep(phy_vars_ue,
	     (frame_parms->symbols_per_tti/2)-1,
	     10,
	     phy_vars_ue->rx_offset,
	     0);
  }
  else {  // TDD
    // SSS
    slot_fep(phy_vars_ue,
	     (frame_parms->symbols_per_tti>>1)-1,
	     11,
	     phy_vars_ue->rx_offset,
	     0);
    // PSS
    slot_fep(phy_vars_ue,
	     2,
	     12,
	     phy_vars_ue->rx_offset,
	     0);
  }
  
  pss_sss_extract(phy_vars_ue,
		  pss_ext,
		  sss5_ext);

  //  write_output("sss5_ext0.m","sss5ext0",sss5_ext,72,1,1);
  // get conjugated channel estimate from PSS (symbol 6), H* = R* \cdot PSS
  // and do channel estimation and compensation based on PSS
  
  pss_ch_est(phy_vars_ue,
	     pss_ext,
	     sss5_ext);
  
  
  
  // now do the SSS detection based on the precomputed sequences in PHY/LTE_TRANSPORT/sss.h
  
  *tot_metric = -99999999;
  
  
  sss0 = (short*)&sss0_ext[0][5];
  sss5 = (short*)&sss5_ext[0][5];
  for (flip=0;flip<2;flip++) {        //  d0/d5 flip in RX frame
    for (phase=0;phase<=7;phase++) {  // phase offset between PSS and SSS
      for (Nid1 = 0 ; Nid1 < 167; Nid1++) {  // possible Nid1 values
	metric = 0;
	if (flip==0) {
	  d0 = &d0_sss[62*(Nid2 + (Nid1*3))];
	  d5 = &d5_sss[62*(Nid2 + (Nid1*3))];
	}
	else {
	  d5 = &d0_sss[62*(Nid2 + (Nid1*3))];
	  d0 = &d5_sss[62*(Nid2 + (Nid1*3))];
	}
	// This is the inner product using one particular value of each unknown parameter
	for (i=0;i<62;i++) {
	  metric += (s16)(((d0[i]*((((phase_re[phase]*(s32)sss0[i<<1])>>19)-((phase_im[phase]*(s32)sss0[1+(i<<1)])>>19)))) + 
			   (d5[i]*((((phase_re[phase]*(s32)sss5[i<<1])>>19)-((phase_im[phase]*(s32)sss5[1+(i<<1)])>>19))))));
	} 
	// if the current metric is better than the last save it
	if (metric > *tot_metric) {
	  *tot_metric = metric;
	  phy_vars_ue->lte_frame_parms.Nid_cell = Nid2+(3*Nid1);
	  *phase_max = phase;
	  *flip_max=flip;
#ifdef DEBUG_SSS
	  msg("(flip,phase,Nid1) (%d,%d,%d), metric_phase %d tot_metric %d, phase_max %d, flip_max %d\n",flip,phase,Nid1,metric,*tot_metric,*phase_max,*flip_max);
#endif
	  
	}
      }
    } 
  }

  return(0);
}

