/*________________________phy_procedures_lte_MeNbMue.c___________________

Authors : Hicham Anouar, Raymond Knopp, Florian Kaltenberger,Torbjorn Sorby
Company : EURECOM
Emails  : anouar@eurecom.fr,  knopp@eurecom.fr, kaltenbe@eurecom.fr, sorby@eurecom.fr
________________________________________________________________*/


// This routine is called periodically by macphy_scheduler to analyse the set of PHY_primitives that were
// Scheduled by MAC and on PHY resources at the appropriate time

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif

#ifdef USER_MODE
#ifndef PBS_SIM 
#define DEBUG_PHY
#endif //PBS_SIM
//#define NULL_SHAPE_BF_ENABLED
#endif //USER_MODE

#define DIAG_PHY


//undef DEBUG_PHY and set debug_msg to option 1 to print only most necessary messages every 100 frames. 
//define DEBUG_PHY and set debug_msg to option 2 to print everything all frames
//use msg for something that should be always printed in any case

//#define debug_msg if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) msg
#define debug_msg msg



/*
void debug_msg(unsigned char debug_level, char* format_string) { 

  if (debug_level<=DEBUG_LEVEL)
    if (debug_level == 2)
      msg(format_string);
    else if (debug_level == 1) 
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) 
	msg(format_string);
*/


#define NS_PER_SLOT 500000

extern inline unsigned int taus(void);
extern int exit_openair;

#ifdef EMOS
//  fifo_dump_emos_UE emos_dump_UE;
  fifo_dump_emos_eNb emos_dump_eNb;
#endif

#ifdef DIAG_PHY
extern int rx_sig_fifo;
#endif

void phy_procedures_eNB_S_RX_secsys(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb) {

  // goal here is to synchronize (again) on PSS from primary eNb
  // should expect offset to be close to zero (all other than the first time)
  int sync_pos=0, sync_pos_slot;

  if (last_slot%2==1) {
    phy_procedures_eNB_S_RX(last_slot, phy_vars_eNb);
    } 
  else if (mac_xface->frame>0) { // last_slot is 2 (where we`ll find PSS from primary eNb)

  // dump one frame of data (would be from HW in real time mode), then call lte_sync_time.  Here, one frame of data would be captured from phy_vars_eNb->lte_eNb_common_vars->rxdata

#ifdef DEBUG_PHY
  msg("[PHY_PROCEDURES_LTE] Frame% d: slot(%d)\n",mac_xface->frame, last_slot);
#endif //DEBUG_PHY
  sync_pos = lte_sync_time(phy_vars_eNb->lte_eNB_common_vars.rxdata[0],
			   &phy_vars_eNb->lte_frame_parms,
			   LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*phy_vars_eNb->lte_frame_parms.samples_per_tti,
			   &phy_vars_eNb->PeNb_id);
#ifdef DEBUG_PHY
  write_output("eNb_rxsig0_1.m","eNb_rxs0_1", phy_vars_eNb->lte_eNB_common_vars.rxdata[0][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
#endif //DEBUG_PHY
  sync_pos_slot = OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*2+2) + CYCLIC_PREFIX_LENGTH + 10;
  phy_vars_eNb->rx_offset = sync_pos - sync_pos_slot;

  // try decoding PBCH (requires rewriting rx_pbch(...))

  // if success set phy_vars_eNb->is_init_sync = 1;
  }
}

void phy_precode_nullBeam_create(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNb) {

  int aa, i, eNb_id=0,PeNb_id=0;
  short re_next, im_next,re_last, im_last;
  short zero_detections = 0, ind_of_maxp =0;
  int maxp=0;

  switch (last_slot) {
  case 5:
#ifdef DEBUG_PHY
    write_output("srs_ch_est_0_5.m","srs_ce_0_5",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1);    
    write_output("srs_ch_est_1_5.m","srs_ce_1_5",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1); 
#endif //DEBUG_PHY   
    // interpolate last SRS_ch_estimates in freq (from slot 7)
    
    // then interpolate in time (from slot 5 and 7) and rearrange according to create NULL-beam
    /*
     * [h1 h2] ==> [-h2 h1]
     */
    break;
  case 7: 
#ifdef DEBUG_PHY
    write_output("srs_ch_est_0_7.m","srs_ce_0_7",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1);    
    write_output("srs_ch_est_1_7.m","srs_ce_1_7",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1); 
    write_output("srs_rxF_0_7.m","srs_F_0_7",phy_vars_eNb->lte_eNB_common_vars.rxdataF[0][0],2*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,2,1); 
    write_output("srs_rxF_1_7.m","srs_F_1_7",phy_vars_eNb->lte_eNB_common_vars.rxdataF[0][1],2*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,2,1); 
#endif //DEBUG_PHY   

    for (aa=0; aa<phy_vars_eNb->lte_frame_parms.nb_antennas_rx; aa++) {
      zero_detections = 0;
      // first half (positive freq), samples on odd
      for (i=2; i<phy_vars_eNb->lte_frame_parms.ofdm_symbol_size>>1; i+=2) {
	
	// interpolate last SRS_ch_estimates in freq (from slot 5)
	re_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][aa]))[((i-1)<<1)];
	im_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][aa]))[((i-1)<<1) + 1];

	re_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][aa]))[((i+1)<<1)];
	im_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][aa]))[((i+1)<<1) + 1];

#ifdef RANDOM_BF	
	re_last = phy_vars_eNb->const_ch[aa][0];
	im_last = phy_vars_eNb->const_ch[aa][1];
	re_next = phy_vars_eNb->const_ch[aa][0];
	im_next = phy_vars_eNb->const_ch[aa][1];
#endif //RANDOM_BF

	if (abs(re_last) >= maxp) {
	  maxp = abs(re_last);
	  ind_of_maxp = i;
	}
	if (abs(im_last) >= maxp) {
	  maxp = abs(im_last);
	  ind_of_maxp = i;
	}
	if (abs(re_next) >= maxp) {
	  maxp = abs(re_next);
	  ind_of_maxp = i;
	}
	if (abs(im_next) >= maxp) {
	  maxp = abs(im_next);
	  ind_of_maxp = i;
	}
	
	if (re_last == 0) {
	  zero_detections += 1;
	}
	if (zero_detections > 3 && i<10) {
	  //very unlikely --> thus a probable error
	  //printf("error in creating the precoder");
	  phy_vars_eNb->has_valid_precoder = 0;
	  return(-1);
	}
	
	// rearrange according to create NULL-beam
	/*
	 * [h1 h2] ==> [-h2 h1]
	 * don't forget format of dl_precoder_SeNb, repeated
	 * ifndef IFFT_FPGA --> NO_PREFIX
	 */
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2)] = (1-2*aa)*re_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 2] = (1-2*aa)*re_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 1] = (1-2*aa)*im_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 3] = (1-2*aa)*im_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2)] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 2] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 1] = (1-2*aa)*(im_last + im_next)>>1;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 3] = (1-2*aa)*(im_last + im_next)>>1;
      }

      // second half (negative freq), samples on even
      for (i=((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size>>1)+1); i<phy_vars_eNb->lte_frame_parms.ofdm_symbol_size; i+=2) {
	
	// interpolate last SRS_ch_estimates in freq (from slot 5)
	re_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((i-1)<<1)];
	im_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((i-1)<<1) + 1];

	re_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((i+1)<<1)];
	im_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((i+1)<<1) + 1];

#ifdef RANDOM_BF
	re_last = phy_vars_eNb->const_ch[aa][0];
	im_last = phy_vars_eNb->const_ch[aa][1];
	re_next = phy_vars_eNb->const_ch[aa][0];
	im_next = phy_vars_eNb->const_ch[aa][1];
#endif //RANDOM_BF

	if (abs(re_last) >= maxp) {
	  maxp = abs(re_last);
	  ind_of_maxp = i;
	}
	if (abs(im_last) >= maxp) {
	  maxp = abs(im_last);
	  ind_of_maxp = i;
	}
        if (abs(re_next) >= maxp) {
	  maxp = abs(re_next);
	  ind_of_maxp = i;
	}
        if (abs(im_next) >= maxp) {
	  maxp = abs(im_next);
	  ind_of_maxp = i;
	}
	
	// rearrange according to create NULL-beam
	/*
	 * [h1 h2] ==> [-h2 h1]
	 */
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2)] = (1-2*aa)*re_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 2] = (1-2*aa)*re_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 1] = (1-2*aa)*im_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[((i-1)<<2) + 3] = (1-2*aa)*im_last;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2)] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 2] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 1] = (1-2*aa)*(im_last + im_next)>>1;
	((short *)phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa])[(i<<2) + 3] = (1-2*aa)*(im_last + im_next)>>1;
      }

      // near DC special case for interpolation.
      // interpolate last SRS_ch_estimates in freq (from slot 5)
      // interpolate first positive and last negative frequency
      re_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size-1)<<1)];
      im_last = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size-1)<<1) + 1];
      re_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[2];
      im_next = ((short *)(phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[eNb_id][aa]))[3];
#ifdef RANDOM_BF
      re_last = phy_vars_eNb->const_ch[aa][0];
      im_last = phy_vars_eNb->const_ch[aa][1];
      re_next = phy_vars_eNb->const_ch[aa][0];
      im_next = phy_vars_eNb->const_ch[aa][1];
#endif //RANDOM_BF
      if (abs(re_last) >= maxp) {
	maxp = abs(re_last);
	ind_of_maxp = i;
      }
      if (abs(im_last) >= maxp) {
	maxp = abs(im_last);
	ind_of_maxp = i;
      }
      if (abs(re_next) >= maxp) {
	maxp = abs(re_next);
	ind_of_maxp = i;
      }
      if (abs(im_next) >= maxp) {
	maxp = abs(im_next);
	ind_of_maxp = i;
      }
	
	// rearrange according to create NULL-beam
	/*
	 * [h1 h2] ==> [-h2 h1]
	 */
	((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa]))[0] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa]))[2] = (1-2*aa)*((re_last + re_next)>>1);
	((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa]))[1] = (1-2*aa)*(im_last + im_next)>>1;
	((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1-aa]))[3] = (1-2*aa)*(im_last + im_next)>>1;

    }
#ifdef DEBUG_PHY
    write_output("srs_ch_est_0.m","srs_ce_0",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1);
    write_output("precoder_1.m","prec_1",phy_vars_eNb->dl_precoder_SeNb[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
    write_output("srs_ch_est_1.m","srs_ce_1",phy_vars_eNb->lte_eNB_common_vars.srs_ch_estimates[PeNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size,1,1);
    write_output("precoder_0.m","prec_0",phy_vars_eNb->dl_precoder_SeNb[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
#endif //DEBUG_PHY

    phy_vars_eNb->has_valid_precoder = 1;
    phy_vars_eNb->log2_maxp = 1 + (log2_approx(maxp));
#ifdef DEBUG_PHY
    printf("index of max precoder coefficient, %d\n", ind_of_maxp);
#endif
    break;
  default:
    break;
    }
}

void phy_precode_nullBeam_apply(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb) {
  
  int i, aa, l, slot_n_symb_offset, eNb_id=0, output_norm;
  
  if (((next_slot < 3) || (next_slot > 9)) && phy_vars_eNb->has_valid_precoder) {
    output_norm = log2_approx(iSqrt(signal_energy_nodc(phy_vars_eNb->dl_precoder_SeNb[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)) + iSqrt(signal_energy_nodc(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)));
    if (output_norm > phy_vars_eNb->log2_maxp) {
      phy_vars_eNb->log2_maxp = output_norm;
    }
    if (next_slot==12) {
      //printf("sigenergy %d, output_norm %d\n",signal_energy_nodc(phy_vars_eNb->dl_precoder_SeNb[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size) + signal_energy_nodc(phy_vars_eNb->dl_precoder_SeNb[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size), output_norm);
    }
#ifdef DEBUG_PHY
    if (next_slot==12) {
      write_output("precoder_a0.m","prec_a0",phy_vars_eNb->dl_precoder_SeNb[eNb_id][0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
      write_output("precoder_a1.m","prec_a1",phy_vars_eNb->dl_precoder_SeNb[eNb_id][1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
      write_output("txdataF_a0_before.m","txF_a0_b",&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0][next_slot*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1),1,1);
      write_output("txdataF_a1_before.m","txF_a1_b",&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1][next_slot*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1),1,1);
    }
#endif //DEBUG_PHY
    
    for (l=0; l<phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1; l++) {
      slot_n_symb_offset = next_slot*((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)<<1)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1) + l*((phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)<<1);
      for (aa=0; aa<(phy_vars_eNb->lte_frame_parms.nb_antennas_tx); aa++) {
	for (i=0; i<phy_vars_eNb->lte_frame_parms.ofdm_symbol_size; i++) {
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)] = ((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0]))[slot_n_symb_offset + (i<<1)];          // real part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+1] = -(((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0]))[slot_n_symb_offset + (i<<1)+1]);   //-imag part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+2] = ((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0]))[slot_n_symb_offset + (i<<1)+1];      // imag part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+3] = ((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0]))[slot_n_symb_offset + (i<<1)];        // real part
	} //for(i... OFDM carriers
      } //for(aa... antennas_tx
      
      for (aa=0; aa<(phy_vars_eNb->lte_frame_parms.nb_antennas_tx); aa++) {
	// precode. Using mult_cpx_vector_norep
	
	mult_cpx_vector_norep((short *)(phy_vars_eNb->dl_precoder_SeNb[eNb_id][aa]), // input 1
			      (short *)(txdataF_rep_tmp[aa]), // input 2
			      &(((short *)(phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa]))[slot_n_symb_offset]), // output
			      phy_vars_eNb->lte_frame_parms.ofdm_symbol_size, // length of vectors (512)
			      phy_vars_eNb->log2_maxp // output_shift
			      );
	
      } //for(aa... antennas_tx
    } //for(l... symbols
    
#ifdef DEBUG_PHY
    if (next_slot==11) {
      write_output("txdataF_rep_a0.m","txF_rep_a0",txdataF_rep_tmp[0],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
      write_output("txdataF_rep_a1.m","txF_rep_a1",txdataF_rep_tmp[1],phy_vars_eNb->lte_frame_parms.ofdm_symbol_size<<1,2,1);
      write_output("txdataF_a0.m","txF_a0",&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0][next_slot*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1),1,1);
      write_output("txdataF_a1.m","txF_a1",&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1][next_slot*(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],(phy_vars_eNb->lte_frame_parms.ofdm_symbol_size)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1),1,1);
    }
#endif
  }
  else {
    msg("phy_precode_nullBeam_apply should not have been called, since next_slot is not a DL or the precoder is not ready");
  }
}

void phy_precode_nullBeam_apply_ue(unsigned char next_slot,PHY_VARS_UE *phy_vars_ue) {

  int i, aa, l, slot_n_symb_offset, output_norm;

  if (((next_slot < 10) && (next_slot > 3)) && phy_vars_ue->has_valid_precoder) {
    output_norm = log2_approx(iSqrt(signal_energy_nodc(phy_vars_ue->ul_precoder_S_UE[0],phy_vars_ue->lte_frame_parms.ofdm_symbol_size)) + iSqrt(signal_energy_nodc(phy_vars_ue->ul_precoder_S_UE[1],phy_vars_ue->lte_frame_parms.ofdm_symbol_size)));
    if (output_norm > phy_vars_ue->log2_maxp) {
      phy_vars_ue->log2_maxp = output_norm;
    }


    for (l=0; l<phy_vars_ue->lte_frame_parms.symbols_per_tti>>1; l++) {
      slot_n_symb_offset = next_slot*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1) + l*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1);

      for (aa=0; aa<(phy_vars_ue->lte_frame_parms.nb_antennas_tx); aa++) {
	for (i=0; i<phy_vars_ue->lte_frame_parms.ofdm_symbol_size; i++) {
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)] = ((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[0]))[slot_n_symb_offset + (i<<1)];          // real part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+1] = -(((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[0]))[slot_n_symb_offset + (i<<1)+1]);   //-imag part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+2] = ((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[0]))[slot_n_symb_offset + (i<<1)+1];      // imag part
	  ((short *)(txdataF_rep_tmp[aa]))[(i<<2)+3] = ((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[0]))[slot_n_symb_offset + (i<<1)];        // real part
	} //for(i... OFDM carriers
      } //for(aa... antennas_tx

      for (aa=0; aa<(phy_vars_ue->lte_frame_parms.nb_antennas_tx); aa++) {
      // precode. Using mult_cpx_vector_norep
	
	mult_cpx_vector_norep((short *)(phy_vars_ue->ul_precoder_S_UE[aa]), // input 1
			      (short *)(txdataF_rep_tmp[aa]), // input 2
			      &(((short *)(phy_vars_ue->lte_ue_common_vars.txdataF[aa]))[slot_n_symb_offset]), // output
			      phy_vars_ue->lte_frame_parms.ofdm_symbol_size, // length of vectors (512)
			      phy_vars_ue->log2_maxp // output_shift
			      );
	
      } //for(aa... antennas_tx
    } //for(l... symbols
  }
}

int phy_precode_nullBeam_create_ue(unsigned char last_slot,PHY_VARS_UE *phy_vars_ue) {  

  int aa,i,symb_offset,n_car,PeNb_id=0;
  short zero_detections = 0, ind_of_maxp =0;
  int maxp=0;

  n_car = phy_vars_ue->lte_frame_parms.N_RB_DL*phy_vars_ue->lte_frame_parms.symbols_per_tti; //300 in the 5MHz case
  symb_offset = phy_vars_ue->lte_frame_parms.ofdm_symbol_size-(n_car>>1);

  if (last_slot==2 || last_slot==10 || 1) { //always allow being called? Why not?
    for (aa=0; aa<phy_vars_ue->lte_frame_parms.nb_antennas_rx; aa++) {
      zero_detections = 0;
      for (i=0; i<(n_car>>1); i++) {
	
      //positive frequencies
	phy_vars_ue->ul_precoder_S_UE[1-aa][i<<1] = (1-2*aa)*phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][aa][i+(n_car>>1)]; //Re0Im0
	phy_vars_ue->ul_precoder_S_UE[1-aa][(i<<1)+1] = (1-2*aa)*phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][aa][i+(n_car>>1)]; //Re0Im0

	if (phy_vars_ue->ul_precoder_S_UE[1-aa][i<<1] == 0) {
	  zero_detections += 1;
	}
	if (zero_detections > 3 && i<10) {
	  //very unlikely --> thus a probable error
	  //printf("error in creating the precoder");
	  phy_vars_ue->has_valid_precoder = 0;
	  return(-1);
	}

#ifdef RANDOM_BF
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[i<<2] = (1-2*aa)*phy_vars_ue->const_ch[aa][0]; //Re0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+1] = (1-2*aa)*phy_vars_ue->const_ch[aa][1]; //Im0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+2] = (1-2*aa)*phy_vars_ue->const_ch[aa][0]; //Re0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+3] = (1-2*aa)*phy_vars_ue->const_ch[aa][1]; //Im0
#endif //RANDOM_BF

	if (abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[i<<2]) > maxp) {
	  maxp = abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[i<<2]);
	  ind_of_maxp = i;
	} 
	
	if (abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+1]) > maxp) {
	  maxp = abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+1]);
	  ind_of_maxp = i;
	}
	if (abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+2]) > maxp) {
	  maxp = abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+2]);
	  ind_of_maxp = i;
	} 
	if (abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+3]) > maxp) {
	  maxp = abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i<<2)+3]);
	  ind_of_maxp = i;
	}
	
	
      //negative frequencies
	
	phy_vars_ue->ul_precoder_S_UE[1-aa][((i+symb_offset)<<1)] = (1-2*aa)*phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][aa][i]; //Re0Im0
	phy_vars_ue->ul_precoder_S_UE[1-aa][((i+symb_offset)<<1)+1] = (1-2*aa)*phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][aa][i]; //Re0Im0
	
#ifdef RANDOM_BF
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i+symb_offset)<<2] = (1-2*aa)*phy_vars_ue->const_ch[aa][0]; //Re0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+1] = (1-2*aa)*phy_vars_ue->const_ch[aa][1]; //Im0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+2] = (1-2*aa)*phy_vars_ue->const_ch[aa][0]; //Re0
	((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+3] = (1-2*aa)*phy_vars_ue->const_ch[aa][1]; //Im0
#endif //RANDOM_BF

	if (abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i+symb_offset)<<2]) > maxp) {
	  maxp = abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[(i+symb_offset)<<2]);
	  ind_of_maxp = i;
	} 
	
	if (abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+1]) > maxp) {
	  maxp = abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+1]);
	  ind_of_maxp = i;
	}
	if (abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+2]) > maxp) {
	  maxp = abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+2]);
	  ind_of_maxp = i;
	} 
	if (abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+3]) > maxp) {
	  maxp = abs(((short *)(phy_vars_ue->ul_precoder_S_UE[1-aa]))[((i+symb_offset)<<2)+3]);
	  ind_of_maxp = i;
	}
	
	
      }
    }

#ifdef DEBUG_PHY
    write_output("dl_ch_est_0.m","dl_ce_0",phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][0],phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
    write_output("ul_precoder_1.m","ul_prec_1",phy_vars_ue->ul_precoder_S_UE[1],phy_vars_ue->lte_frame_parms.ofdm_symbol_size<<1,2,1);
    write_output("dl_ch_est_1.m","dl_ce_1",phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[PeNb_id][1],phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
    write_output("ul_precoder_0.m","ul_prec_0",phy_vars_ue->ul_precoder_S_UE[0],phy_vars_ue->lte_frame_parms.ofdm_symbol_size<<1,2,1);
#endif //DEBUG_PHY
    
    phy_vars_ue->has_valid_precoder = 1;
    phy_vars_ue->log2_maxp = 1 + (log2_approx(maxp));
#ifdef DEBUG_PHY
    printf("index of max precoder coefficient, %d\n", ind_of_maxp);
#endif
  }
}

void phy_procedures_eNb_lte(unsigned char last_slot, unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNb) {
  char aa;
  int eNb_id=0;
  
  if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,last_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_RX(%d)\n",mac_xface->frame, last_slot);
#endif
    phy_procedures_eNB_RX(last_slot,phy_vars_eNb);
    if (phy_vars_eNb->is_secondary_eNb) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_precode_nullBeam_create(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_precode_nullBeam_create(last_slot,phy_vars_eNb);
    }
  }
  if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,last_slot>>1)==SF_S) {
    if (phy_vars_eNb->is_secondary_eNb) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_RX_secsys(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_eNB_S_RX_secsys(last_slot,phy_vars_eNb);
    }
    else {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_eNB_S_RX(last_slot,phy_vars_eNb);
    }
  }
  if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,next_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_TX(%d)\n",mac_xface->frame, next_slot);
#endif
    if (phy_vars_eNb->is_secondary_eNb) {
      if (phy_vars_eNb->has_valid_precoder) {
	phy_procedures_eNB_TX(next_slot,phy_vars_eNb);
#ifdef NULL_SHAPE_BF_ENABLED
#ifdef DEBUG_PHY
	msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_precode_nullBeam_apply(%d)\n",mac_xface->frame, next_slot);
#endif
	phy_precode_nullBeam_apply(next_slot,phy_vars_eNb);
#else //Copy SISO stream to the other antenna
	  memcpy(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 &phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 (phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));	
#endif //NULL_SHAPE_BF_ENABLED
      }
      else { //make sure tx-buffer is cleared, in case no transmission.
	for (aa=0; aa<(phy_vars_eNb->lte_frame_parms.nb_antennas_tx); aa++) {
#ifdef IFFT_FPGA
	  memset(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 0,(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
	  memset(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 0,phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
	}
      }
    }
    else {
      phy_procedures_eNB_TX(next_slot,phy_vars_eNb);
    }
  }
  if (subframe_select_tdd(phy_vars_eNb->lte_frame_parms.tdd_config,next_slot>>1)==SF_S) {
    if (phy_vars_eNb->is_secondary_eNb && next_slot%2==0) {
      if ( mac_xface->frame%100 && phy_vars_eNb->has_valid_precoder) {
#ifdef DEBUG_PHYgg
	msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
	phy_procedures_eNB_S_TX(next_slot,phy_vars_eNb);
#ifdef NULL_SHAPE_BF_ENABLED
#ifdef DEBUG_PHY
	msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_precode_nullBeam_apply(%d)\n",mac_xface->frame, next_slot);
#endif //DEBUG_PHY
	phy_precode_nullBeam_apply(next_slot,phy_vars_eNb);
#else //Copy SISO stream to the other antenna
	  memcpy(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][1][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 &phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][0][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 (phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));	
#endif //NULL_SHAPE_BF_ENABLED
      } 
      else { //make sure tx-buffer is cleared, in case no transmission.
	for (aa=0; aa<(phy_vars_eNb->lte_frame_parms.nb_antennas_tx); aa++) {
#ifdef IFFT_FPGA
	  memset(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 0,(phy_vars_eNb->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
	  memset(&phy_vars_eNb->lte_eNB_common_vars.txdataF[eNb_id][aa][next_slot*phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti>>1)],
		 0,phy_vars_eNb->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNb->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
	}
      }
    }
    else {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_eNB_S_TX(next_slot,phy_vars_eNb);
    }
  }
}


void phy_procedures_ue_lte(unsigned char last_slot, unsigned char next_slot, PHY_VARS_UE *phy_vars_ue) {
  char aa;
  
  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,last_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
    phy_procedures_UE_RX(last_slot, phy_vars_ue);
    if (phy_vars_ue->is_secondary_ue && last_slot==10 && mac_xface->frame>0) {
      phy_precode_nullBeam_create_ue(last_slot,phy_vars_ue);
    }
  }
  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,last_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
    phy_procedures_UE_RX(last_slot, phy_vars_ue);
    if (phy_vars_ue->is_secondary_ue && last_slot==2 && mac_xface->frame>0) {
      phy_precode_nullBeam_create_ue(last_slot,phy_vars_ue);
    }
  }
  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,next_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
    if (phy_vars_ue->is_secondary_ue) {
      if (phy_vars_ue->has_valid_precoder) {
	phy_procedures_UE_S_TX(next_slot, phy_vars_ue);
#ifdef NULL_SHAPE_BF_ENABLED
	phy_precode_nullBeam_apply_ue(next_slot,phy_vars_ue);
#else //Copy SISO stream to the other antenna
	  memcpy(&phy_vars_ue->lte_ue_common_vars.txdataF[1][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
		 &phy_vars_ue->lte_ue_common_vars.txdataF[0][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
		 (phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));	
#endif //NULL_SHAPE_BF_ENABLED
      }
      else { //make sure tx-buffer is cleared, in case no transmission.
	for (aa=0; aa<(phy_vars_ue->lte_frame_parms.nb_antennas_tx); aa++) {
#ifdef IFFT_FPGA
	  memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
		 0,(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
	  memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
		 0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
	}
      }
    }
    else {
      phy_procedures_UE_S_TX(next_slot, phy_vars_ue);
    }
  }
  if (subframe_select_tdd(phy_vars_ue->lte_frame_parms.tdd_config,next_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
    msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_TX(%d)\n",mac_xface->frame, next_slot);
#endif
    if (phy_vars_ue->is_secondary_ue) {
      if (phy_vars_ue->has_valid_precoder) {
	phy_procedures_UE_TX(next_slot, phy_vars_ue);
#ifdef NULL_SHAPE_BF_ENABLED
	phy_precode_nullBeam_apply_ue(next_slot,phy_vars_ue);
#else //Copy SISO stream to the other antenna
	  memcpy(&phy_vars_ue->lte_ue_common_vars.txdataF[1][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
		 &phy_vars_ue->lte_ue_common_vars.txdataF[0][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
		 (phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif //NULL_SHAPE_BF_ENABLED
      }
      else { //make sure tx-buffer is cleared, in case no transmission.
	for (aa=0; aa<(phy_vars_ue->lte_frame_parms.nb_antennas_tx); aa++) {
#ifdef IFFT_FPGA
	  memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
		 0,(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
	  memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
		 0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
	}
      }
    }
    else {
      phy_procedures_UE_TX(next_slot, phy_vars_ue);
    }
  }
}
