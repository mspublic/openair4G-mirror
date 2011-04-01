/*________________________phy_procedures_lte.c________________________

  Authors : Raymond Knopp, Florian Kaltenberger
  Company : EURECOM
  Emails  : knopp@eurecom.fr, kaltenbe@eurecom.fr
  ________________________________________________________________*/


#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

//#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
//#endif

//#ifdef USER_MODE
#define DEBUG_PHY
//#endif

#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#endif

//#define DIAG_PHY


#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 23/25 RBs)

#define NS_PER_SLOT 500000

extern inline unsigned int taus(void);
extern int exit_openair;

u8 ulsch_input_buffer[2700] __attribute__ ((aligned(16)));
//u8 *RRCConnectionRequest_ptr;

#ifdef DLSCH_THREAD
extern int dlsch_instance_cnt[8];
extern int dlsch_subframe[8];
extern pthread_mutex_t dlsch_mutex[8];
/// Condition variable for dlsch thread
extern pthread_cond_t dlsch_cond[8];
#endif

DCI_ALLOC_t dci_alloc_rx[8];

#ifdef EMOS
fifo_dump_emos_UE emos_dump_UE;
#endif

#ifdef DIAG_PHY
extern int rx_sig_fifo;
#endif

//unsigned int ue_RRCConnReq_frame;
//u8 ue_RRCConnReq_subframe;

#ifdef USER_MODE

void dump_dlsch(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe) {
  unsigned int coded_bits_per_codeword;

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
				  phy_vars_ue->dlsch_ue[eNB_id][0]->nb_rb,
				  phy_vars_ue->dlsch_ue[eNB_id][0]->rb_alloc,
				  get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->mcs),  
				  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,subframe);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_dlsch_vars[0]->rxdataF_ext[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[0],300*12,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_dlsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_dlsch_vars[0]->rxdataF_comp[0],300*12,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_dlsch_vars[0]->llr[0],coded_bits_per_codeword,1,0);
  
  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_mag,300*12,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_dlsch_vars[0]->dl_ch_magb,300*12,1,1);
}

void dump_dlsch_SI(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe) {
  unsigned int coded_bits_per_codeword;
  u8 nsymb = ((phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12);

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
				  phy_vars_ue->dlsch_ue_SI[eNB_id]->nb_rb,
				  phy_vars_ue->dlsch_ue_SI[eNB_id]->rb_alloc,
				  get_Qm(phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs),  
				  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,subframe);
  printf("Dumping dlsch_SI : nb_rb %d, mcs %d, nb_rb %d, num_pdcch_symbols %d,G %d\n",
	 phy_vars_ue->dlsch_ue_SI[eNB_id]->nb_rb,
	 phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs,  
	 phy_vars_ue->dlsch_ue_SI[eNB_id]->nb_rb,  
	 phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
	 coded_bits_per_codeword);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_dlsch_vars_SI[0]->rxdataF_ext[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_dlsch_vars_SI[0]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_dlsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_dlsch_vars_SI[0]->rxdataF_comp[0],300*nsymb,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_dlsch_vars_SI[0]->llr[0],coded_bits_per_codeword,1,0);
  
  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_dlsch_vars_SI[0]->dl_ch_mag,300*nsymb,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_dlsch_vars_SI[0]->dl_ch_magb,300*nsymb,1,1);
}



void dump_dlsch_ra(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe) {
  unsigned int coded_bits_per_codeword;
  u8 nsymb = ((phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12);

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
				  phy_vars_ue->dlsch_ue_ra[eNB_id]->nb_rb,
				  phy_vars_ue->dlsch_ue_ra[eNB_id]->rb_alloc,
				  get_Qm(phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->mcs),  
				  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,subframe);
  printf("Dumping dlsch_ra : nb_rb %d, mcs %d, nb_rb %d, num_pdcch_symbols %d,G %d\n",
	 phy_vars_ue->dlsch_ue_ra[eNB_id]->nb_rb,
	 phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->mcs,  
	 phy_vars_ue->dlsch_ue_ra[eNB_id]->nb_rb,  
	 phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
	 coded_bits_per_codeword);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_dlsch_vars_ra[0]->rxdataF_ext[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_dlsch_vars_ra[0]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_dlsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_dlsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_dlsch_vars_ra[0]->rxdataF_comp[0],300*nsymb,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_dlsch_vars_ra[0]->llr[0],coded_bits_per_codeword,1,0);
  
  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_dlsch_vars_ra[0]->dl_ch_mag,300*nsymb,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_dlsch_vars_ra[0]->dl_ch_magb,300*nsymb,1,1);
}
#endif


void process_timing_advance_rar(PHY_VARS_UE *phy_vars_ue,u16 timing_advance) {
  
  u8 card_id;

  if ((timing_advance>>10) & 1) //it is negative
    timing_advance = timing_advance - (1<<11);
  
  if (openair_daq_vars.manual_timing_advance == 0) {
    openair_daq_vars.timing_advance = cmax(0,TIMING_ADVANCE_INIT + timing_advance*4);
    
#ifdef CBMIMO1
    for (card_id=0;card_id<number_of_cards;card_id++)
      pci_interface[card_id]->timing_advance = openair_daq_vars.timing_advance;
#endif
  }
  
  debug_msg("[PHY][UE %d] Frame %d, received (rar) timing_advance = %d (%d)\n",phy_vars_ue->Mod_id,mac_xface->frame, timing_advance,openair_daq_vars.timing_advance);

}

void process_timing_advance(u8 timing_advance) {

  u8 card_id;

  if ((timing_advance>>5) & 1) //it is negative
    timing_advance = timing_advance - (1<<6);
  
  if (openair_daq_vars.manual_timing_advance == 0) {
    if ( (mac_xface->frame % 100) == 0) {
      if ((timing_advance > 3) || (timing_advance < -3) )
	openair_daq_vars.timing_advance = cmax(0,(int)openair_daq_vars.timing_advance+timing_advance*4);
      
#ifdef CBMIMO1
      for (card_id=0;card_id<number_of_cards;card_id++)
	pci_interface[card_id]->timing_advance = openair_daq_vars.timing_advance;
#endif
      
    }
  }
}

#ifdef EMOS
void phy_procedures_emos_UE_TX(u8 next_slot,u8 eNB_id) {
  u8 harq_pid;

  if (next_slot%2==0) {      
    // get harq_pid from subframe relationship
    harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,(next_slot>>1));    
    if (harq_pid==255) {
      msg("ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n");
      return;
    }

    if (ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {
      emos_dump_UE.uci_cnt[next_slot>>1] = 1;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o,ulsch_ue[eNB_id]->o,MAX_CQI_BITS*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O = ulsch_ue[eNB_id]->O;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o_RI,ulsch_ue[eNB_id]->o_RI,2*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O_RI = ulsch_ue[eNB_id]->O_RI;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o_ACK,ulsch_ue[eNB_id]->o_ACK,4*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O_ACK = ulsch_ue[eNB_id]->O_ACK;
    }
    else {
      emos_dump_UE.uci_cnt[next_slot>>1] = 0;
    }
  }
}
#endif

void phy_procedures_UE_TX(u8 next_slot,PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag) {
  
  u16 first_rb, nb_rb;
  u8 harq_pid;
  unsigned int input_buffer_length;
  unsigned int i, aa;
  u8 RRCConnReq_flag=0;
  u8 pucch_ack_payload[2];
  u8 n1_pucch;

#ifdef EMOS
  phy_procedures_emos_UE_TX(next_slot);
#endif

  if ((next_slot%2)==0) {
    if ((abstraction_flag==0)) {      
      for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++){
	//  printf("Clearing TX buffer\n");
#ifdef IFFT_FPGA
	memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
	       0,(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
	memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
	       0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
      }
      
      if (phy_vars_ue->UE_mode[eNB_id] != PRACH) {
#ifdef DEBUG_PHY
	debug_msg("[PHY][UE %d] Frame %d, slot %d: Generating SRS\n",phy_vars_ue->Mod_id,mac_xface->frame,next_slot);
#endif
#ifdef OFDMA_ULSCH
	generate_srs_tx(&phy_vars_ue->lte_frame_parms,&phy_vars_ue->soundingrs_ul_config_dedicated,phy_vars_ue->lte_ue_common_vars.txdataF[0],AMP,next_slot>>1);
#else
	generate_srs_tx(&phy_vars_ue->lte_frame_parms,&phy_vars_ue->soundingrs_ul_config_dedicated,phy_vars_ue->lte_ue_common_vars.txdataF[0],scfdma_amps[12],next_slot>>1);
#endif
      }
    }
    else {
      generate_srs_tx_emul(phy_vars_ue,next_slot>>1);
    }

    // get harq_pid from subframe relationship
    harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,(next_slot>>1));


    /*  printf("UE %d RRCConnReq_active[eNB_id] %d, frame %d subframe %d\n",
	   phy_vars_ue->Mod_id,
	   phy_vars_ue->ulsch_ue_RRCConnReq_active[eNB_id],
       	   phy_vars_ue->ulsch_ue_RRCConnReq_frame[eNB_id],
	   phy_vars_ue->ulsch_ue_RRCConnReq_subframe[eNB_id]);
    */
#ifdef OPENAIR2

    if ((phy_vars_ue->ulsch_ue_RRCConnReq_active[eNB_id] == 1) && (phy_vars_ue->ulsch_ue_RRCConnReq_frame[eNB_id] == mac_xface->frame) && (phy_vars_ue->ulsch_ue_RRCConnReq_subframe[eNB_id] == (next_slot>>1))) {
      //      harq_pid = 0;
      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
      generate_ue_ulsch_params_from_rar(phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b,
					phy_vars_ue->ulsch_ue_RRCConnReq_subframe[eNB_id],
					phy_vars_ue->ulsch_ue[eNB_id],
					&phy_vars_ue->PHY_measurements,
					&phy_vars_ue->lte_frame_parms,
					eNB_id,
					phy_vars_ue->current_dlsch_cqi[eNB_id]);
      phy_vars_ue->ulsch_ue[eNB_id]->power_offset = 14;
      msg("[PHY][UE %d]: Setting RRCConnReq_flag in subframe %d, for harq_pid %d\n",phy_vars_ue->Mod_id,next_slot>>1,harq_pid);
      RRCConnReq_flag = 1;
      
    }
    else {
      
      if (harq_pid==255) {
	msg("ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n");
	return;
      }
      RRCConnReq_flag=0;
    }
#endif

    if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {

      // deactivate service request
      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;

      get_ack(&phy_vars_ue->lte_frame_parms,
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack,
	      (next_slot>>1),
	      phy_vars_ue->ulsch_ue[eNB_id]->o_ACK);
      
      first_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb;
      nb_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb;
      msg("[PHY][UE %d] Subframe %d Generating PUSCH (harq_pid %d): first_rb %d, nb_rb %d, ACK (%d,%d)\n",phy_vars_ue->Mod_id,next_slot>>1,harq_pid,first_rb,nb_rb,phy_vars_ue->ulsch_ue[eNB_id]->o_ACK[0],phy_vars_ue->ulsch_ue[eNB_id]->o_ACK[1]);
      
      if (abstraction_flag==0) {
#ifdef OFDMA_ULSCH      
	generate_drs_pusch(&phy_vars_ue->lte_frame_parms,phy_vars_ue->lte_ue_common_vars.txdataF[0],AMP,next_slot>>1,first_rb,nb_rb,0,0,0);
#else
	generate_drs_pusch(&phy_vars_ue->lte_frame_parms,phy_vars_ue->lte_ue_common_vars.txdataF[0],scfdma_amps[nb_rb],next_slot>>1,first_rb,nb_rb,0,0,0);
#endif
      }      
      input_buffer_length = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS/8;
      //      msg("[PHY][UE] Frame %d, subframe %d : ULSCH SDU (TX)  input buffer (%d bytes) : ",mac_xface->frame,next_slot>>1,phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS/8);

      for (i=0;i<input_buffer_length;i++) {
	ulsch_input_buffer[i]= (u8)(taus()&0xff);
      }
#ifdef DEBUG_PHY      
      debug_msg("[PHY][UE %d] Frame %d, Subframe %d ulsch harq_pid %d : O %d, O_ACK %d, O_RI %d, TBS %d\n",phy_vars_ue->Mod_id,mac_xface->frame,next_slot>>1,harq_pid,phy_vars_ue->ulsch_ue[eNB_id]->O,phy_vars_ue->ulsch_ue[eNB_id]->O_ACK,phy_vars_ue->ulsch_ue[eNB_id]->O_RI,phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS);
#endif
      
      if (RRCConnReq_flag == 1) {
	msg("[PHY][UE %d] Frame %d, Subframe %d next slot %d Generating RRCConnReq (nb_rb %d, first_rb %d) : %x,%x,%x,%x,%x,%x\n",phy_vars_ue->Mod_id,mac_xface->frame,next_slot>>1, next_slot, 
	    phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,		     
	    phy_vars_ue->RRCConnectionRequest_ptr[eNB_id][0],
	    phy_vars_ue->RRCConnectionRequest_ptr[eNB_id][1],
	    phy_vars_ue->RRCConnectionRequest_ptr[eNB_id][2],
	    phy_vars_ue->RRCConnectionRequest_ptr[eNB_id][3],
	    phy_vars_ue->RRCConnectionRequest_ptr[eNB_id][4],
	    phy_vars_ue->RRCConnectionRequest_ptr[eNB_id][5]);
	if (abstraction_flag==0)
	  ulsch_encoding(phy_vars_ue->RRCConnectionRequest_ptr[eNB_id],&phy_vars_ue->lte_frame_parms,phy_vars_ue->ulsch_ue[eNB_id],harq_pid,0);

	else
	  ulsch_encoding_emul(phy_vars_ue->RRCConnectionRequest_ptr[eNB_id],phy_vars_ue,eNB_id,harq_pid,0);
	}
      else {
	msg("[PHY][UE %d] ULSCH : Searching for MAC SDUs\n",phy_vars_ue->Mod_id);
#ifdef OPENAIR2
	if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->Ndi==1)
	  mac_xface->ue_get_sdu(phy_vars_ue->Mod_id,0,ulsch_input_buffer,input_buffer_length);
#endif
	msg("[PHY][UE] Frame %d, subframe %d : ULSCH SDU (TX)  (%d bytes) : ",mac_xface->frame,next_slot>>1,phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3);
	for (i=0;i<phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3;i++) 
	  msg("%x.",ulsch_input_buffer[i]);
	msg("\n");
	if (abstraction_flag==0)
	  ulsch_encoding(ulsch_input_buffer,&phy_vars_ue->lte_frame_parms,phy_vars_ue->ulsch_ue[eNB_id],harq_pid,0);
	else
	  ulsch_encoding_emul(ulsch_input_buffer,phy_vars_ue,eNB_id,harq_pid,0);
      }
      if (abstraction_flag == 0) {
#ifdef OFDMA_ULSCH
	ulsch_modulation(phy_vars_ue->lte_ue_common_vars.txdataF,AMP,(next_slot>>1),&phy_vars_ue->lte_frame_parms,phy_vars_ue->ulsch_ue[eNB_id],0,0,0);
#else
	ulsch_modulation(phy_vars_ue->lte_ue_common_vars.txdataF,scfdma_amps[nb_rb],(next_slot>>1),&phy_vars_ue->lte_frame_parms,phy_vars_ue->ulsch_ue[eNB_id],0,0,0);
      
#endif
      }

    } // ULSCH is active
    else {  // check if we need to use PUCCH
      get_ack(&phy_vars_ue->lte_frame_parms,phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack,(next_slot>>1),pucch_ack_payload);
      
      generate_pucch(phy_vars_ue->lte_ue_common_vars.txdataF,
		     &phy_vars_ue->lte_frame_parms,
		     phy_vars_ue->ncs_cell,
		     (phy_vars_ue->lte_frame_parms.frame_type==0)?pucch_format1a : pucch_format1b,
		     &phy_vars_ue->pucch_config_dedicated,
		     n1_pucch,
		     0,  // n2_pucch
		     1,  // shortened format
		     pucch_ack_payload,
		     scfdma_amps[1],
		     next_slot>>1);
		     
    }
  } // next_slot is even
}


void phy_procedures_UE_S_TX(u8 next_slot,PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag) {

  int aa, card_id;

  //  printf("S_TX: txdataF[0] %p\n",phy_vars_ue->lte_ue_common_vars.txdataF[0]);
  if (next_slot%2==1) {
    for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++){
      //  printf("Clearing TX buffer\n");
      if (abstraction_flag == 0) {
#ifdef IFFT_FPGA
	memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
	       0,(phy_vars_ue->lte_frame_parms.N_RB_DL*12)*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
#else
	memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][next_slot*phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)],
	       0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size*(phy_vars_ue->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
#endif
      }
    }

    if (phy_vars_ue->UE_mode[eNB_id] == PRACH) {

      if ((openair_daq_vars.timing_advance == TIMING_ADVANCE_INIT) ||
	  (openair_daq_vars.manual_timing_advance != 0)) {

#ifdef OPENAIR2
	if ((phy_vars_ue->RRCConnectionRequest_ptr[eNB_id] = mac_xface->ue_get_rach(phy_vars_ue->Mod_id,0))!=NULL) {
#endif
	  if (phy_vars_ue->prach_timer==0) {
	    phy_vars_ue->generate_prach=0;
	    // to be replaced with PRACH
	    if (abstraction_flag == 0) {
	      generate_pss(phy_vars_ue->lte_ue_common_vars.txdataF,
			   AMP,
			   &phy_vars_ue->lte_frame_parms,
			   0, //lte_ue_common_vars->eNB_id,
			   PRACH_SYMBOL,
			   next_slot);
	    }
	    else {
	      phy_vars_ue->generate_prach=1;
	      UE_transport_info[phy_vars_ue->Mod_id].cntl.prach_flag=1;
	      printf("[PHY][UE %d]generate_prach\n ", phy_vars_ue->Mod_id);

	    }
	    debug_msg("[PHY][UE %d] Frame %d, slot %d: Generating PRACH for UL, TX power %d dBm (PL %d dB)\n",
		      phy_vars_ue->Mod_id,mac_xface->frame,next_slot,
		      43-phy_vars_ue->PHY_measurements.rx_rssi_dBm[0]-114,
		      43-phy_vars_ue->PHY_measurements.rx_rssi_dBm[0]);
	    phy_vars_ue->prach_timer++;
	    if (phy_vars_ue->prach_timer==10)
	      phy_vars_ue->prach_timer=0;
	  }
	  debug_msg("[PHY][UE %d] Frame %d, slot %d: Generating PRACH for UL, TX power %d dBm (PL %d dB)\n",
	      phy_vars_ue->Mod_id,mac_xface->frame,next_slot,
	      43-phy_vars_ue->PHY_measurements.rx_rssi_dBm[0]-114,
	      43-phy_vars_ue->PHY_measurements.rx_rssi_dBm[0]);
#ifdef OPENAIR2
	}
#endif
      }
      else {
	openair_daq_vars.timing_advance = TIMING_ADVANCE_INIT;
	
#ifdef CBMIMO1
	for (card_id=0;card_id<number_of_cards;card_id++)
	  pci_interface[card_id]->timing_advance = openair_daq_vars.timing_advance;
#endif
      }
    }
    else {
      phy_vars_ue->generate_prach=0;
    }
  }
  //  printf("[PHY][UE] S_TX (%p): Energy %d dB\n",&phy_vars_ue->lte_ue_common_vars.txdataF[0][(next_slot*6*phy_vars_ue->lte_frame_parms.ofdm_symbol_size)],dB_fixed(signal_energy(&phy_vars_ue->lte_ue_common_vars.txdataF[0][(next_slot*6*phy_vars_ue->lte_frame_parms.ofdm_symbol_size)],phy_vars_ue->lte_frame_parms.samples_per_tti))); 
}
  
void lte_ue_measurement_procedures(u8 last_slot, u16 l, PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag) {
  

#ifdef EMOS
  u8 aa;

  // first slot in frame is special
  if (((last_slot==0) || (last_slot==1) || (last_slot==12) || (last_slot==13)) && 
      ((l==0) || (l==4-phy_vars_ue->lte_frame_parms.Ncp))) {
    for (eNB_id=0; eNB_id<3; eNB_id++) 
      for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++)
	lte_dl_channel_estimation_emos(emos_dump_UE.channel[eNB_id],
				       phy_vars_ue->lte_ue_common_vars->rxdataF,
				       &phy_vars_ue->lte_frame_parms,
				       last_slot,
				       aa,
				       l,
				       eNB_id);
  }
#endif

  if (l==0) {
    // UE measurements 
    if (abstraction_flag==0) {
      lte_ue_measurements(phy_vars_ue,
			  &phy_vars_ue->lte_frame_parms,
#ifndef USER_MODE
			  (last_slot>>1)*phy_vars_ue->lte_frame_parms.symbols_per_tti*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,
#else
			  (last_slot>>1)*phy_vars_ue->lte_frame_parms.symbols_per_tti*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size+phy_vars_ue->lte_frame_parms.nb_prefix_samples),
#endif
			  (last_slot == 2) ? 1 : 2,
			  1);
    }
    else {
      lte_ue_measurements_emul(phy_vars_ue,last_slot,eNB_id);
    }
#ifdef DEBUG_PHY    
    if (last_slot == 0) {
	
	debug_msg("[PHY][UE %d] frame %d, slot %d, freq_offset_filt = %d \n",phy_vars_ue->Mod_id,mac_xface->frame, last_slot, phy_vars_ue->lte_ue_common_vars.freq_offset);
	
	debug_msg("[PHY][UE %d] frame %d, slot %d, RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), RX gain %d dB\n",
		  phy_vars_ue->Mod_id,mac_xface->frame, last_slot,
		  phy_vars_ue->PHY_measurements.rx_rssi_dBm[0] - ((phy_vars_ue->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0), 
		phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][0],
		phy_vars_ue->PHY_measurements.wideband_cqi_dB[0][1],
		phy_vars_ue->PHY_measurements.wideband_cqi[0][0],
		phy_vars_ue->PHY_measurements.wideband_cqi[0][1],
		phy_vars_ue->rx_total_gain_dB);
      
      debug_msg("[PHY][UE %d] frame %d, slot %d, N0 %d dBm digital (%d, %d) dB, linear (%d, %d)\n",
		phy_vars_ue->Mod_id,mac_xface->frame, last_slot,
		dB_fixed(phy_vars_ue->PHY_measurements.n0_power_tot/phy_vars_ue->lte_frame_parms.nb_antennas_rx) - (int)phy_vars_ue->rx_total_gain_dB,
		phy_vars_ue->PHY_measurements.n0_power_dB[0],
		phy_vars_ue->PHY_measurements.n0_power_dB[1],
		phy_vars_ue->PHY_measurements.n0_power[0],
		phy_vars_ue->PHY_measurements.n0_power[1]);
    }
#endif
  }
  

  
  if ((last_slot==1) && (l==(4-phy_vars_ue->lte_frame_parms.Ncp))) {
    
    // AGC
    if (openair_daq_vars.rx_gain_mode == DAQ_AGC_ON)
      //      if (mac_xface->frame % 10 == 0)
      phy_adjust_gain (0,512,0,phy_vars_ue);
    
    eNB_id = 0;
    if (abstraction_flag == 0) 
      lte_adjust_synch(&phy_vars_ue->lte_frame_parms,
		       phy_vars_ue,
		       eNB_id,
		       0,
		       16384);

    //if (openair_daq_vars.auto_freq_correction == 1) 
    /*
      if (mac_xface->frame % 100 == 0) {
      if ((ue_common_vars->freq_offset>100) && (openair_daq_vars.freq_offset < 500)) {
      openair_daq_vars.freq_offset+=100;
      openair_set_freq_offset(0,penair_daq_vars.freq_offset);
      }
      else if ((ue_common_vars->freq_offset<-100) && (openair_daq_vars.freq_offset > -500)) {
      openair_daq_vars.freq_offset-=100;
      openair_set_freq_offset(0,penair_daq_vars.freq_offset);
      }
      }
    */
  }
}

#ifdef EMOS
void phy_procedures_emos_UE_RX(u8 last_slot,u8 eNB_id) {

  u8 i;
  memcpy(&emos_dump_UE.PHY_measurements[last_slot],&PHY_vars->PHY_measurements,sizeof(PHY_MEASUREMENTS));
  if (last_slot==0) {
    emos_dump_UE.timestamp = rt_get_time_ns();
    emos_dump_UE.frame_rx = mac_xface->frame;
    emos_dump_UE.UE_mode = phy_vars_ue->UE_mode;
    emos_dump_UE.freq_offset = lte_ue_common_vars->freq_offset;
    emos_dump_UE.timing_advance = openair_daq_vars.timing_advance;
    emos_dump_UE.timing_offset  = PHY_vars->rx_offset;
    emos_dump_UE.rx_total_gain_dB = PHY_vars->rx_total_gain_dB;
    emos_dump_UE.eNB_id = lte_ue_common_vars->eNB_id;
  }
  if (last_slot==1) {
    for (eNB_id = 0; eNB_id<3; eNB_id++) { 
      memcpy(emos_dump_UE.pbch_pdu[eNB_id],lte_ue_pbch_vars[eNB_id]->decoded_output,PBCH_PDU_SIZE);
      emos_dump_UE.pbch_errors[eNB_id] = lte_ue_pbch_vars[eNB_id]->pdu_errors;
      emos_dump_UE.pbch_errors_last[eNB_id] = lte_ue_pbch_vars[eNB_id]->pdu_errors_last;
      emos_dump_UE.pbch_errors_conseq[eNB_id] = lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq;
      emos_dump_UE.pbch_fer[eNB_id] = lte_ue_pbch_vars[eNB_id]->pdu_fer;
    }
  }
  if (last_slot%2==0) {
    for (i=0; i<2; i++) 
      memcpy(&emos_dump_UE.DCI_alloc[i][last_slot>>1], &dci_alloc_rx[i], sizeof(DCI_ALLOC_t));
  }
  if (last_slot==0) {
    eNB_id = lte_ue_common_vars->eNB_id;
    emos_dump_UE.dci_errors = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors;
    emos_dump_UE.dci_received = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received;
    emos_dump_UE.dci_false = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false;
    emos_dump_UE.dci_missed = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_missed;
    emos_dump_UE.dlsch_errors = dlsch_errors;
    emos_dump_UE.dlsch_errors_last = dlsch_errors_last;
    emos_dump_UE.dlsch_received = dlsch_received;
    emos_dump_UE.dlsch_received_last = dlsch_received_last;
    emos_dump_UE.dlsch_fer = dlsch_fer;
    emos_dump_UE.dlsch_cntl_errors = dlsch_cntl_errors;
    emos_dump_UE.dlsch_ra_errors = dlsch_ra_errors;
  }
  if (last_slot==0) {
    debug_msg("[PHY][UE %d] frame %d, slot %d, Writing EMOS data to FIFO\n",phy_vars_ue->Mod_id,mac_xface->frame, last_slot);
    if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_UE, sizeof(fifo_dump_emos_UE))!=sizeof(fifo_dump_emos_UE)) {
      debug_msg("[PHY][UE %d] frame %d, slot %d, Problem writing EMOS data to FIFO\n",phy_vars_ue->Mod_id,mac_xface->frame, last_slot);
      return;
    }
  }
}
#endif

void lte_ue_pbch_procedures(u8 eNB_id,u8 last_slot, PHY_VARS_UE *phy_vars_ue,u8 abstraction_flag) {

  int pbch_tx_ant;
  u8 pbch_phase;

  for (pbch_phase=0;pbch_phase<4;pbch_phase++) {
    //printf("Trying PBCH %d\n",pbch_phase);
    if (abstraction_flag == 0) {
      pbch_tx_ant = rx_pbch(&phy_vars_ue->lte_ue_common_vars,
			    phy_vars_ue->lte_ue_pbch_vars[eNB_id],
			    &phy_vars_ue->lte_frame_parms,
			    eNB_id,
			    phy_vars_ue->lte_frame_parms.mode1_flag==1?SISO:ALAMOUTI,
			    pbch_phase);



    }
    else
      pbch_tx_ant = rx_pbch_emul(phy_vars_ue,
				 eNB_id,
				 pbch_phase);

    //printf("Received %d TX antennas\n",pbch_tx_ant);
    if ((pbch_tx_ant>0) && (pbch_tx_ant<=4))
      break;
  }

  if ((pbch_tx_ant>0) && (pbch_tx_ant<=4)) {
    mac_xface->frame = 	((phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[0]&3)<<6) | (((phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1]>>2))<<2);
    mac_xface->frame += pbch_phase;
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq = 0;
#ifdef EMOS
    emos_dump_UE.frame_tx = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[0];
    emos_dump_UE.mimo_mode = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1];
    //PHY_vars->PHY_measurements.frame_tx = *((unsigned int*) lte_ue_pbch_vars->decoded_output);
#endif
    //phy_vars_ue->lte_frame_parms.mode1_flag = (phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1] == 1);
    //openair_daq_vars.dlsch_transmission_mode = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1];
  }
  else {
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq++;
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors++;
  }
  
  if (mac_xface->frame % 100 == 0) {
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_fer = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors - phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_last;
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_last = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors;
  }
  
  
  debug_msg("[PHY][UE %d] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
	    phy_vars_ue->Mod_id,mac_xface->frame, last_slot, 
	    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors, 
	    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq);
  debug_msg("[PHY][UE %d] frame %d, slot %d, PBCH received frame=%d, transmission mode=%d!\n",
	    phy_vars_ue->Mod_id,mac_xface->frame, last_slot,
	    (phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[0]<<2)+pbch_phase,
	    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1]);
  
  
  if (phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq>200) {
    msg("[PHY][UE %d] frame %d, slot %d, PBCH consecutive errors > 200, going out of sync!\n",phy_vars_ue->Mod_id,mac_xface->frame, last_slot);
    openair_daq_vars.mode = openair_NOT_SYNCHED;
    phy_vars_ue->UE_mode[eNB_id] = NOT_SYNCHED;
    openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
    openair_dma(0,FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1
    mac_xface->frame = -1;
    openair_daq_vars.synch_wait_cnt=0;
    openair_daq_vars.sched_cnt=-1;
    
    
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq=0;
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors=0;
    
    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors = 0;
    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_missed = 0;
    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false  = 0;    
    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received = 0;    
    
    phy_vars_ue->dlsch_errors[eNB_id] = 0;
    phy_vars_ue->dlsch_errors_last[eNB_id] = 0;
    phy_vars_ue->dlsch_received[eNB_id] = 0;
    phy_vars_ue->dlsch_received_last[eNB_id] = 0;
    phy_vars_ue->dlsch_fer[eNB_id] = 0;
    phy_vars_ue->dlsch_SI_errors[eNB_id] = 0;
    phy_vars_ue->dlsch_ra_errors[eNB_id] = 0;
    
  }
}

int lte_ue_pdcch_procedures(u8 eNB_id,u8 last_slot, PHY_VARS_UE *phy_vars_ue,u8 abstraction_flag) {	

  unsigned int dci_cnt, i;
  //DCI_PDU *DCI_pdu;

#ifdef DEBUG_PHY
  debug_msg("[PHY][UE %d] Frame %d, slot %d (%d): DCI decodingm crnti %x\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot,last_slot>>1,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti);
#endif
  
  //  write_output("UE_rxsigF0.m","UE_rxsF0", lte_ue_common_vars->rxdataF[0],512*12*2,2,1);
  //  write_output("UE_rxsigF1.m","UE_rxsF1", lte_ue_common_vars->rxdataF[1],512*12*2,2,1);


  //phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols = 3;


  if (abstraction_flag == 0)  {
    rx_pdcch(&phy_vars_ue->lte_ue_common_vars,
	     phy_vars_ue->lte_ue_pdcch_vars,
	     &phy_vars_ue->lte_frame_parms,
	     last_slot>>1,
	     eNB_id,
	     (phy_vars_ue->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
	     phy_vars_ue->is_secondary_ue); 
    dci_cnt = dci_decoding_procedure(phy_vars_ue->lte_ue_pdcch_vars,
				     dci_alloc_rx,
				     eNB_id,
				     &phy_vars_ue->lte_frame_parms,
				     get_mi(&phy_vars_ue->lte_frame_parms,0),
				     SI_RNTI,RA_RNTI);
  }

  else {
    dci_cnt = dci_decoding_procedure_emul(phy_vars_ue->lte_ue_pdcch_vars,
					  PHY_vars_eNB_g[eNB_id]->num_ue_spec_dci[(last_slot>>1)&1],
					  PHY_vars_eNB_g[eNB_id]->num_common_dci[(last_slot>>1)&1],
					  PHY_vars_eNB_g[eNB_id]->dci_alloc[(last_slot>>1)&1],
					  dci_alloc_rx,
					  eNB_id);
  }

  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received += dci_cnt;
  
  //#ifdef DEBUG_PHY
  debug_msg("[PHY][UE %d] Frame %d, slot %d, Mode %s: DCI found %i\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot,mode_string[phy_vars_ue->UE_mode[eNB_id]],dci_cnt);
  //#endif

  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received += dci_cnt;
  
  //#ifdef DIAG_PHY
  if (last_slot==18)
    debug_msg("[PHY][UE %d][DIAG] Frame %d, slot %d: PDCCH: DCI errors %d, DCI received %d, DCI missed %d, DCI False Detection %d \n",
	      phy_vars_ue->Mod_id,mac_xface->frame,last_slot,
	      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors,
	      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received,
	      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_missed,
	      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false);
  //#endif // DIAG_PHY
  
#ifdef EMOS
  emos_dump_UE.dci_cnt[last_slot>>1] = dci_cnt;
#endif

#ifdef DIAG_PHY
  if (phy_vars_ue->UE_mode[eNB_id] == PUSCH)
    if (dci_cnt > 2) {
      msg("[PHY][UE %d][DIAG] frame %d, subframe %d: received %d>2 DCI!\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot>>1,dci_cnt);
      return(-1);
    }
#endif

  for (i=0;i<dci_cnt;i++){

    if ((phy_vars_ue->UE_mode[eNB_id] != PRACH) && 
	(dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti) &&
	(dci_alloc_rx[i].format != format0)) {
      //#ifdef DEBUG_PHY
      debug_msg("[PHY][UE %d] frame %d, subframe %d: Found rnti %x, format %d\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot>>1,dci_alloc_rx[i].rnti,
	  dci_alloc_rx[i].format);
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 20))
	dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      //#endif      
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode[eNB_id] == PUSCH) && ((last_slot>>1) != 6)) {
	msg("[PHY][UE %d][DIAG] frame %d, subframe %d: should not have received C_RNTI!\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
	return(-1);
	
      }
#endif
      
      
      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (void *)&dci_alloc_rx[i].dci_pdu,
					    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
					    dci_alloc_rx[i].format,
					    phy_vars_ue->dlsch_ue[eNB_id],
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {

	phy_vars_ue->dlsch_received[eNB_id]++;
	//#ifdef DEBUG_PHY
	debug_msg("[PHY][UE %d] Generated UE DLSCH C_RNTI format %d\n",phy_vars_ue->Mod_id,dci_alloc_rx[i].format);
	//#endif    
	  
	// we received a CRNTI, so we're in PUSCH
	phy_vars_ue->UE_mode[eNB_id] = PUSCH;
	/*
#ifdef DEBUG_PHY
	msg("[PHY][UE] frame %d, subframe %d: Mode PUSCH.\n",mac_xface->frame,last_slot>>1);
#ifdef USER_MODE
	exit(-1);
#else
	exit_openair=1;
#endif
#endif
	*/
      }
    }

    else if ((dci_alloc_rx[i].rnti == SI_RNTI) && (dci_alloc_rx[i].format == format1A)) {
#ifdef DEBUG_PHY
      debug_msg("[PHY][UE %d] subframe %d: Found rnti %x, format 1A, dci_cnt %d\n",phy_vars_ue->Mod_id,last_slot>>1,dci_alloc_rx[i].rnti,i);
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 20))
	dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
#endif      
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode[eNB_id] == PUSCH) && ((last_slot>>1) != 0)) {
	msg("[PHY][UE %d][DIAG] frame %d, subframe %d: should not have received SI_RNTI!\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
	return(-1);
      }
#endif
      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (void *)&dci_alloc_rx[i].dci_pdu,
					    SI_RNTI,
					    format1A,
					    &phy_vars_ue->dlsch_ue_SI[eNB_id], 
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {

#ifdef DEBUG_PHY
	debug_msg("[PHY][UE %d] Generate UE DLSCH SI_RNTI format 1A\n",phy_vars_ue->Mod_id);
#endif
      }
    }

    else if ((dci_alloc_rx[i].rnti == RA_RNTI) && (dci_alloc_rx[i].format == format1A)) {
#ifdef DEBUG_PHY
      debug_msg("[PHY][UE %d] subframe %d: Found rnti %x, format 1A, dci_cnt %d\n",phy_vars_ue->Mod_id,last_slot>>1,dci_alloc_rx[i].rnti,i);
#endif      
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode[eNB_id] == PUSCH) && ((last_slot>>1) != 7)) {
	msg("[PHY][UE %d][DIAG] frame %d, subframe %d: should not have received RA_RNTI!\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
	return(-1);
      }
#endif

      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					    RA_RNTI,
					    format1A,
					    &phy_vars_ue->dlsch_ue_ra[eNB_id], 
					    &phy_vars_ue->lte_frame_parms,
					    SI_RNTI,
					    RA_RNTI,
					    P_RNTI)==0) {
#ifdef DEBUG_PHY
	debug_msg("[PHY][UE %d] Generate UE DLSCH RA_RNTI format 1A, rb_alloc %x, dlsch_ue_ra[eNB_id] %p\n",
	    phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue_ra[eNB_id]->rb_alloc[0],phy_vars_ue->dlsch_ue_ra[eNB_id]);
#endif
      }
    }

    else if ((phy_vars_ue->UE_mode[eNB_id] != PRACH) && 
	     (dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti) && 
	     (dci_alloc_rx[i].format == format0)) {
#ifdef DIAG_PHY
      if ((phy_vars_ue->UE_mode[eNB_id] == PUSCH) && ((last_slot>>1) != 9)) {
	msg("[PHY][UE %d][DIAG] frame %d, subframe %d: should not have received C_RNTI!\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
	return(-1);
      }
#endif

      phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;

      generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
					last_slot>>1,
					format0,
					phy_vars_ue->ulsch_ue[eNB_id],
					phy_vars_ue->dlsch_ue[eNB_id],
					&phy_vars_ue->PHY_measurements,
					&phy_vars_ue->lte_frame_parms,
					SI_RNTI,
					RA_RNTI,
					P_RNTI,
					eNB_id,
                                        phy_vars_ue->current_dlsch_cqi[eNB_id],
					0);  // generate_srs
#ifdef DEBUG_PHY
      debug_msg("[PHY][UE %d] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",phy_vars_ue->Mod_id,last_slot>>1);
#endif
    }

    else if ((dci_alloc_rx[i].rnti == RA_RNTI) && (dci_alloc_rx[i].format == format0)) {
#ifdef DEBUG_PHY
      debug_msg("[PHY][UE %d] subframe %d: Found rnti %x, format 0, dci_cnt %d\n",phy_vars_ue->Mod_id,last_slot>>1,dci_alloc_rx[i].rnti,i);
#endif      
#ifdef DIAG_PHY
      if (phy_vars_ue->UE_mode[eNB_id] == PUSCH) {
	msg("[PHY][UE %d][DIAG] frame %d, subframe %d: should not have received RA_RNTI!\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
	return(-1);
      }
#endif
      /*
	generate_ue_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
	RA_RNTI,
	last_slot>>1,
	format0,
	ulsch_ue[eNB_id], 
	lte_frame_parms,
	SI_RNTI,
	RA_RNTI,
	P_RNTI);

	printf("[PHY][UE %d] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",last_slot>>1);
      */
    }
    else {
      msg("[PHY][UE %d] frame %d, subframe %d: received DCI with RNTI=%x (%x)  and format %d!\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot>>1,dci_alloc_rx[i].rnti,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,dci_alloc_rx[i].format);
#ifdef DIAG_PHY
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
      return(-1);
#endif
    }

  }
  return(0);
}


int phy_procedures_UE_RX(u8 last_slot, PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag) {

  u16 l,m,n_symb,i,j;
  //  int eNB_id = 0, eNB_id_i = 1;
  u8 dual_stream_UE = 0;
  int ret;
  u8 harq_pid;
  int timing_advance;
  unsigned int dlsch_buffer_length;
  u8 pilot1,pilot2,pilot3;


  if (phy_vars_ue->lte_frame_parms.Ncp == 0) {  // normal prefix
    pilot1 = 4;
    pilot2 = 7;
    pilot3 = 11;
  }
  else {  // extended prefix
    pilot1 = 3;
    pilot2 = 6;
    pilot3 = 9;
  }

  if (subframe_select(&phy_vars_ue->lte_frame_parms,last_slot>>1) == SF_S) 
    if ((last_slot%2)==0)
      n_symb = 3;
    else
      n_symb = 0;
  else
    n_symb = phy_vars_ue->lte_frame_parms.symbols_per_tti/2;

  // RX processing of symbols in last_slot
  for (l=0;l<n_symb;l++) {

    if (abstraction_flag == 0) {
      slot_fep(&phy_vars_ue->lte_frame_parms,
	       &phy_vars_ue->lte_ue_common_vars,
	       l,
	       last_slot,
	       0,
#ifdef USER_MODE
	       0
#else
	       1
#endif
	       );
    }

    lte_ue_measurement_procedures(last_slot,l,phy_vars_ue,eNB_id,abstraction_flag);


    if ((last_slot==1) && (l==4-phy_vars_ue->lte_frame_parms.Ncp)) {

      phy_vars_ue->ulsch_no_allocation_counter[eNB_id]++;

      if (phy_vars_ue->ulsch_no_allocation_counter[eNB_id] == 10) {
	printf("[UE %d] no_allocation : setting mode to PRACH\n",phy_vars_ue->Mod_id);
	phy_vars_ue->UE_mode[eNB_id] = PRACH;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti = 0x1234;
      }
       
      lte_ue_pbch_procedures(eNB_id,last_slot,phy_vars_ue,abstraction_flag);

      if (phy_vars_ue->UE_mode[eNB_id] == RA_RESPONSE) {
	phy_vars_ue->RRCConnReq_timer[eNB_id]--;
	//	msg("[UE RAR] frame %d: RRCConnReq_timer %d\n",mac_xface->frame,phy_vars_ue->RRCConnReq_timer);

	if (phy_vars_ue->RRCConnReq_timer[eNB_id] == 0) {
	  printf("[UE %d] RRCConnReq_timer = 0 : setting mode to PRACH\n",phy_vars_ue->Mod_id);
	  phy_vars_ue->UE_mode[eNB_id] = PRACH;
	  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti = 0x1234;
	}
      }
    }

    // process last DLSCH symbols + invoke decoding
    if (((last_slot%2)==0) && (l==0)) {
      //      printf("phy_procedures: lte_ue_dlsch_vars %p lte_ue_dlsch_vars[0] %p\n",lte_ue_dlsch_vars,lte_ue_dlsch_vars[0]);

      //      if ( (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1) && (phy_vars_ue->dlsch_ue_SI[eNB_id]->active == 1))
      //	msg("[PHY][UE %d] WARNING: dlsch_ue and dlsch_ue_SI active, but data structures can only handle one at a time\n",phy_vars_ue->Mod_id);

      if (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1) {
	//#ifdef DEBUG_PHY

	//#endif      
	// process symbols 10,11,12 and trigger DLSCH decoding
	if (abstraction_flag == 0) {
	  for (m=pilot3;m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++) {

	    rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		     phy_vars_ue->lte_ue_dlsch_vars,
		     &phy_vars_ue->lte_frame_parms,
		     eNB_id,
		     eNB_id+1,
		     phy_vars_ue->dlsch_ue[eNB_id],
		     ((last_slot>>1)-1)%10,  // subframe
		     m,                    // symbol
		     0,                    // first_symbol_flag
		     dual_stream_UE,
		     &phy_vars_ue->PHY_measurements,
		     phy_vars_ue->is_secondary_ue);
	  }
	}
	
	phy_vars_ue->dlsch_ue[eNB_id][0]->active = 0;
	harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid;
	debug_msg("[PHY][UE %d] Frame %d, slot %d: Scheduling DLSCH decoding\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot);      
#ifdef DLSCH_THREAD //USER_MODE


	if (harq_pid != 0) {
	  msg("[PHY][UE %d] Frame %d, slot %d: No DLSCH decoding thread for harq_pid = %d\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot,harq_pid);
	  return(-1);
	}
	
	if (pthread_mutex_lock (&dlsch_mutex[harq_pid]) != 0) {               // Signal MAC_PHY Scheduler
	  msg("[PHY][UE %d] ERROR pthread_mutex_lock\n",phy_vars_ue->Mod_id);     // lock before accessing shared resource
	  return(-1);
	}
	dlsch_instance_cnt[harq_pid]++;
	dlsch_subframe[harq_pid] = ((last_slot>>1)-1)%10;
	pthread_mutex_unlock (&dlsch_mutex[harq_pid]);
	
	if (dlsch_instance_cnt[harq_pid] == 0) {
	  if (pthread_cond_signal(&dlsch_cond[harq_pid]) != 0) {
	    msg("[PHY][UE %d] ERROR pthread_cond_signal for dlsch_cond[%d]\n",phy_vars_ue->Mod_id,harq_pid);
	    return(-1);
	  }
	}
	else {
	  msg("[PHY][UE %d] DLSCH thread for harq_pid %d busy!!!\n",phy_vars_ue->Mod_id,harq_pid);
	  return(-1);
	}
	
#else
	if (phy_vars_ue->dlsch_ue[eNB_id][0]) {
	  if (abstraction_flag == 0) {
	    dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
			       phy_vars_ue->dlsch_ue[0][0],
			       get_G(&phy_vars_ue->lte_frame_parms,
				     phy_vars_ue->dlsch_ue[eNB_id][0]->nb_rb,
				     phy_vars_ue->dlsch_ue[eNB_id][0]->rb_alloc,
				     get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs),
				     phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,((last_slot>>1)-1)%10),
			       phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->llr[0],
			       0,
			       (((last_slot>>1)-1)%10)<<1);
	    debug_msg("[PHY][UE %d] Calling dlsch_decoding for subframe %d\n",phy_vars_ue->Mod_id,((last_slot>>1)-1)%10);
	    ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->llr[0],
				 &phy_vars_ue->lte_frame_parms,
				 phy_vars_ue->dlsch_ue[eNB_id][0],
				 ((last_slot>>1)-1)%10,
				 phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
	  }

	  else {
	    ret = dlsch_decoding_emul(phy_vars_ue,
				      ((last_slot>>1)-1)%10,
				      2,
				      eNB_id);
	  }

	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    phy_vars_ue->dlsch_errors[eNB_id]++;
#ifdef USER_MODE
#ifdef DEBUG_PHY
	    if (mac_xface->frame > 10) {
	      printf("[PHY][UE %d] Frame %d, subframe %d : DLSCH (rv %d,mcs %d) in error\n",phy_vars_ue->Mod_id,
		     mac_xface->frame,last_slot>>1,phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
		     phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
	      //	      dump_dlsch(phy_vars_ue,((last_slot>>1)-1)%10);
	      //	      exit(-1);
	    }
#endif
#endif
	  } 
	  else {
#ifdef USER_MODE
#ifdef DEBUG_PHY	    	    
	    printf("dlsch harq_pid %d (rx): \n",phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid);
	    for (j=0;j<phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->TBS>>3;j++)
	      printf("%x.",phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->b[j]);
	    printf("\n");
#endif 
#endif	    
#ifdef OPENAIR2
	    mac_xface->ue_send_sdu(phy_vars_ue->Mod_id,
				   phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->b,
				   0);
#endif
	  }
	}

	if (mac_xface->frame % 200 == 0) {
	  if ((phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]) != 0) 
	    phy_vars_ue->dlsch_fer[eNB_id] = (200*(phy_vars_ue->dlsch_errors[eNB_id] - phy_vars_ue->dlsch_errors_last[eNB_id]))/(phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]);
	  phy_vars_ue->dlsch_errors_last[eNB_id] = phy_vars_ue->dlsch_errors[eNB_id];
	  phy_vars_ue->dlsch_received_last[eNB_id] = phy_vars_ue->dlsch_received[eNB_id];
	  
	  // CQI adaptation when current MCS is odd, even is handled by eNB
	  if ((phy_vars_ue->dlsch_fer[eNB_id] > 40) && (phy_vars_ue->current_dlsch_cqi[eNB_id]>0) && ((phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->mcs%2) == 0))
	    phy_vars_ue->current_dlsch_cqi[eNB_id]--;
#ifndef USER_MODE
	  if ((phy_vars_ue->dlsch_fer[eNB_id] < 16) && (phy_vars_ue->current_dlsch_cqi[eNB_id]<8) && ((phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->mcs%2) == 1))
	    phy_vars_ue->current_dlsch_cqi[eNB_id]++;
#else
	  if ((phy_vars_ue->dlsch_fer[eNB_id] < 16) && (phy_vars_ue->current_dlsch_cqi[eNB_id]<14) && ((phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->mcs%2) == 1))
	    phy_vars_ue->current_dlsch_cqi[eNB_id]++;
#endif
	  
	}

	debug_msg("[PHY][UE %d] Frame %d, slot %d: dlsch_decoding ret %d (mcs %d, TBS %d)\n",
		  phy_vars_ue->Mod_id,mac_xface->frame,last_slot,ret,
		  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->mcs,
		  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[0]->TBS);
	debug_msg("[PHY][UE %d] Frame %d, slot %d: dlsch_errors %d, dlsch_received %d, dlsch_fer %d, current_dlsch_cqi %d\n",
		  phy_vars_ue->Mod_id,mac_xface->frame,last_slot,
		  phy_vars_ue->dlsch_errors[eNB_id],
		  phy_vars_ue->dlsch_received[eNB_id],
		  phy_vars_ue->dlsch_fer[eNB_id],
		  phy_vars_ue->current_dlsch_cqi[eNB_id]);

#endif
      }
      else {
	phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[((last_slot>>1)-1)%10].send_harq_status = 0;
      }
      
      if (phy_vars_ue->dlsch_ue_SI[eNB_id]->active == 1) {
#ifdef DEBUG_PHY
	//debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH (SI) demod symbols 10,11,12\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot);
#endif
	
	// process symbols 10,11,12 and trigger DLSCH decoding
	if (abstraction_flag==0) {
	  for (m=pilot3;m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++) {
	    debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH (SI) demod between pilot 3 and 4 (2nd slot), m %d\n",
		      phy_vars_ue->Mod_id,mac_xface->frame,last_slot,m);
	    rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		     phy_vars_ue->lte_ue_dlsch_vars_SI,
		     &phy_vars_ue->lte_frame_parms,
		     eNB_id,
		     eNB_id+1,
		     &phy_vars_ue->dlsch_ue_SI[eNB_id],
		     last_slot>>1,  // subframe,
		     m,
		     0,
		     dual_stream_UE,
		     &phy_vars_ue->PHY_measurements,
		     phy_vars_ue->is_secondary_ue);
	  }
	}
	//	write_output("dlsch_ra_llr.m","llr",lte_ue_dlsch_vars_ra[eNB_id]->llr[0],40,1,0);

	phy_vars_ue->dlsch_ue_SI[eNB_id]->active = 0;
      
	if (mac_xface->frame < phy_vars_ue->dlsch_SI_errors[eNB_id])
	  phy_vars_ue->dlsch_SI_errors[eNB_id]=0;

	if (phy_vars_ue->dlsch_ue_SI[eNB_id]) {

	  if (abstraction_flag==0) {

	    debug_msg("Decoding DLSCH_SI : rb_alloc %x : nb_rb %d G %d\n",phy_vars_ue->dlsch_ue_SI[eNB_id]->rb_alloc[0],
		phy_vars_ue->dlsch_ue_SI[eNB_id]->nb_rb,
		get_G(&phy_vars_ue->lte_frame_parms,
		      phy_vars_ue->dlsch_ue_SI[eNB_id]->nb_rb,
		      phy_vars_ue->dlsch_ue_SI[eNB_id]->rb_alloc,
		      get_Qm(phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs),
		      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,((last_slot>>1)-1)%10)
		);
	    dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
			       phy_vars_ue->dlsch_ue_SI[eNB_id],
			       get_G(&phy_vars_ue->lte_frame_parms,
				     phy_vars_ue->dlsch_ue_SI[eNB_id]->nb_rb,
				     phy_vars_ue->dlsch_ue_SI[eNB_id]->rb_alloc,
				     get_Qm(phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs),
				     phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,((last_slot>>1)-1)%10),
			       phy_vars_ue->lte_ue_dlsch_vars_SI[eNB_id]->llr[0],
			       0,
			       (((last_slot>>1)-1)%10)<<1);
	    
	    ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars_SI[eNB_id]->llr[0],
				 &phy_vars_ue->lte_frame_parms,
				 phy_vars_ue->dlsch_ue_SI[eNB_id],
				 ((last_slot>>1)-1)%10,
				 phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);

#ifdef DEBUG_PHY
      for (i=0;i<11;i++)
	debug_msg("dlsch_output_buffer[%d]=%x\n",i,phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->c[0][i]);
#endif

	  }

	  else {
	    ret = dlsch_decoding_emul(phy_vars_ue,
				      ((last_slot>>1)-1)%10,
				      0,
				      eNB_id);
	  }

	  if (ret == (1+MAX_TURBO_ITERATIONS)) {
	    phy_vars_ue->dlsch_SI_errors[eNB_id]++;
	    msg("[PHY][UE %d] Frame %d, subframe %d, received SI in error\n",phy_vars_ue->Mod_id,mac_xface->frame,((last_slot>>1)-1)%10);
#ifdef USER_MODE
	    dump_dlsch_SI(phy_vars_ue,eNB_id,((last_slot>>1)-1)%10);
#endif
	    return(-1);
	  }
	  else {

	    //timing_advance = process_rar(dlsch_ue_SI->harq_processes[0]->b,&dummy);
	    timing_advance = 0;
	    process_timing_advance(timing_advance);

	    ///	    debug_msg("[PHY][UE %d] Frame %d, subframe %d, received (from SI) timing_advance = %d (%d)\n",mac_xface->frame,((last_slot>>1)-1)%10, timing_advance, openair_daq_vars.timing_advance);
	    dlsch_buffer_length = phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS/8;
	    debug_msg("[PHY][UE %d] Frame %d, subframe %d, received SI\n",phy_vars_ue->Mod_id,mac_xface->frame,((last_slot>>1)-1)%10);

#ifdef OPENAIR2
	    mac_xface->ue_decode_si(phy_vars_ue->Mod_id,
				    0,
				    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->b,
				    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS>>3);
	    
#endif
	  }
	}   
	
	debug_msg("[PHY][UE %d] Frame %d, slot %d: dlsch_decoding (SI) ret %d (%d errors)\n",
		  phy_vars_ue->Mod_id,mac_xface->frame,last_slot,ret,phy_vars_ue->dlsch_SI_errors[eNB_id]);

      }
    

      if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active == 1) {
	//#ifdef DEBUG_PHY
	debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH (RA) demod symbols 10,11,12\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot);
	//#endif
      
	// process symbols 10,11,12 and trigger DLSCH decoding
	if (abstraction_flag==0) {
	  for (m=pilot3;m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++)
	    rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		     phy_vars_ue->lte_ue_dlsch_vars_ra,
		     &phy_vars_ue->lte_frame_parms,
		     eNB_id,
		     eNB_id+1,
		     &phy_vars_ue->dlsch_ue_ra[eNB_id],
		     last_slot>>1,  // subframe,
		     m, // symbol
		     0, // first_symbol_flag
		     dual_stream_UE,
		     &phy_vars_ue->PHY_measurements,
		     phy_vars_ue->is_secondary_ue);
	}

	phy_vars_ue->dlsch_ue_ra[eNB_id]->active = 0;
	
	if (mac_xface->frame < phy_vars_ue->dlsch_ra_errors[eNB_id])
	  phy_vars_ue->dlsch_ra_errors[eNB_id]=0;

	phy_vars_ue->dlsch_ue_ra[eNB_id]->rnti = RA_RNTI;

	//	printf("[PHY][UE] Calling dlsch_decoding (RA) for subframe %d\n",((last_slot>>1)-1)%10);
	if (abstraction_flag==0) {
	  dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
			     phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
			     phy_vars_ue->dlsch_ue_ra[eNB_id],
			     get_G(&phy_vars_ue->lte_frame_parms,
				   phy_vars_ue->dlsch_ue_ra[eNB_id]->nb_rb,
				   phy_vars_ue->dlsch_ue_ra[eNB_id]->rb_alloc,
				   get_Qm(phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->mcs),
				   phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,((last_slot>>1)-1)%10),
			     phy_vars_ue->lte_ue_dlsch_vars_ra[eNB_id]->llr[0],
			     0,
			     (((last_slot>>1)-1)%10)<<1);
	  ret = dlsch_decoding(phy_vars_ue->lte_ue_dlsch_vars_ra[eNB_id]->llr[0],
			       &phy_vars_ue->lte_frame_parms,
			       phy_vars_ue->dlsch_ue_ra[eNB_id],
			       ((last_slot>>1)-1)%10,  // subframe
			       phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
#ifdef DEBUG_PHY
      for (i=0;i<11;i++)
	debug_msg("RA : dlsch_output_buffer[%d]=%x\n",i,phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->c[0][i]);
#endif
	}

	else {
	  ret = dlsch_decoding_emul(phy_vars_ue,
				    ((last_slot>>1)-1)%10,
				    1,
				    eNB_id);
	}

	if (ret == (1+MAX_TURBO_ITERATIONS)) {
	  phy_vars_ue->dlsch_ra_errors[eNB_id]++;
#ifdef USER_MODE
	  dump_dlsch_ra(phy_vars_ue,eNB_id,((last_slot>>1)-1)%10);
#endif
	  return(-1);

	}
	else {
	  debug_msg("[PHY][UE %d] Received RAR in frame %d, subframe %d mode %d\n",phy_vars_ue->Mod_id,mac_xface->frame,((last_slot>>1)-1)%10, phy_vars_ue->UE_mode[eNB_id]);
	  
	  if (phy_vars_ue->UE_mode[eNB_id] != PUSCH) {
#ifdef OPENAIR2
	    timing_advance = mac_xface->ue_process_rar(phy_vars_ue->Mod_id,
						       phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b,
						       &phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti);
	    
	    timing_advance = 0;
	    process_timing_advance_rar(phy_vars_ue,timing_advance);
#endif
	    
	    phy_vars_ue->ulsch_ue_RRCConnReq_active[eNB_id]=1;
	    get_RRCConnReq_alloc(&phy_vars_ue->lte_frame_parms,
				 ((last_slot>>1)-1)%10, //(last_slot==0) ? 9 : ((last_slot>>1)-1),
				 mac_xface->frame,
				 &phy_vars_ue->ulsch_ue_RRCConnReq_frame[eNB_id],
				 &phy_vars_ue->ulsch_ue_RRCConnReq_subframe[eNB_id]);
#ifdef DEBUG_PHY
	    debug_msg("Got RRCConnReq_alloc mac frame %d subframe %d: RRCConnReq_frame %d, RRCConnReq_subframe %d\n",
		   mac_xface->frame,
		   last_slot>>1,
		   phy_vars_ue->ulsch_ue_RRCConnReq_frame[eNB_id],
		   phy_vars_ue->ulsch_ue_RRCConnReq_subframe[eNB_id]);
#endif
	    phy_vars_ue->UE_mode[eNB_id] = RA_RESPONSE;
	    phy_vars_ue->RRCConnReq_timer[eNB_id] = 10;
	    phy_vars_ue->ulsch_ue[eNB_id]->power_offset = 6;
	    phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;
	  } // mode != PUSCH
	} //ret <= MAX_ITERATIONS
	
	
	debug_msg("[PHY][UE %d] Frame %d, slot %d: dlsch_decoding (RA) ret %d (%d errors)\n",
		  phy_vars_ue->Mod_id,mac_xface->frame,last_slot,ret,phy_vars_ue->dlsch_ra_errors[eNB_id]);
	
      } // dlsch_ue_ra[eNB_id]->active == 1
      
    }

    if (((last_slot%2)==0) && (l==pilot1))  {
#ifdef DEBUG_PHY
      msg("[PHY][UE %d] Frame %d, slot %d: Calling pdcch procedures\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot);
#endif
      if (lte_ue_pdcch_procedures(eNB_id,last_slot,phy_vars_ue,abstraction_flag) == -1) {
	msg("[PHY][UE %d] Frame %d, slot %d: Error in pdcch procedures\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot);
	return(-1);
	//exit_openair=1;
      }
      //printf("num_pdcch_symbols %d\n",phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
    }

    if (abstraction_flag==0) {

      if (((last_slot%2)==1) && (l==0)) {
	
	for (m=phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols;
	     m<pilot2;
	     m++) {      
	  
	  if (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1)  {
	    //#ifdef DEBUG_PHY
	  debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH demod first slot (m %d)\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot,m);
	  //#endif
	  
	  // process DLSCH received in first slot
	  
	  rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		   phy_vars_ue->lte_ue_dlsch_vars,
		   &phy_vars_ue->lte_frame_parms,
		   eNB_id,
		   eNB_id+1,
		   phy_vars_ue->dlsch_ue[eNB_id],
		   last_slot>>1,  // subframe,
		   m,
		   (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
		   dual_stream_UE,
                   &phy_vars_ue->PHY_measurements,
                   phy_vars_ue->is_secondary_ue);
	  } // CRNTI active
	  
	  if (phy_vars_ue->dlsch_ue_SI[eNB_id]->active == 1)  {
#ifdef DEBUG_PHY
	    debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH (SI) demod first slot, m %d\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot,m);
#endif
	      
	    // process SI DLSCH in first slot
	    rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		     phy_vars_ue->lte_ue_dlsch_vars_SI,
		     &phy_vars_ue->lte_frame_parms,
		     eNB_id,
		     eNB_id+1,
		     &phy_vars_ue->dlsch_ue_SI[eNB_id],
		     last_slot>>1,  // subframe,
		     m,
		     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
		     dual_stream_UE,
		     &phy_vars_ue->PHY_measurements,
		     phy_vars_ue->is_secondary_ue);
	  } // SI active
	  
	  if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active == 1)  {
#ifdef DEBUG_PHY
	    debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH (RA) demod in first slot\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot);
#endif
	    
	    rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		     phy_vars_ue->lte_ue_dlsch_vars_ra,
		     &phy_vars_ue->lte_frame_parms,
		     eNB_id,
		     eNB_id+1,
		     &phy_vars_ue->dlsch_ue_ra[eNB_id],
		     last_slot>>1,  // subframe,
		     m,
		     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,
		     dual_stream_UE,
		     &phy_vars_ue->PHY_measurements,
		     phy_vars_ue->is_secondary_ue);
	  } // RA active
	} // loop from first dlsch symbol to end of slot
      } // 2nd quarter

      if (((last_slot%2)==1) && (l==pilot1)) {
	for (m=pilot2;m<pilot3;m++) {
	  
	  if (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1) {
#ifdef DEBUG_PHY
	    debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH demod between pilot 2 and 3 (2nd slot)\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot);
#endif
	    rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		     phy_vars_ue->lte_ue_dlsch_vars,
		     &phy_vars_ue->lte_frame_parms,
		     eNB_id,
		     eNB_id+1,
		     phy_vars_ue->dlsch_ue[eNB_id],
		     last_slot>>1,  // subframe,
		     m,
		     0,
		     dual_stream_UE,
		     &phy_vars_ue->PHY_measurements,
		     phy_vars_ue->is_secondary_ue);
	  } // CRNTI active
	  
	  if(phy_vars_ue->dlsch_ue_SI[eNB_id]->active == 1) {
#ifdef DEBUG_PHY
	    debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH (SI) demod between pilot 2 and 3 (2nd slot), m %d\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot,m);
#endif
	      rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		       phy_vars_ue->lte_ue_dlsch_vars_SI,
		       &phy_vars_ue->lte_frame_parms,
		       eNB_id,
		       eNB_id+1,
		       &phy_vars_ue->dlsch_ue_SI[eNB_id],
		       last_slot>>1,  // subframe,
		       m,
		       0,   // first_symbol_flag
		       dual_stream_UE,
		       &phy_vars_ue->PHY_measurements,
		       phy_vars_ue->is_secondary_ue);
	  } // SI active
	  
	  if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active == 1) {
#ifdef DEBUG_PHY
	    debug_msg("[PHY][UE %d] Frame %d, slot %d: DLSCH (RA) demod between pilot 2 and 3 (2nd slot)\n",phy_vars_ue->Mod_id,mac_xface->frame,last_slot);
#endif
	    
	    rx_dlsch(&phy_vars_ue->lte_ue_common_vars,
		     phy_vars_ue->lte_ue_dlsch_vars_ra,
		     &phy_vars_ue->lte_frame_parms,
		     eNB_id,
		     eNB_id+1,
		     &phy_vars_ue->dlsch_ue_ra[eNB_id],
		     last_slot>>1,  // subframe,
		     m,
		     0,   // first_symbol_flag
		     dual_stream_UE,
		     &phy_vars_ue->PHY_measurements,
		     phy_vars_ue->is_secondary_ue);
	  } // RA active
	  
	} // loop over 3rd quarter
      } // 3rd quarter of subframe
    } // abstraction_flag==0   
  }// l loop
#ifdef EMOS
  phy_procedures_emos_UE_RX(last_slot);
#endif

  return (0);
}
  
//#undef DEBUG_PHY

void phy_procedures_ue_lte(u8 last_slot, u8 next_slot, PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag) {

  if (subframe_select(&phy_vars_ue->lte_frame_parms,next_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_UE_TX(next_slot,phy_vars_ue,eNB_id,abstraction_flag);
  }
  if (subframe_select(&phy_vars_ue->lte_frame_parms,last_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_UE_RX(last_slot,phy_vars_ue,eNB_id,abstraction_flag);
  }
    if (subframe_select(&phy_vars_ue->lte_frame_parms,next_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_UE_S_TX(next_slot,phy_vars_ue,eNB_id,abstraction_flag);
    }
    if (subframe_select(&phy_vars_ue->lte_frame_parms,last_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      debug_msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_UE_RX(last_slot,phy_vars_ue,eNB_id,abstraction_flag);
    }

#ifdef OPENAIR2
    if (last_slot%2==0)
      mac_xface->ue_scheduler(phy_vars_ue->Mod_id, last_slot>>1);
#endif
}
