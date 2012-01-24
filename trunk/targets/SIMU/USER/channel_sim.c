#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/extern.h"

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log_if.h"
#include "UTIL/LOG/log_extern.h"
#include "RRC/LITE/extern.h"
#include "PHY_INTERFACE/extern.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OPT/opt.h" // to test OPT
#endif

#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"

#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef XFORMS
#include "forms.h"
#include "phy_procedures_sim_form.h"
#endif

#include "oaisim.h"

#define RF
//#define DEBUG_SIM

void do_OFDM_mod(mod_sym_t **txdataF, s32 **txdata, u16 next_slot, LTE_DL_FRAME_PARMS *frame_parms) {

  int aa, slot_offset, slot_offset_F;

#ifdef IFFT_FPGA
  s32 **txdataF2;
  int i, l;

  txdataF2    = (s32 **)malloc(2*sizeof(s32*));
  txdataF2[0] = (s32 *)malloc(NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7)*sizeof(s32));
  txdataF2[1] = (s32 *)malloc(NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7)*sizeof(s32));
  
  bzero(txdataF2[0],NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7)*sizeof(s32));
  bzero(txdataF2[1],NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7)*sizeof(s32));
  
  slot_offset_F = (next_slot)*(frame_parms->N_RB_DL*12)*((frame_parms->Ncp==1) ? 6 : 7);
  slot_offset = (next_slot)*(frame_parms->samples_per_tti>>1);
  
  //write_output("eNB_txsigF0.m","eNB_txsF0", lte_eNB_common_vars->txdataF[eNB_id][0],300*120,1,4);
  //write_output("eNB_txsigF1.m","eNB_txsF1", lte_eNB_common_vars->txdataF[eNB_id][1],300*120,1,4);
  
  
  // do talbe lookup and write results to txdataF2
  for (aa=0;aa<frame_parms->nb_antennas_tx;aa++) {
    
    l = slot_offset_F;	
    for (i=0;i<NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7);i++) 
      if ((i%512>=1) && (i%512<=150))
	txdataF2[aa][i] = ((s32*)mod_table)[txdataF[aa][l++]];
      else if (i%512>=362)
	txdataF2[aa][i] = ((s32*)mod_table)[txdataF[aa][l++]];
      else 
	txdataF2[aa][i] = 0;
    
  }
  
  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
    if (frame_parms->Ncp == 1)
      PHY_ofdm_mod(txdataF2[aa],        // input
		   &txdata[aa][slot_offset],         // output
		   frame_parms->log2_symbol_size,                // log2_fft_size
		   6,                 // number of symbols
		   frame_parms->nb_prefix_samples,               // number of prefix samples
		   frame_parms->twiddle_ifft,  // IFFT twiddle factors
		   frame_parms->rev,           // bit-reversal permutation
		   CYCLIC_PREFIX);
    else {
      normal_prefix_mod(txdataF2[aa],&txdata[aa][slot_offset],7,frame_parms);
    }
  }
  
  free(txdataF2[0]);
  free(txdataF2[1]);
  free(txdataF2);
  
#else //IFFT_FPGA
  
  slot_offset_F = (next_slot)*(frame_parms->ofdm_symbol_size)*((frame_parms->Ncp==1) ? 6 : 7);
  slot_offset = (next_slot)*(frame_parms->samples_per_tti>>1);
  
  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
    if (frame_parms->Ncp == 1)
      PHY_ofdm_mod(&txdataF[aa][slot_offset_F],        // input
		   &txdata[aa][slot_offset],         // output
		   frame_parms->log2_symbol_size,                // log2_fft_size
		   6,                 // number of symbols
		   frame_parms->nb_prefix_samples,               // number of prefix samples
		   frame_parms->twiddle_ifft,  // IFFT twiddle factors
		   frame_parms->rev,           // bit-reversal permutation
		   CYCLIC_PREFIX);
    else {
      normal_prefix_mod(&txdataF[aa][slot_offset_F],
			&txdata[aa][slot_offset],
			7,
			frame_parms);
    }
  }  
#endif //IFFT_FPGA
  
}

void do_DL_sig(double **r_re0,double **r_im0,
	       double **r_re,double **r_im,
	       double **s_re,double **s_im,
	       channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX],
	       node_desc_t *enb_data[NUMBER_OF_eNB_MAX],
	       node_desc_t *ue_data[NUMBER_OF_UE_MAX], 
	       u16 next_slot,u8 abstraction_flag,LTE_DL_FRAME_PARMS *frame_parms) {

  s32 att_eNB_id=-1;
  s32 **txdata,**rxdata;
  
  u8 eNB_id=0,UE_id=0;
  double tx_pwr, rx_pwr;
  s32 rx_pwr2;
  u32 i,aa;
  u32 slot_offset,slot_offset_meas;

  double min_path_loss=-200;

  u8 hold_channel=0;
  u8 aatx,aarx;

  if (next_slot==0)
    hold_channel = 0;
  else
    hold_channel = 1;

  if (abstraction_flag != 0) {
    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {

      // calculate the random channel from each eNB
      for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
	random_channel(eNB2UE[eNB_id][UE_id]);
	/*
	for (i=0;i<eNB2UE[eNB_id][UE_id]->nb_taps;i++)
	  printf("eNB2UE[%d][%d]->a[0][%d] = (%f,%f)\n",eNB_id,UE_id,i,eNB2UE[eNB_id][UE_id]->a[0][i].x,eNB2UE[eNB_id][UE_id]->a[0][i].y);
	*/
	freq_channel(eNB2UE[eNB_id][UE_id], frame_parms->N_RB_DL,51);
      }

      // find out which eNB the UE is attached to
      for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
	if (find_ue(PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[eNB_id]->crnti,PHY_vars_eNB_g[eNB_id])>=0) {
	  // UE with UE_id is connected to eNb with eNB_id
	  att_eNB_id=eNB_id;
	  LOG_I(OCM,"UE attached to eNB (UE%d->eNB%d)\n",UE_id,eNB_id);
	}
      }

      // if UE is not attached yet, find assume its the eNB with the smallest pathloss
      if (att_eNB_id<0) {
	for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
	  if (min_path_loss<eNB2UE[eNB_id][UE_id]->path_loss_dB) {
	    min_path_loss = eNB2UE[eNB_id][UE_id]->path_loss_dB;
	    att_eNB_id=eNB_id;
	    LOG_I(OCM,"UE attached to eNB (UE%d->eNB%d)\n",UE_id,eNB_id);
	  }
	}
      }

      if (att_eNB_id<0) {
	LOG_E(OCM,"Cannot find eNB for UE %d, return\n",UE_id);
	return; //exit(-1);
      }

      rx_pwr = signal_energy_fp2(eNB2UE[att_eNB_id][UE_id]->ch[0],
				 eNB2UE[att_eNB_id][UE_id]->channel_length)*eNB2UE[att_eNB_id][UE_id]->channel_length;
      LOG_D(OCM,"Channel eNB %d => UE %d : tx_power %f dBm, path_loss %f dB\n",
	     eNB_id,UE_id,
	     PHY_vars_eNB_g[att_eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower,
	     //	     enb_data[att_eNB_id]->tx_power_dBm,
	     
	     eNB2UE[att_eNB_id][UE_id]->path_loss_dB);
      LOG_I(OCM,"Channel eNB %d => UE %d : gain %f dB (%f)\n",att_eNB_id,UE_id,10*log10(rx_pwr),rx_pwr);  

    
      // calculate the SNR for the attached eNB
      init_snr(eNB2UE[att_eNB_id][UE_id],  enb_data[att_eNB_id], ue_data[UE_id],PHY_vars_UE_g[UE_id]->sinr_dB,&PHY_vars_UE_g[UE_id]->N0);

      // calculate sinr here
      for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
	if (att_eNB_id != eNB_id) {
	  calculate_sinr(eNB2UE[eNB_id][UE_id], enb_data[eNB_id], ue_data[UE_id], PHY_vars_UE_g[UE_id]->sinr_dB);
	}
      }
      //dlsch_abstraction(PHY_vars_UE_g[UE_id]->sinr_dB, rb_alloc, 8);

      // fill in perfect channel estimates
      channel_desc_t *desc1;
      s32 **dl_channel_est = PHY_vars_UE_g[UE_id]->lte_ue_common_vars.dl_ch_estimates[0];
      s16 nb_samples=301;
      //      double scale = pow(10.0,(enb_data[att_eNB_id]->tx_power_dBm + eNB2UE[att_eNB_id][UE_id]->path_loss_dB + (double) PHY_vars_UE_g[UE_id]->rx_total_gain_dB)/20.0);
      double scale = pow(10.0,(PHY_vars_eNB_g[att_eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower+eNB2UE[att_eNB_id][UE_id]->path_loss_dB + (double) PHY_vars_UE_g[UE_id]->rx_total_gain_dB)/20.0);
      //      printf("[CHANNEL_SIM] scale =%lf (%d dB)\n",scale,(int) (20*log10(scale)));
      desc1 = eNB2UE[att_eNB_id][UE_id];
      freq_channel(desc1,frame_parms->N_RB_DL,nb_samples);
      //write_output("channel.m","ch",desc1->ch[0],desc1->channel_length,1,8);
      //write_output("channelF.m","chF",desc1->chF[0],nb_samples,1,8);
      int count,count1,a_rx,a_tx;
      for(a_tx=0;a_tx<frame_parms->nb_antennas_tx;a_tx++)
	{ 
	  for (a_rx=0;a_rx<frame_parms->nb_antennas_rx;a_rx++)
	    {
	      for (count=0;count<frame_parms->symbols_per_tti/2;count++)
		{ 
		  for (count1=0;count1<frame_parms->N_RB_DL*12;count1++)
		    { 
		      ((s16 *) dl_channel_est[(a_tx<<1)+a_rx])[2*count1+(count*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(desc1->chF[a_rx+(a_tx*frame_parms->nb_antennas_rx)][count1].x*scale);
		      ((s16 *) dl_channel_est[(a_tx<<1)+a_rx])[2*count1+1+(count*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(desc1->chF[a_rx+(a_tx*frame_parms->nb_antennas_rx)][count1].y*scale) ;
		    }
		}
	    }
	}
      
    } //UE_id
  }
  
  else { //abstraction_flag

    for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
      do_OFDM_mod(PHY_vars_eNB_g[eNB_id]->lte_eNB_common_vars.txdataF[0],
		  PHY_vars_eNB_g[eNB_id]->lte_eNB_common_vars.txdata[0],
		  next_slot,
		  &PHY_vars_eNB_g[eNB_id]->lte_frame_parms);
      /*
      do_OFDM_mod(PHY_vars_eNB_g[eNB_id]->lte_eNB_common_vars.txdataF[0],
		  PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata,
		  next_slot,
		  &PHY_vars_eNB_g[eNB_id]->lte_frame_parms);
      return;
      */
    }

    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
      // Compute RX signal for UE = UE_id
      for (i=0;i<(frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
	  r_re[aa][i]=0.0;
	  r_im[aa][i]=0.0;
	}
      }

      for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
	//	if (((double)PHY_vars_UE_g[UE_id]->tx_power_dBm + 
	//	     eNB2UE[eNB_id][UE_id]->path_loss_dB) <= -107.0)
	//	  break;
	frame_parms = &PHY_vars_eNB_g[eNB_id]->lte_frame_parms;
	txdata = PHY_vars_eNB_g[eNB_id]->lte_eNB_common_vars.txdata[0];
	slot_offset = (next_slot)*(frame_parms->samples_per_tti>>1);
	slot_offset_meas = ((next_slot&1)==0) ? slot_offset : (slot_offset-(frame_parms->samples_per_tti>>1));
	tx_pwr = dac_fixed_gain(s_re,
				s_im,
				txdata,
				slot_offset,
				frame_parms->nb_antennas_tx,
				frame_parms->samples_per_tti>>1,
				slot_offset_meas,
				frame_parms->ofdm_symbol_size,
				14,
				//				enb_data[eNB_id]->tx_power_dBm); 
				PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower);
#ifdef DEBUG_SIM
	LOG_D(OCM,"eNB %d: tx_pwr %f dBm, for slot %d (subframe %d)\n",
	       eNB_id,
	       10*log10(tx_pwr),
	       next_slot,
	       next_slot>>1);
#endif
	//eNB2UE[eNB_id][UE_id]->path_loss_dB = 0;
	multipath_channel(eNB2UE[eNB_id][UE_id],s_re,s_im,r_re0,r_im0,
			  frame_parms->samples_per_tti>>1,hold_channel);
#ifdef DEBUG_SIM	  
	rx_pwr = signal_energy_fp2(eNB2UE[eNB_id][UE_id]->ch[0],
				   eNB2UE[eNB_id][UE_id]->channel_length)*eNB2UE[eNB_id][UE_id]->channel_length;
	LOG_D(OCM,"Channel eNB %d => UE %d : Channel gain %f dB (%f)\n",eNB_id,UE_id,10*log10(rx_pwr),rx_pwr);  
#endif
	
	
#ifdef DEBUG_SIM
	//	for (i=0;i<eNB2UE[eNB_id][UE_id]->channel_length;i++)
	//	  printf("ch(%d,%d)[%d] : (%f,%f)\n",eNB_id,UE_id,i,eNB2UE[eNB_id][UE_id]->ch[0][i]);

	printf("[SIM][DL] Channel eNB %d => UE %d : tx_power %f dBm, path_loss %f dB\n",
	       eNB_id,UE_id,
	       (double)PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower,
	       //	       enb_data[eNB_id]->tx_power_dBm,
	       eNB2UE[eNB_id][UE_id]->path_loss_dB);

#endif
#ifdef DEBUG_SIM      
	rx_pwr = signal_energy_fp(r_re0,r_im0,frame_parms->nb_antennas_rx,512,0);
	printf("[SIM][DL] UE %d : rx_pwr %f dBm for slot %d (subframe %d)\n",UE_id,10*log10(rx_pwr),next_slot,next_slot>>1);  
#endif      
	
	if (eNB2UE[eNB_id][UE_id]->first_run == 1)
	  eNB2UE[eNB_id][UE_id]->first_run = 0;
	
	
	// RF model
#ifdef DEBUG_SIM
	printf("[SIM][DL] UE %d : rx_gain %d dB for slot %d (subframe %d)\n",UE_id,PHY_vars_UE_g[UE_id]->rx_total_gain_dB,next_slot,next_slot>>1);      
#endif
	/*
	rf_rx(r_re0,
	      r_im0,
	      NULL,
	      NULL,
	      0,
	      frame_parms->nb_antennas_rx,
	      frame_parms->samples_per_tti>>1,
	      1e3/eNB2UE[eNB_id][UE_id]->BW,  // sampling time (ns)
	      0.0,               // freq offset (Hz) (-20kHz..20kHz)
	      0.0,               // drift (Hz) NOT YET IMPLEMENTED
	      ue_data[UE_id]->rx_noise_level,                // noise_figure NOT YET IMPLEMENTED
	      (double)PHY_vars_UE_g[UE_id]->rx_total_gain_dB - 66.227,   // rx_gain (dB) (66.227 = 20*log10(pow2(11)) = gain from the adc that will be applied later)
	      200.0,               // IP3_dBm (dBm)
	      &eNB2UE[eNB_id][UE_id]->ip,               // initial phase
	      30.0e3,            // pn_cutoff (kHz)
	      -500.0,            // pn_amp (dBc) default: 50
	      0.0,               // IQ imbalance (dB),
	      0.0);              // IQ phase imbalance (rad)
	*/

	rf_rx_simple(r_re0,
		     r_im0,
		     frame_parms->nb_antennas_rx,
		     frame_parms->samples_per_tti>>1,
		     1e3/eNB2UE[eNB_id][UE_id]->BW,  // sampling time (ns)
		     (double)PHY_vars_UE_g[UE_id]->rx_total_gain_dB - 66.227);   // rx_gain (dB) (66.227 = 20*log10(pow2(11)) = gain from the adc that will be applied later)
		     
	rx_pwr = signal_energy_fp(r_re0,r_im0,frame_parms->nb_antennas_rx,512,0);
#ifdef DEBUG_SIM    
	printf("[SIM][DL] UE %d : ADC in (eNB %d) %f dB for slot %d (subframe %d)\n",
	       UE_id,eNB_id,
	       10*log10(rx_pwr),next_slot,next_slot>>1);  
#endif    	
	for (i=0;i<(frame_parms->samples_per_tti>>1);i++) {
	  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
	    r_re[aa][i]+=r_re0[aa][i]; 
	    r_im[aa][i]+=r_im0[aa][i]; 
	    
	  }
	}
	
      }      
      rx_pwr = signal_energy_fp(r_re,r_im,frame_parms->nb_antennas_rx,512,0);
#ifdef DEBUG_SIM    
      printf("[SIM][DL] UE %d : ADC in %f dB for slot %d (subframe %d)\n",UE_id,10*log10(rx_pwr),next_slot,next_slot>>1);  
#endif    

      rxdata = PHY_vars_UE_g[UE_id]->lte_ue_common_vars.rxdata;
      slot_offset = (next_slot)*(frame_parms->samples_per_tti>>1);
      
      adc(r_re,
	  r_im,
	  0,
	  slot_offset,
	  rxdata,
	  frame_parms->nb_antennas_rx,
	  frame_parms->samples_per_tti>>1,
	  12);
      
      rx_pwr2 = signal_energy(rxdata[0]+slot_offset,512);
#ifdef DEBUG_SIM    
      printf("[SIM][DL] UE %d : rx_pwr (ADC out) %f dB (%d) for slot %d (subframe %d), writing to %p\n",UE_id, 10*log10((double)rx_pwr2),rx_pwr2,next_slot,next_slot>>1,rxdata);  
#endif
    }
  } // UE_index loop

}


void do_UL_sig(double **r_re0,double **r_im0,double **r_re,double **r_im,double **s_re,double **s_im,channel_desc_t *UE2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX],node_desc_t *enb_data[NUMBER_OF_eNB_MAX],node_desc_t *ue_data[NUMBER_OF_UE_MAX],u16 next_slot,u8 abstraction_flag,LTE_DL_FRAME_PARMS *frame_parms) {

  s32 **txdata,**rxdata;

  u8 UE_id=0,eNB_id=0,aa;
  double tx_pwr, rx_pwr;
  s32 rx_pwr2;
  u32 i;
  u32 slot_offset,slot_offset_meas;

  u8 hold_channel=1;
  u8 aatx,aarx;

  if (next_slot==4) {
    hold_channel = 0;
  }


  if (abstraction_flag!=0) {
    for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
      for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
	random_channel(UE2eNB[UE_id][eNB_id]);
	freq_channel(UE2eNB[UE_id][eNB_id], frame_parms->N_RB_UL,2);
      }
    }
  }
  else { //abstraction

    /*
    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
      do_OFDM_mod(PHY_vars_UE_g[UE_id]->lte_ue_common_vars.txdataF,PHY_vars_UE_g[UE_id]->lte_ue_common_vars.txdata,next_slot,&PHY_vars_UE_g[UE_id]->lte_frame_parms);
    }
    */

    for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
      // Clear RX signal for eNB = eNB_id
      for (i=0;i<(frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
	  r_re[aa][i]=0.0;
	  r_im[aa][i]=0.0;
	}
      }
      
      // Compute RX signal for eNB = eNB_id
      for (UE_id=0;UE_id<NB_UE_INST;UE_id++){

	txdata = PHY_vars_UE_g[UE_id]->lte_ue_common_vars.txdata;
	frame_parms = &PHY_vars_UE_g[UE_id]->lte_frame_parms;
	slot_offset = (next_slot)*(frame_parms->samples_per_tti>>1);
	slot_offset_meas = ((next_slot&1)==0) ? slot_offset : (slot_offset-(frame_parms->samples_per_tti>>1));

	if (((double)PHY_vars_UE_g[UE_id]->tx_power_dBm + 	
	     UE2eNB[UE_id][eNB_id]->path_loss_dB) <= -125.0) {
	  
	  // don't simulate a UE that is 
	}
	else {
	  
	  tx_pwr = dac_fixed_gain(s_re,
				  s_im,
				  txdata,
				  slot_offset,
				  frame_parms->nb_antennas_tx,
				  frame_parms->samples_per_tti>>1,
				  slot_offset_meas,
				  frame_parms->ofdm_symbol_size,
				  14,
				  PHY_vars_UE_g[UE_id]->tx_power_dBm);
	  //				ue_data[UE_id]->tx_power_dBm); 
#ifdef DEBUG_SIM
	  printf("[SIM][UL] UE %d tx_pwr %f dBm for slot %d (subframe %d, slot_offset %d, slot_offset_meas %d)\n",UE_id,10*log10(tx_pwr),next_slot,next_slot>>1,slot_offset,slot_offset_meas);
#endif
	  
	  
	  rx_pwr = signal_energy_fp(s_re,s_im,frame_parms->nb_antennas_rx,frame_parms->samples_per_tti>>1,0);
#ifdef DEBUG_SIM    
	  printf("[SIM][UL] UE %d rx_pwr %f dBm for slot %d (subframe %d)\n",UE_id,10*log10(rx_pwr),next_slot,next_slot>>1);  
#endif
	  
	  multipath_channel(UE2eNB[UE_id][eNB_id],s_re,s_im,r_re0,r_im0,
			    frame_parms->samples_per_tti>>1,hold_channel);

#ifdef DEBUG_SIM	  
	  rx_pwr = signal_energy_fp2(UE2eNB[UE_id][eNB_id]->ch[0],
				     UE2eNB[UE_id][eNB_id]->channel_length)*UE2eNB[UE_id][eNB_id]->channel_length;
	  printf("[SIM][UL] slot %d Channel UE %d => eNB %d : %f dB (hold %d)\n",next_slot,UE_id,eNB_id,10*log10(rx_pwr),hold_channel);  
#endif
	  
#ifdef DEBUG_SIM    
	  rx_pwr = signal_energy_fp(r_re0,r_im0,frame_parms->nb_antennas_rx,frame_parms->samples_per_tti>>1,0);
	  printf("[SIM][UL] eNB %d : eNB out %f dB (%f) for slot %d (subframe %d), sptti %d\n",
		 eNB_id,10*log10(rx_pwr),rx_pwr,next_slot,next_slot>>1,frame_parms->samples_per_tti);  
#endif
	  
	  
	  if (UE2eNB[UE_id][eNB_id]->first_run == 1)
	    UE2eNB[UE_id][eNB_id]->first_run = 0;
	  
	  
	  
	  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
	    for (i=0;i<(frame_parms->samples_per_tti>>1);i++) {
	      r_re[aa][i]+=r_re0[aa][i]; 
	      r_im[aa][i]+=r_im0[aa][i]; 
	    }
	  }
	}
      } //UE_id
      
	// RF model
	/*
	  rf_rx(r_re0,
	  r_im0,
	  NULL,
	  NULL,
	  0,
	  frame_parms->nb_antennas_rx,
	  frame_parms->samples_per_tti>>1,
	  1e3/UE2eNB[UE_id][eNB_id]->BW,  // sampling time (ns) 
	  0.0,               // freq offset (Hz) (-20kHz..20kHz)
	  0.0,               // drift (Hz) NOT YET IMPLEMENTED
	  enb_data[eNB_id]->rx_noise_level,                // noise_figure NOT YET IMPLEMENTED
	  (double)PHY_vars_eNB_g[eNB_id]->rx_total_gain_eNB_dB - 66.227,   // rx_gain (dB) (66.227 = 20*log10(pow2(11)) = gain from the adc that will be applied later)
	  200.0,               // IP3_dBm (dBm)
	  &UE2eNB[UE_id][eNB_id]->ip,               // initial phase
	  30.0e3,            // pn_cutoff (kHz)
	  -500.0,            // pn_amp (dBc) default: 50
	  0.0,               // IQ imbalance (dB),
	  0.0);              // IQ phase imbalance (rad)
	*/
      
      rf_rx_simple(r_re,
		   r_im,
		   frame_parms->nb_antennas_rx,
		   frame_parms->samples_per_tti>>1,
		   1e3/UE2eNB[0][eNB_id]->BW,  // sampling time (ns) 
		   (double)PHY_vars_eNB_g[eNB_id]->rx_total_gain_eNB_dB - 66.227);   // rx_gain (dB) (66.227 = 20*log10(pow2(11)) = gain from the adc that will be applied later)

      rx_pwr = signal_energy_fp(r_re,r_im,frame_parms->nb_antennas_rx,frame_parms->samples_per_tti>>1,0);
#ifdef DEBUG_SIM    
      printf("[SIM][UL] rx_pwr (ADC in) %f dB for slot %d (subframe %d)\n",10*log10(rx_pwr),next_slot,next_slot>>1);  
#endif
      
      rxdata = PHY_vars_eNB_g[eNB_id]->lte_eNB_common_vars.rxdata[0];
      slot_offset = (next_slot)*(frame_parms->samples_per_tti>>1);
      
      adc(r_re,
	  r_im,
	  0,
	  slot_offset,
	  rxdata,
	  frame_parms->nb_antennas_rx,
	  frame_parms->samples_per_tti>>1,
	  12);
      
      rx_pwr2 = signal_energy(rxdata[0]+slot_offset,frame_parms->samples_per_tti>>1);
#ifdef DEBUG_SIM    
      printf("[SIM][UL] eNB %d rx_pwr (ADC out) %f dB (%d) for slot %d (subframe %d)\n",eNB_id,10*log10((double)rx_pwr2),rx_pwr2,next_slot,next_slot>>1);  
#endif    
      
    } // eNB_id
  } // abstraction_flag==0
  
}


void init_channel_vars(LTE_DL_FRAME_PARMS *frame_parms, double ***s_re,double ***s_im,double ***r_re,double ***r_im,double ***r_re0,double ***r_im0) {

  int i;

  *s_re = malloc(2*sizeof(double*));
  *s_im = malloc(2*sizeof(double*)); 
  *r_re = malloc(2*sizeof(double*));
  *r_im = malloc(2*sizeof(double*));
  *r_re0 = malloc(2*sizeof(double*));
  *r_im0 = malloc(2*sizeof(double*));
  
  
  for (i=0;i<2;i++) {
    
    (*s_re)[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero((*s_re)[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    (*s_im)[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero((*s_im)[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    (*r_re)[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero((*r_re)[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    (*r_im)[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero((*r_im)[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    (*r_re0)[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero((*r_re0)[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    (*r_im0)[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero((*r_im0)[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }
  
}

