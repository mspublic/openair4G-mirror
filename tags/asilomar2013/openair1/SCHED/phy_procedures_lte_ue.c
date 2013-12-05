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

/*! \file phy_procedures_lte_ue.c
 * \brief Implementation of UE procedures from 36.213 LTE specifications
 * \author R. Knopp, F. Kaltenberger
 * \date 2011
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
 * \note
 * \warning
 */

#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif

#ifdef EXMIMO
#ifdef DRIVER2013
#include "openair0_lib.h"
extern int card;
#else
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif
#endif

#define DEBUG_PHY_PROC
#define UE_TX_POWER (-10)

//#ifdef OPENAIR2
#ifndef PUCCH
#define PUCCH
#endif
//#endif

//#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#include "UTIL/LOG/log.h"
//#endif

#ifdef EMOS
fifo_dump_emos_UE emos_dump_UE;
#endif

#include "UTIL/LOG/vcd_signal_dumper.h"

#ifndef OPENAIR2
//#define DIAG_PHY
#endif

#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 23/25 RBs)

#define NS_PER_SLOT 500000

extern inline unsigned int taus(void);
extern int oai_exit;

u8 ulsch_input_buffer[2700] __attribute__ ((aligned(16)));
u8 access_mode;

#ifdef DLSCH_THREAD
extern int dlsch_instance_cnt[8];
extern int dlsch_subframe[8];
extern pthread_mutex_t dlsch_mutex[8];
/// Condition variable for dlsch thread
extern pthread_cond_t dlsch_cond[8];
extern int rx_pdsch_instance_cnt;
extern int rx_pdsch_slot;
extern pthread_mutex_t rx_pdsch_mutex;
/// Condition variable for rx_pdsch thread
extern pthread_cond_t rx_pdsch_cond;
#endif

DCI_ALLOC_t dci_alloc_rx[8];

#ifdef DIAG_PHY
extern int rx_sig_fifo;
#endif

/*
#if defined(CBMIMO) || defined(EXMIMO)
#include <rtai_lxrt.h>
extern struct timing_info_t {
  unsigned int frame, hw_slot, last_slot, next_slot;
  RTIME time0, time1, time2;
  unsigned int mbox0, mbox1, mbox2, mbox_target;
} timing_info[2];
#endif
*/

#ifdef EXMIMO
extern u32 carrier_freq[4];
#endif

#ifdef USER_MODE

void dump_dlsch(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe,u8 harq_pid) {
  unsigned int coded_bits_per_codeword;
  u8 nsymb = (phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12;

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
                                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->nb_rb,
                                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rb_alloc,
                                  get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs),  
                                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                                  phy_vars_ue->frame,subframe);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*nsymb*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_pdsch_vars[0]->rxdataF_ext[0],2*nsymb*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_pdsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_pdsch_vars[0]->rxdataF_comp[0],300*12,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_pdsch_vars[0]->llr[0],coded_bits_per_codeword,1,0);
  
  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_pdsch_vars[0]->dl_ch_mag,300*12,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_pdsch_vars[0]->dl_ch_magb,300*12,1,1);
}

void dump_dlsch_SI(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe) {
  unsigned int coded_bits_per_codeword;
  u8 nsymb = ((phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12);

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
                                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
                                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc,
                                  get_Qm(phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs),  
                                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                                  phy_vars_ue->frame,subframe);
  LOG_D(PHY,"[UE %d] Dumping dlsch_SI : nb_rb %d, mcs %d, nb_rb %d, num_pdcch_symbols %d,G %d\n",
      phy_vars_ue->Mod_id,
      phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
      phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs,  
      phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,  
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
      coded_bits_per_codeword);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*nsymb*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_pdsch_vars_SI[0]->rxdataF_ext[0],2*nsymb*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_pdsch_vars_SI[0]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_pdsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_pdsch_vars_SI[0]->rxdataF_comp[0],300*nsymb,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_pdsch_vars_SI[0]->llr[0],coded_bits_per_codeword,1,0);
  
  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_pdsch_vars_SI[0]->dl_ch_mag,300*nsymb,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_pdsch_vars_SI[0]->dl_ch_magb,300*nsymb,1,1);
  exit(-1);
}

#ifdef EXMIMO
unsigned int prach_gain_table[31] = {100,112,126,141,158,178,200,224,251,282,316,359,398,447,501,562,631,708,794,891,1000,1122,1258,1412,1585,1778,1995,2239,2512,2818,3162};

unsigned int get_tx_amp(int gain_dBm, int gain_max_dBm) {

  //int gain_dB = gain_dBm - gain_max_dBm;
  int gain_dB = gain_max_dBm;

  if (gain_dB < -30) {
    return(AMP/32);
  }
  else if (gain_dB>0)
    return(AMP);
  else
    return(100*AMP/prach_gain_table[-gain_dB]);
}
#endif

void dump_dlsch_ra(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe) {
  unsigned int coded_bits_per_codeword;
  u8 nsymb = ((phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12);

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
                                  phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,
                                  phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->rb_alloc,
                                  get_Qm(phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->mcs),  
                                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                                  phy_vars_ue->frame,subframe);
  LOG_D(PHY,"[UE %d] Dumping dlsch_ra : nb_rb %d, mcs %d, nb_rb %d, num_pdcch_symbols %d,G %d\n",
      phy_vars_ue->Mod_id,
      phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,
      phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->mcs,  
      phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,  
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
      coded_bits_per_codeword);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_pdsch_vars_ra[0]->rxdataF_ext[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_pdsch_vars_ra[0]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_pdsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_pdsch_vars_ra[0]->rxdataF_comp[0],300*nsymb,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_pdsch_vars_ra[0]->llr[0],coded_bits_per_codeword,1,0);
  
  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_pdsch_vars_ra[0]->dl_ch_mag,300*nsymb,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_pdsch_vars_ra[0]->dl_ch_magb,300*nsymb,1,1);
}
#endif

void phy_reset_ue(u8 Mod_id,u8 eNB_index) {

  // This flushes ALL DLSCH and ULSCH harq buffers of ALL connected eNBs...add the eNB_index later
  // for more flexibility
  
  u8 i,j,k;
  PHY_VARS_UE *phy_vars_ue = PHY_vars_UE_g[Mod_id];
  //[NUMBER_OF_CONNECTED_eNB_MAX][2];
  for(i=0;i<NUMBER_OF_CONNECTED_eNB_MAX;i++) {
    for(j=0;j<2;j++) {
      //DL HARQ
      if(phy_vars_ue->dlsch_ue[i][j]) {
	for(k=0;k<NUMBER_OF_HARQ_PID_MAX && phy_vars_ue->dlsch_ue[i][j]->harq_processes[k];k++) {
	  phy_vars_ue->dlsch_ue[i][j]->harq_processes[k]->status = SCH_IDLE;
	}
      }
    }
    //UL HARQ
    if(phy_vars_ue->ulsch_ue[i]) {
      for(k=0;k<NUMBER_OF_HARQ_PID_MAX && phy_vars_ue->ulsch_ue[i]->harq_processes[k];k++) {
	phy_vars_ue->ulsch_ue[i]->harq_processes[k]->status = SCH_IDLE;
	//Set NDIs for all UL HARQs to 0
	phy_vars_ue->ulsch_ue[i]->harq_processes[k]->Ndi = 0;
	
      }
    }
    
    // flush Msg3 buffer
    phy_vars_ue->ulsch_ue_Msg3_active[i] = 0;
    
  }
}

void ra_failed(u8 Mod_id,u8 eNB_index) {

  // if contention resolution fails, go back to PRACH
  PHY_vars_UE_g[Mod_id]->UE_mode[eNB_index] = PRACH;
  LOG_E(PHY,"[UE %d] Frame %d Random-access procedure fails, going back to PRACH, setting SIStatus = 0 and State RRC_IDLE\n",Mod_id,PHY_vars_UE_g[Mod_id]->frame);
  //mac_xface->macphy_exit("");
  //  exit(-1);
}

void ra_succeeded(u8 Mod_id,u8 eNB_index) {

  int i;

  LOG_I(PHY,"[UE %d][RAPROC] Frame %d Random-access procedure succeeded\n",Mod_id,PHY_vars_UE_g[Mod_id]->frame);

  PHY_vars_UE_g[Mod_id]->ulsch_ue_Msg3_active[eNB_index] = 0;
  PHY_vars_UE_g[Mod_id]->UE_mode[eNB_index] = PUSCH;

  for (i=0;i<8;i++) { 
    if (PHY_vars_UE_g[Mod_id]->ulsch_ue[eNB_index]->harq_processes[i])
      PHY_vars_UE_g[Mod_id]->ulsch_ue[eNB_index]->harq_processes[i]->status=IDLE;
  }


}

UE_MODE_t get_ue_mode(u8 Mod_id,u8 eNB_index) {

  return(PHY_vars_UE_g[Mod_id]->UE_mode[eNB_index]);

}
void process_timing_advance_rar(PHY_VARS_UE *phy_vars_ue,u16 timing_advance) {

  /*
  if ((timing_advance>>10) & 1) //it is negative
    timing_advance = timing_advance - (1<<11);
  */

  if (openair_daq_vars.manual_timing_advance == 0) {
    phy_vars_ue->timing_advance = timing_advance*4;    

  }

#ifdef DEBUG_PHY_PROC  
  LOG_I(PHY,"[UE %d] Frame %d, received (rar) timing_advance %d, HW timing advance %d\n",phy_vars_ue->Mod_id,phy_vars_ue->frame, phy_vars_ue->timing_advance,openair_daq_vars.timing_advance);
#endif

}

void process_timing_advance(u8 Mod_id,s16 timing_advance) {

  //  u32 frame = PHY_vars_UE_g[Mod_id]->frame;
 
  if ((timing_advance>>5) & 1) //it is negative
    timing_advance = timing_advance - (1<<6);
  
  if (openair_daq_vars.manual_timing_advance == 0) {
    //if ( (frame % 100) == 0) {
    //if ((timing_advance > 3) || (timing_advance < -3) )
    PHY_vars_UE_g[Mod_id]->timing_advance = cmax(0,(int)PHY_vars_UE_g[Mod_id]->timing_advance+timing_advance*4);
      
    //}
  }

  LOG_I(PHY,"[UE %d] Got timing advance %d from MAC, new value %d\n",Mod_id, timing_advance, PHY_vars_UE_g[Mod_id]->timing_advance);
  

}

u8 is_SR_TXOp(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe) {
  
  LOG_D(PHY,"[UE %d][SR %x] Frame %d subframe %d Checking for SR TXOp (sr_ConfigIndex %d)\n",
      phy_vars_ue->Mod_id,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,phy_vars_ue->frame,subframe,
      phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex);
  
  if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 4) {        // 5 ms SR period
    if ((subframe%5) == phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex)
      return(1);
  }
  else if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 14) {  // 10 ms SR period
    if (subframe==(phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex-5))
      return(1);
  }
  else if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 34) { // 20 ms SR period
    if ((10*(phy_vars_ue->frame&1)+subframe) == (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex-15))
      return(1);
  }
  else if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 74) { // 40 ms SR period
    if ((10*(phy_vars_ue->frame&3)+subframe) == (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex-35))
      return(1);
  }
  else if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 154) { // 80 ms SR period
    if ((10*(phy_vars_ue->frame&7)+subframe) == (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex-75))
      return(1);
  }

  return(0);
}

u16 get_n1_pucch(PHY_VARS_UE *phy_vars_ue,
		 u8 eNB_id,
		 u8 subframe,
		 u8 *b,
		 u8 SR) {

  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  u8 nCCE0,nCCE1,harq_ack1,harq_ack0;
  ANFBmode_t bundling_flag;
  u16 n1_pucch0=0,n1_pucch1=0;
  int subframe_offset;
  int sf;
  int M;
  // clear this, important for case where n1_pucch selection is not used

  phy_vars_ue->pucch_sel[subframe] = 0;

  if (frame_parms->frame_type == FDD ) { // FDD
    sf = (subframe<4)? subframe+6 : subframe-4;
    printf("n1_pucch_UE: subframe %d, nCCE %d\n",sf,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[sf]);
    if (SR == 0) 
      return(frame_parms->pucch_config_common.n1PUCCH_AN + phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[sf]);
    else
      return(phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
  }
  else {

    bundling_flag = phy_vars_ue->pucch_config_dedicated[eNB_id].tdd_AckNackFeedbackMode;
#ifdef DEBUG_PHY_PROC
    if (bundling_flag==bundling){
      LOG_D(PHY,"[UE%d] Frame %d subframe %d : get_n1_pucch, bundling, SR %d/%d\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,subframe,SR,
	    phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
    }
    else {
      LOG_D(PHY,"[UE%d] Frame %d subframe %d : get_n1_pucch, multiplexing, SR %d/%d\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,subframe,SR,
	    phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
    }
#endif
    switch (frame_parms->tdd_config) {
    case 1:  // DL:S:UL:UL:DL:DL:S:UL:UL:DL

      harq_ack0 = 2; // DTX
      M=1;
      // This is the offset for a particular subframe (2,3,4) => (0,2,4)
      if (subframe == 2) {  // ACK subframes 5 (forget 6)
	subframe_offset = 5;
	M=2;
      }
      else if (subframe == 3) {   // ACK subframe 9
	subframe_offset = 9;	
      }
      else if (subframe == 7) {  // ACK subframes 0 (forget 1)
	subframe_offset = 0;
	M=2;
      }
      else if (subframe == 8) {   // ACK subframes 4
	subframe_offset = 4;
      }
      else {
	LOG_E(PHY,"[UE%d] : Frame %d phy_procedures_lte.c: get_n1pucch, illegal subframe %d for tdd_config %d\n",
	    phy_vars_ue->Mod_id,phy_vars_ue->frame,subframe,frame_parms->tdd_config);
	return(0);
      }


      // i=0
      nCCE0 = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[subframe_offset];
      n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0+ frame_parms->pucch_config_common.n1PUCCH_AN; 

      // set ACK/NAK to values if not DTX
      if (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[subframe_offset].send_harq_status>0)  // n-6 // subframe 5 is to be ACK/NAKed
	harq_ack0 = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[subframe_offset].ack; 
      

      if (harq_ack0!=2) {  // DTX
	if (SR == 0) {  // last paragraph pg 68 from 36.213 (v8.6), m=0
	  b[0]=(M==2) ? 1-harq_ack0 : harq_ack0;
	  b[1]=harq_ack0;   // in case we use pucch format 1b (subframes 2,7)
	  phy_vars_ue->pucch_sel[subframe] = 0;
	  return(n1_pucch0);
	}
	else { // SR and only 0 or 1 ACKs (first 2 entries in Table 7.3-1 of 36.213)
	  b[0]=harq_ack0;
	  return(phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
	}
      }


      break;
    case 3:  // DL:S:UL:UL:UL:DL:DL:DL:DL:DL
      // in this configuration we have M=2 from pg 68 of 36.213 (v8.6)
      // Note: this doesn't allow using subframe 1 for PDSCH transmission!!! (i.e. SF 1 cannot be acked in SF 2)
      // set ACK/NAKs to DTX
      harq_ack1 = 2; // DTX
      harq_ack0 = 2; // DTX
      // This is the offset for a particular subframe (2,3,4) => (0,2,4)
      subframe_offset = (subframe-2)<<1;
      // i=0
      nCCE0 = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[5+subframe_offset];
      n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0+ frame_parms->pucch_config_common.n1PUCCH_AN; 
      // i=1
      nCCE1 = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[(6+subframe_offset)%10];
      n1_pucch1 = get_Np(frame_parms->N_RB_DL,nCCE1,1) + nCCE1 + frame_parms->pucch_config_common.n1PUCCH_AN; 

      // set ACK/NAK to values if not DTX
      if (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[(6+subframe_offset)%10].send_harq_status>0)  // n-6 // subframe 6 is to be ACK/NAKed
	harq_ack1 = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[(6+subframe_offset)%10].ack;
      if (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[5+subframe_offset].send_harq_status>0)  // n-6 // subframe 5 is to be ACK/NAKed
	harq_ack0 = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[5+subframe_offset].ack; 
      

      if (harq_ack1!=2) { // n-6 // subframe 6,8,0 and maybe 5,7,9 is to be ACK/NAKed
	  
	if ((bundling_flag==bundling)&&(SR == 0)) {  // This is for bundling without SR, 
	                                             // n1_pucch index takes value of smallest element in set {0,1} 
	                                             // i.e. 0 if harq_ack0 is not DTX, otherwise 1
	  b[0] = harq_ack1;
	  if (harq_ack0!=2)
	    b[0]=b[0]&harq_ack0;
	  phy_vars_ue->pucch_sel[subframe] = 1;
	  return(n1_pucch1);
	  
	}
	else if ((bundling_flag==multiplexing)&&(SR==0)) {  // Table 10.1
	  if (harq_ack0 == 2)
	    harq_ack0 = 0;
	  b[1] = harq_ack0;
	  b[0] = (harq_ack0!=harq_ack1)?0:1;
	  if ((harq_ack0 == 1) && (harq_ack1 == 0)) {
	    phy_vars_ue->pucch_sel[subframe] = 0;
	    return(n1_pucch0);
	  }
	  else {
	    phy_vars_ue->pucch_sel[subframe] = 1;
	    return(n1_pucch1);
	  }
	}
	else if (SR==1) { // SR and 0,1,or 2 ACKS, (first 3 entries in Table 7.3-1 of 36.213)
	  // this should be number of ACKs (including
	  if (harq_ack0 == 2)
	    harq_ack0 = 0;
	  b[0]= harq_ack1 | harq_ack0;
	  b[1]= harq_ack1 ^ harq_ack0;
	  return(phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
	}
      }
      else if (harq_ack0!=2) {// n-7  // subframe 5,7,9 only is to be ACK/NAKed
	if ((bundling_flag==bundling)&&(SR == 0)) {  // last paragraph pg 68 from 36.213 (v8.6), m=0
	  b[0]=harq_ack0;
	  phy_vars_ue->pucch_sel[subframe] = 0;
	  return(n1_pucch0);
	}
	else if ((bundling_flag==multiplexing)&&(SR==0)) {  // Table 10.1 with i=1 set to DTX
	  b[0] = harq_ack0;
	  b[1] = 1-b[0];
	  phy_vars_ue->pucch_sel[subframe] = 0;
	  return(n1_pucch0);
	}
	else if (SR==1) { // SR and only 0 or 1 ACKs (first 2 entries in Table 7.3-1 of 36.213)
	  b[0]=harq_ack0;
	  b[1]=b[0];
	  return(phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
	}
      }
      break;

    }  // switch tdd_config     
  }
  LOG_E(PHY,"[UE%d] : Frame %d phy_procedures_lte.c: get_n1pucch, exit without proper return\n",phy_vars_ue->frame);
  return(-1);
}


#ifdef EMOS
/*
void phy_procedures_emos_UE_TX(u8 next_slot,u8 eNB_id) {
  u8 harq_pid;
  

  if (next_slot%2==0) {      
    // get harq_pid from subframe relationship
    harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,phy_vars_ue->frame,(next_slot>>1));    
    if (harq_pid==255) {
      LOG_E(PHY,"[UE%d] Frame %d : FATAL ERROR: illegal harq_pid, returning\n",
	  0,phy_vars_ue->frame);
      return;
    }

    if (ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {
      emos_dump_UE.uci_cnt[next_slot>>1] = 1;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o,ulsch_ue[eNB_id]->o,MAX_CQI_BITS*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O = ulsch_ue[eNB_id]->O;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o_RI,ulsch_ue[eNB_id]->o_RI,2*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O_RI = ulsch_ue[eNB_id]->O_RI;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o_ACK,ulsch_ue[eNB_id]->o_ACK,4*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O_ACK = ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK;
    }
    else {
      emos_dump_UE.uci_cnt[next_slot>>1] = 0;
    }
  }
}
*/
#endif

int dummy_tx_buffer[3840*4] __attribute__((aligned(16)));
#ifndef OPENAIR2
  PRACH_RESOURCES_t prach_resources_local;
#endif

void phy_procedures_UE_TX(u8 next_slot,PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag,runmode_t mode,relaying_type_t r_type) {
  
  //  int i_d;
  u16 first_rb, nb_rb;
  u8 harq_pid;
  unsigned int input_buffer_length;
  unsigned int i,aa;
  u8 Msg3_flag=0;
  u8 pucch_ack_payload[2];
  u8 n1_pucch;
  ANFBmode_t bundling_flag;
  PUCCH_FMT_t format;
  u8 SR_payload;
  s32 prach_power;
  u8 subframe,nsymb;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  u8 generate_ul_signal = 0;
  u8 ack_status=0;
  s8 Po_PUCCH;
  s32 ulsch_start=0;
#ifdef EXMIMO
  int ulsch_end=0,overflow=0;
  int k,l;
#endif

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX,VCD_FUNCTION_IN);
  start_meas(&phy_vars_ue->phy_proc_tx);

#ifdef EMOS
  //phy_procedures_emos_UE_TX(next_slot);
#endif

  if ((next_slot%2)==0) {
    phy_vars_ue->tx_power_dBm=-127;
    subframe = next_slot>>1;

    //    printf("[PHY][UE] Frame %d, subframe %d Clearing TX buffer\n",phy_vars_ue->frame,next_slot>>1);
    if ((abstraction_flag==0)) {      
      for (aa=0;aa<frame_parms->nb_antennas_tx;aa++){
	//	printf("[PHY][UE][RAROC] frame %d subframe %d Clearing TX buffer\n",phy_vars_ue->frame,next_slot>>1);
	memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe*frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti],
	       0,frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti*sizeof(s32));
      }
    }

    if (phy_vars_ue->UE_mode[eNB_id] != PRACH) {
    /*
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[UE  %d] Frame %d, slot %d: Generating SRS\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot);
#endif
      if (abstraction_flag == 0) {
#ifdef OFDMA_ULSCH
	generate_srs_tx(phy_vars_ue,eNB_id,AMP,next_slot>>1);
#else
	generate_srs_tx(phy_vars_ue,eNB_id,AMP,next_slot>>1);
#endif
      }
      
#ifdef PHY_ABSTRACTION
      else {
	generate_srs_tx_emul(phy_vars_ue,next_slot>>1);
      }
#endif
    */
      // get harq_pid from subframe relationship
      harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,
				   (((next_slot>>1)==0)?1:0) + phy_vars_ue->frame,
				   (next_slot>>1));
      

#ifdef OPENAIR2
      if ((phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 1) && 
	  (phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id] == (((next_slot>>1)==0)?1:0) + phy_vars_ue->frame) && 
	  (phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id] == (next_slot>>1))) { // Initial Transmission of Msg3
	
	phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
	if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round==0)
	  generate_ue_ulsch_params_from_rar(phy_vars_ue,
					    eNB_id);

	phy_vars_ue->ulsch_ue[eNB_id]->power_offset = 14;
	LOG_I(PHY,"[UE  %d][RAPROC] Frame %d: Setting Msg3_flag in subframe %d, for harq_pid %d\n",
	    phy_vars_ue->Mod_id,
	    phy_vars_ue->frame,
	    next_slot>>1,
	    harq_pid);
	Msg3_flag = 1;
      }
      else {
	
	if (harq_pid==255) {
	  LOG_E(PHY,"[UE%d] Frame %d ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n",
	      phy_vars_ue->Mod_id,phy_vars_ue->frame);
	  mac_xface->macphy_exit("");
          vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
	  stop_meas(&phy_vars_ue->phy_proc_tx);
	  return;
	}
	Msg3_flag=0;
      }
#endif
      
      if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {
	
	generate_ul_signal = 1;
#ifdef OPENAIR2
	pusch_power_cntl(phy_vars_ue,(next_slot>>1),eNB_id,1, abstraction_flag);
	phy_vars_ue->tx_power_dBm = phy_vars_ue->ulsch_ue[eNB_id]->Po_PUSCH;
#else
	phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif
	LOG_D(PHY,"[UE  %d][PUSCH %d] Frame %d subframe %d harq pid %d, Po_PUSCH : %d dBm\n",
	      phy_vars_ue->Mod_id,harq_pid,phy_vars_ue->frame,next_slot>>1,harq_pid, phy_vars_ue->tx_power_dBm);	

	// deactivate service request
	phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
	
	ack_status = get_ack(&phy_vars_ue->lte_frame_parms,
			     phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack,
			     (next_slot>>1),
			     phy_vars_ue->ulsch_ue[eNB_id]->o_ACK);
	
	first_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb;
	nb_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb;
	
	
	//frame_parms->pusch_config_c ommon.ul_ReferenceSignalsPUSCH.cyclicShift = 0;
	
	//frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[20] = 0;
	
	
	
#ifdef DEBUG_PHY_PROC
	LOG_I(PHY,"[UE  %d][PUSCH %d] Frame %d subframe %d Generating PUSCH : first_rb %d, nb_rb %d, round %d, Ndi %d, mcs %d, rv %d, cyclic_shift %d (cyclic_shift_common %d,n_DMRS2 %d,n_PRS %d), ACK (%d,%d), O_ACK %d\n",
	      phy_vars_ue->Mod_id,harq_pid,phy_vars_ue->frame,next_slot>>1,
	      first_rb,nb_rb,
	      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
	      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->Ndi,
	      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->mcs,
	      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
	      (frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift+
	      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->n_DMRS2+
	      frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[next_slot])%12,
	      frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift,
	      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->n_DMRS2,
	      frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[next_slot],
	      phy_vars_ue->ulsch_ue[eNB_id]->o_ACK[0],phy_vars_ue->ulsch_ue[eNB_id]->o_ACK[1],
	      phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK);
	if (ack_status > 0) {
	  LOG_I(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d Generating ACK (%d,%d) for %d bits on PUSCH\n",
		phy_vars_ue->Mod_id,
		phy_vars_ue->ulsch_ue[eNB_id]->rnti,
		phy_vars_ue->frame,next_slot>>1,
		phy_vars_ue->ulsch_ue[eNB_id]->o_ACK[0],phy_vars_ue->ulsch_ue[eNB_id]->o_ACK[1],
		phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK);
	}
#endif
	

#ifdef EXMIMO
	if (abstraction_flag==0) {
	  for (aa=0; aa<1/*frame_parms->nb_antennas_tx*/; aa++)
	    generate_drs_pusch(phy_vars_ue,eNB_id,get_tx_amp(phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm),next_slot>>1,first_rb,nb_rb,aa);
	}      
#else
	if (abstraction_flag==0) {
	  for (aa=0; aa<1/*frame_parms->nb_antennas_tx*/; aa++)
	    generate_drs_pusch(phy_vars_ue,eNB_id,AMP,next_slot>>1,first_rb,nb_rb,aa);
	}      
#endif
	
	//#ifdef DEBUG_PHY_PROC      
	//	debug_LOG_D(PHY,"[UE  %d] Frame %d, Subframe %d ulsch harq_pid %d : O %d, O_ACK %d, O_RI %d, TBS %d\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot>>1,harq_pid,phy_vars_ue->ulsch_ue[eNB_id]->O,phy_vars_ue->ulsch_ue[eNB_id]->O_ACK,phy_vars_ue->ulsch_ue[eNB_id]->O_RI,phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS);
	//#endif
	if (Msg3_flag == 1) {
	  LOG_I(PHY,"[UE  %d][RAPROC] Frame %d, Subframe %d next slot %d Generating (RRCConnectionRequest) Msg3 (nb_rb %d, first_rb %d, Ndi %d, rvidx %d) Msg3: %x.%x.%x|%x.%x.%x.%x.%x.%x\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot>>1, next_slot, 
		phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
		phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
		phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->Ndi,
		phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
		phy_vars_ue->prach_resources[eNB_id]->Msg3[0],
		phy_vars_ue->prach_resources[eNB_id]->Msg3[1],
		phy_vars_ue->prach_resources[eNB_id]->Msg3[2],
		phy_vars_ue->prach_resources[eNB_id]->Msg3[3],
		phy_vars_ue->prach_resources[eNB_id]->Msg3[4],
		phy_vars_ue->prach_resources[eNB_id]->Msg3[5],
		phy_vars_ue->prach_resources[eNB_id]->Msg3[6],
		phy_vars_ue->prach_resources[eNB_id]->Msg3[7],
		phy_vars_ue->prach_resources[eNB_id]->Msg3[8]);

	  start_meas(&phy_vars_ue->ulsch_encoding_stats);	      
	  if (abstraction_flag==0) {
	    if (ulsch_encoding(phy_vars_ue->prach_resources[eNB_id]->Msg3,
			       phy_vars_ue,
			       harq_pid,
			       eNB_id,
			       phy_vars_ue->transmission_mode[eNB_id],0,0)!=0) {
	      LOG_E(PHY,"ulsch_coding.c: FATAL ERROR: returning\n");
	      mac_xface->macphy_exit("");
              vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
	      stop_meas(&phy_vars_ue->phy_proc_tx);
	      return;
	    }
	  }
#ifdef PHY_ABSTRACTION
	  else {
	    ulsch_encoding_emul(phy_vars_ue->prach_resources[eNB_id]->Msg3,phy_vars_ue,eNB_id,harq_pid,0);
	  }
#endif
	  stop_meas(&phy_vars_ue->ulsch_encoding_stats);	      


#ifdef OPENAIR2
	  // signal MAC that Msg3 was sent
	  mac_xface->Msg3_transmitted(phy_vars_ue->Mod_id,
				      phy_vars_ue->frame,
				      eNB_id);
#endif
	}      
	else {
	  input_buffer_length = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS/8;
	 
#ifdef OPENAIR2
	  //  LOG_D(PHY,"[UE  %d] ULSCH : Searching for MAC SDUs\n",phy_vars_ue->Mod_id);
	  if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->Ndi==1) { 
	    //if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->calibration_flag == 0) {
	    access_mode=SCHEDULED_ACCESS;
	    mac_xface->ue_get_sdu(phy_vars_ue->Mod_id,
				  phy_vars_ue->frame,
				  (next_slot>>1),
				  eNB_id,
				  ulsch_input_buffer,
				  input_buffer_length,
				  &access_mode);
	    
	    //}
	    /*
	    else {
	      // Get calibration information from TDD procedures
	      LOG_D(PHY,"[UE %d] Frame %d, subframe %d : ULSCH: Getting TDD Auto-Calibration information\n",
		    phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot>>1);
	      for (i=0;i<input_buffer_length;i++)
		ulsch_input_buffer[i]= i;
	      
	    }
	    */
	  }
#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_ULSCH
	  LOG_I(PHY,"[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d)  (%d bytes) : \n",phy_vars_ue->frame,next_slot>>1,harq_pid, phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3);
	  for (i=0;i<phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3;i++) 
	    LOG_T(PHY,"%x.",ulsch_input_buffer[i]);
	  LOG_T(PHY,"\n");
#endif
#endif
#else //OPENAIR2
      // the following lines were necessary for the calibration in CROWN
      /*
      if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->calibration_flag == 0) {
          for (i=0;i<input_buffer_length;i++) 
              ulsch_input_buffer[i]= (u8)(taus()&0xff);
	  }
	  else {
          // Get calibration information from TDD procedures
      }
      */

	  for (i=0;i<input_buffer_length;i++) 
	    ulsch_input_buffer[i]= (u8)(taus()&0xff);
	  
      // the following lines were necessary for the collaborative UL in PUCCO
	  /*
	  memset(phy_vars_ue->ulsch_ue[eNB_id]->o    ,0,MAX_CQI_BYTES*sizeof(u8));
	  memset(phy_vars_ue->ulsch_ue[eNB_id]->o_RI ,0,2*sizeof(u8));
	  memset(phy_vars_ue->ulsch_ue[eNB_id]->o_ACK,0,4*sizeof(u8));
	  for (i=0;i<input_buffer_length;i++)
	    ulsch_input_buffer[i]= i;
	  */

#endif //OPENAIR2
	  start_meas(&phy_vars_ue->ulsch_encoding_stats);	      
	  if (abstraction_flag==0) {
	    /*
	    if (phy_vars_ue->frame%100==0) {
	      LOG_I(PHY,"Encoding ulsch\n");
	    }
	    */
	    if (ulsch_encoding(ulsch_input_buffer,
			       phy_vars_ue,
			       harq_pid,
			       eNB_id,
			       phy_vars_ue->transmission_mode[eNB_id],0,
			       0)!=0) {  //  Nbundled, to be updated!!!!
	      LOG_E(PHY,"ulsch_coding.c: FATAL ERROR: returning\n");
              vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
	      stop_meas(&phy_vars_ue->phy_proc_tx);
	      return;
	    }
	  }
#ifdef PHY_ABSTRACTION
	  else {
	    ulsch_encoding_emul(ulsch_input_buffer,phy_vars_ue,eNB_id,harq_pid,0);
	  }
#endif
	  stop_meas(&phy_vars_ue->ulsch_encoding_stats);	      
	}
	if (abstraction_flag == 0) {
#ifdef OPENAIR2
	  phy_vars_ue->tx_power_dBm = phy_vars_ue->ulsch_ue[eNB_id]->Po_PUSCH;
#else
	  phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif
	  LOG_I(PHY,"[UE  %d][PUSCH %d] Frame %d subframe %d, generating PUSCH, Po_PUSCH: %d dBm, amp %d\n",
		phy_vars_ue->Mod_id,harq_pid,phy_vars_ue->frame,next_slot>>1,phy_vars_ue->tx_power_dBm,
#ifdef EXMIMO
		get_tx_amp(phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm)
#else
		AMP
#endif
	);    
	  start_meas(&phy_vars_ue->ulsch_modulation_stats);	      	      	  
#ifdef OFDMA_ULSCH
	  ulsch_modulation(phy_vars_ue->lte_ue_common_vars.txdataF,
#ifdef EXMIMO                       
                       get_tx_amp(phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm),
#else
                       AMP,
#endif
                       phy_vars_ue->frame,
                       (next_slot>>1),
                       &phy_vars_ue->lte_frame_parms,
                       phy_vars_ue->ulsch_ue[eNB_id]);

#else //OFDMA_ULSCH
	  ulsch_modulation(phy_vars_ue->lte_ue_common_vars.txdataF,
#ifdef EXMIMO                       
                       get_tx_amp(phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm),
#else
                       AMP,
#endif
                       phy_vars_ue->frame,
                       (next_slot>>1),
                       &phy_vars_ue->lte_frame_parms,
                       phy_vars_ue->ulsch_ue[eNB_id]);
      
#endif //OFDMA_ULSCH
	  start_meas(&phy_vars_ue->ulsch_modulation_stats);	      	      	  
	}
	if (abstraction_flag==1) {
	  // clear SR
	  phy_vars_ue->sr[next_slot>>1]=0;
	}
      } // ULSCH is active
      else if ((phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_cba_scheduling_flag == 1) && 
	       (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->status == CBA_ACTIVE)) {
	phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;
	//	phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->status= IDLE; 
	first_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb;
	nb_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb;
	input_buffer_length = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS/8;
	access_mode=CBA_ACCESS;
	
	LOG_I(PHY,"[UE %d] Frame %d, subframe %d: CBA num dci %d \n", 
	      phy_vars_ue->Mod_id,phy_vars_ue->frame,(next_slot>>1),
	      phy_vars_ue->ulsch_ue[eNB_id]->num_cba_dci[(next_slot>>1)]);
	
	mac_xface->ue_get_sdu(phy_vars_ue->Mod_id,
			      phy_vars_ue->frame,
			      (next_slot>>1),
			      eNB_id,
			      ulsch_input_buffer,
			      input_buffer_length,
			      &access_mode);
	
	phy_vars_ue->ulsch_ue[eNB_id]->num_cba_dci[(next_slot>>1)]=0;
	
	if (access_mode > UNKNOWN_ACCESS){

	  if (abstraction_flag==0) {
	    if (ulsch_encoding(ulsch_input_buffer,
			       phy_vars_ue,
			       harq_pid,
			       eNB_id,
			       phy_vars_ue->transmission_mode[eNB_id],0,
			       0)!=0) {  //  Nbundled, to be updated!!!!
	      LOG_E(PHY,"ulsch_coding.c: FATAL ERROR: returning\n");
              vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
	      stop_meas(&phy_vars_ue->phy_proc_tx);
	      return;
	    }
	  }
#ifdef PHY_ABSTRACTION
	  else {
	    ulsch_encoding_emul(ulsch_input_buffer,phy_vars_ue,eNB_id,harq_pid,0);
	  }
#endif
	} else {
	  phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->status= IDLE; 
	  //reset_cba_uci(phy_vars_ue->ulsch_ue[eNB_id]->o);
	  LOG_N(PHY,"[UE %d] Frame %d, subframe %d: CBA transmission cancelled or postponed\n",
		phy_vars_ue->Mod_id, phy_vars_ue->frame,(next_slot>>1));
	}
      }
#ifdef PUCCH
      else if (phy_vars_ue->UE_mode[eNB_id] == PUSCH){  // check if we need to use PUCCH 1a/1b
	//      debug_LOG_D(PHY,"[UE%d] Frame %d, subframe %d: Checking for PUCCH 1a/1b\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot>>1);
	bundling_flag = phy_vars_ue->pucch_config_dedicated[eNB_id].tdd_AckNackFeedbackMode;
	
	if ((frame_parms->frame_type==FDD) || 
	    (bundling_flag==bundling)    || 
	    ((frame_parms->frame_type==TDD)&&(frame_parms->tdd_config==1)&&((next_slot!=4)||(next_slot!=14)))) {
	  format = pucch_format1a;
	  LOG_D(PHY,"[UE] PUCCH 1a\n");
	}
	else {
	  format = pucch_format1b;
	  LOG_D(PHY,"[UE] PUCCH 1b\n");
	}
	
	// Check for SR and do ACK/NACK accordingly
	if (is_SR_TXOp(phy_vars_ue,eNB_id,next_slot>>1)==1) {
	  LOG_I(PHY,"[UE %d][SR %x] Frame %d subframe %d: got SR_TXOp, Checking for SR for PUSCH from MAC\n",
	 	phy_vars_ue->Mod_id,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,phy_vars_ue->frame,next_slot>>1);
#ifdef OPENAIR2
	  SR_payload = mac_xface->ue_get_SR(phy_vars_ue->Mod_id,
					    phy_vars_ue->frame,
					    eNB_id,
					    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
					    next_slot>>1); // subframe used for meas gap
#else
	  SR_payload = 1;
#endif
	 
	  if (SR_payload>0) {
	    generate_ul_signal = 1;
	    LOG_I(PHY,"[UE %d][SR %x] Frame %d subframe %d got the SR for PUSCH is %d\n",
		  phy_vars_ue->Mod_id,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,phy_vars_ue->frame,next_slot>>1,SR_payload);
	  }
	  else {
	    phy_vars_ue->sr[next_slot>>1]=0;
	  }
	}
	else
	  SR_payload=0;
	
	if (get_ack(&phy_vars_ue->lte_frame_parms,
		    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack,
		    (next_slot>>1),pucch_ack_payload) > 0) {
	  // we need to transmit ACK/NAK in this subframe
	  
	  generate_ul_signal = 1;
	  
	  n1_pucch = get_n1_pucch(phy_vars_ue,
				  eNB_id,
				  next_slot>>1,
				  pucch_ack_payload,
				  SR_payload); 

#ifdef OPENAIR2
	    Po_PUCCH = pucch_power_cntl(phy_vars_ue,(next_slot>>1),eNB_id,format);
	    phy_vars_ue->tx_power_dBm = Po_PUCCH;
#else
	    phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif

	    if (SR_payload>0) {
	      LOG_I(PHY,"[UE  %d][SR %x] Frame %d subframe %d Generating PUCCH 1a/1b (with SR for PUSCH), n1_pucch %d, Po_PUCCH, amp %d\n",
		    phy_vars_ue->Mod_id, 
		    phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
		    phy_vars_ue->frame, next_slot>>1,
		    phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
		    Po_PUCCH,
#ifdef EXMIMO
		    get_tx_amp(Po_PUCCH,phy_vars_ue->tx_power_max_dBm)
#else
			AMP
#endif
);
	    }
	    else {
	      LOG_I(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d Generating PUCCH 1a/1b, n1_pucch %d, b[0]=%d,b[1]=%d (SR_Payload %d), Po_PUCCH %d, amp %d\n",
		    phy_vars_ue->Mod_id, 
		    phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
		    phy_vars_ue->frame, next_slot>>1,
		    n1_pucch,pucch_ack_payload[0],pucch_ack_payload[1],SR_payload,
		    Po_PUCCH,
#ifdef EXMIMO
		    get_tx_amp(Po_PUCCH,phy_vars_ue->tx_power_max_dBm)
#else
			AMP
#endif
			);
	    }

	  if (abstraction_flag == 0) {

	    generate_pucch(phy_vars_ue->lte_ue_common_vars.txdataF,
			   &phy_vars_ue->lte_frame_parms,
			   phy_vars_ue->ncs_cell,
			   format,
			   &phy_vars_ue->pucch_config_dedicated[eNB_id],
			   n1_pucch,
			   0,  // n2_pucch
			   1,  // shortened format
			   pucch_ack_payload,
#ifdef EXMIMO
			   get_tx_amp(Po_PUCCH,phy_vars_ue->tx_power_max_dBm),
#else
			   AMP,
#endif
			   next_slot>>1);

	  }
	  else {
#ifdef PHY_ABSTRACTION
	    LOG_D(PHY,"Calling generate_pucch_emul ... (ACK %d %d, SR %d)\n",pucch_ack_payload[0],pucch_ack_payload[1],SR_payload);
	    generate_pucch_emul(phy_vars_ue,
				format,
				phy_vars_ue->lte_frame_parms.pucch_config_common.nCS_AN,
				pucch_ack_payload,
				SR_payload,
				next_slot>>1);
#endif
	  }
	}
	else if (SR_payload==1) { // no ACK/NAK but SR is triggered by MAC

#ifdef OPENAIR2
	  Po_PUCCH = pucch_power_cntl(phy_vars_ue,(next_slot>>1),eNB_id,pucch_format1);
	  phy_vars_ue->tx_power_dBm = Po_PUCCH;
#else
	  phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif

	  LOG_I(PHY,"[UE  %d][SR %x] Frame %d subframe %d Generating PUCCH 1 (SR for PUSCH), n1_pucch %d, Po_PUCCH %d\n",
		phy_vars_ue->Mod_id, 
		phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
		phy_vars_ue->frame, next_slot>>1,
		phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
		Po_PUCCH);
	  
	  if (abstraction_flag == 0) {

	    generate_pucch(phy_vars_ue->lte_ue_common_vars.txdataF,
			   &phy_vars_ue->lte_frame_parms,
			   phy_vars_ue->ncs_cell,
			   pucch_format1,
			   &phy_vars_ue->pucch_config_dedicated[eNB_id],
			   phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
			   0,  // n2_pucch
			   1,  // shortened format
			   pucch_ack_payload,  // this is ignored anyway, we just need a pointer
#ifdef EXMIMO
			   get_tx_amp(Po_PUCCH,phy_vars_ue->tx_power_max_dBm),
#else
			   AMP,
#endif
			   next_slot>>1);	 
	  }
	  else {
	    LOG_D(PHY,"Calling generate_pucch_emul ...\n");
	    generate_pucch_emul(phy_vars_ue,
				pucch_format1,
				phy_vars_ue->lte_frame_parms.pucch_config_common.nCS_AN,
				pucch_ack_payload,
				SR_payload,
				next_slot>>1);
	  }
	}
      }
#endif  // PUCCH


      if (abstraction_flag == 0) {
	if (generate_ul_signal == 1 ) {
	  
	  subframe = next_slot>>1;
	  nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

#ifdef EXMIMO //this is the EXPRESS MIMO case
	ulsch_start = (phy_vars_ue->rx_offset+subframe*frame_parms->samples_per_tti-openair_daq_vars.timing_advance-phy_vars_ue->timing_advance+5)%(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti);
#else //this is the normal case
	ulsch_start = (frame_parms->samples_per_tti*subframe);
#endif //else EXMIMO

	start_meas(&phy_vars_ue->ofdm_mod_stats);	      	      	  
	for (aa=0; aa<1/*frame_parms->nb_antennas_tx*/; aa++) {
	    if (frame_parms->Ncp == 1) 
	      PHY_ofdm_mod(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe*nsymb*frame_parms->ofdm_symbol_size],
#ifdef EXMIMO
			   dummy_tx_buffer, 
#else
			   &phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],
#endif
			   frame_parms->log2_symbol_size,
			   nsymb,
			   frame_parms->nb_prefix_samples,
			   frame_parms->twiddle_ifft,
			   frame_parms->rev,
			   CYCLIC_PREFIX);
	    else
	      normal_prefix_mod(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe*nsymb*frame_parms->ofdm_symbol_size],
#ifdef EXMIMO
				dummy_tx_buffer, 
#else
				&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],
#endif
				nsymb,
				&phy_vars_ue->lte_frame_parms);

	    /*
	    if ((next_slot>>1) == 8) {
	      printf("Symbol 0 %p (offset %d) base %p\n", 
		     &phy_vars_ue->lte_ue_common_vars.txdataF[0][nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*subframe],
		     nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*subframe,
		     phy_vars_ue->lte_ue_common_vars.txdataF[0]);
	      write_output("txsigF8.m","txsF8", &phy_vars_ue->lte_ue_common_vars.txdataF[0][nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*subframe],
			   phy_vars_ue->lte_frame_parms.ofdm_symbol_size*nsymb,1,1);	      
	      write_output("txsig8.m","txs8", &phy_vars_ue->lte_ue_common_vars.txdata[0][phy_vars_ue->lte_frame_parms.samples_per_tti*subframe],
			   phy_vars_ue->lte_frame_parms.samples_per_tti,1,1);	      
	    }
	    */
#ifndef OFDMA_ULSCH
#ifdef EXMIMO
	    apply_7_5_kHz(phy_vars_ue,dummy_tx_buffer,0);
	    apply_7_5_kHz(phy_vars_ue,dummy_tx_buffer,1);
#else
	    apply_7_5_kHz(phy_vars_ue,&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],0);
	    apply_7_5_kHz(phy_vars_ue,&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],1);
#endif
	    /*
	    if ((next_slot>>1) == 8) {
	      write_output("txsig8_mod.m","txs8_mod", &phy_vars_ue->lte_ue_common_vars.txdata[0][phy_vars_ue->lte_frame_parms.samples_per_tti*subframe],
			   phy_vars_ue->lte_frame_parms.samples_per_tti,1,1);	      
	    }
	    */
#endif

#ifdef EXMIMO
	    overflow = ulsch_start - 9*frame_parms->samples_per_tti;
	    //if ((next_slot==4) && (aa==0)) printf("ulsch_start %d, overflow %d\n",ulsch_start,overflow);
	    for (k=ulsch_start,l=0; k<cmin(frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,ulsch_start+frame_parms->samples_per_tti); k++,l++)
	      {
		((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k] = ((short*)dummy_tx_buffer)[2*l]<<4;
		((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k+1] = ((short*)dummy_tx_buffer)[2*l+1]<<4;
	      }
	    for (k=0;k<overflow;k++,l++)
	      {
		((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k] = ((short*)dummy_tx_buffer)[2*l]<<4;
		((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k+1] = ((short*)dummy_tx_buffer)[2*l+1]<<4;
	      }
#endif
	  
	  } //nb_antennas_tx
	stop_meas(&phy_vars_ue->ofdm_mod_stats);	      	      	  
	} // generate_ul_signal == 1
      } 
    } // mode != PRACH
    //  }// next_slot is even
    //  else {  // next_slot is odd, do the PRACH here
 
#ifdef OPENAIR2 
    if ((phy_vars_ue->UE_mode[eNB_id] == PRACH) && (phy_vars_ue->lte_frame_parms.prach_config_common.prach_Config_enabled==1)) {
#else
    if (1) {
#endif
      // check if we have PRACH opportunity
      if (is_prach_subframe(&phy_vars_ue->lte_frame_parms,phy_vars_ue->frame,next_slot>>1)) {
	LOG_D(PHY,"UE %d: Frame %d, SF %d Clearing generate_prach\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot>>1);
	phy_vars_ue->generate_prach=0;
#ifdef OPENAIR2
	// ask L2 for RACH transport
	if ((mode != rx_calib_ue) && (mode != rx_calib_ue_med) && (mode != rx_calib_ue_byp) && (mode != no_L2_connect) ){
	  phy_vars_ue->prach_resources[eNB_id] = mac_xface->ue_get_rach(phy_vars_ue->Mod_id,
									phy_vars_ue->frame,
									eNB_id,
									next_slot>>1);
	  LOG_D(PHY,"Got prach_resources for eNB %d address %d, RRCCommon %d\n",eNB_id,phy_vars_ue->prach_resources[eNB_id],UE_mac_inst[phy_vars_ue->Mod_id].radioResourceConfigCommon);
	}
#endif
	if (phy_vars_ue->prach_resources[eNB_id]!=NULL) {
	  
	  phy_vars_ue->generate_prach=1;
	  phy_vars_ue->prach_cnt=0;
#ifdef SMBV
      phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex = 19;
#endif
#ifdef OAI_EMU
	  phy_vars_ue->prach_PreambleIndex=phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex; 
#endif
	  if (abstraction_flag == 0) {
	    LOG_D(PHY,"[UE  %d][RAPROC] Frame %d, Subframe %d : Generating PRACH, preamble %d, TARGET_RECEIVED_POWER %d dBm, PRACH TDD Resource index %d, RA-RNTI %d\n",
		  phy_vars_ue->Mod_id,
		  phy_vars_ue->frame,
		  next_slot>>1,
		  phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex,
		  phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER,
		  phy_vars_ue->prach_resources[eNB_id]->ra_TDD_map_index,
		  phy_vars_ue->prach_resources[eNB_id]->ra_RNTI);

#ifdef OPENAIR2
	    phy_vars_ue->tx_power_dBm = phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER+get_PL(phy_vars_ue->Mod_id,eNB_id);
#else
	    phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif

#ifdef EXMIMO
	    phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp = get_tx_amp(phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm);
#else
	    phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp = AMP;
#endif
	    start_meas(&phy_vars_ue->tx_prach);
	    prach_power = generate_prach(phy_vars_ue,eNB_id,next_slot>>1,phy_vars_ue->frame);
	    stop_meas(&phy_vars_ue->tx_prach);
	    LOG_D(PHY,"[UE  %d][RAPROC] PRACH PL %d dB, power %d dBm, digital power %d dB (amp %d)\n",
		  phy_vars_ue->Mod_id,
		  get_PL(phy_vars_ue->Mod_id,eNB_id),
		  phy_vars_ue->tx_power_dBm,
		  dB_fixed(prach_power),
		  phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp);
	  }
	  else {
	    UE_transport_info[phy_vars_ue->Mod_id].cntl.prach_flag=1;
	    UE_transport_info[phy_vars_ue->Mod_id].cntl.prach_id=phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex;
#ifdef OPENAIR2
	    mac_xface->Msg1_transmitted(phy_vars_ue->Mod_id,
					 phy_vars_ue->frame,
					 eNB_id);
#endif
	  }
	  LOG_D(PHY,"[UE  %d][RAPROC] Frame %d, subframe %d: Generating PRACH (eNB %d) preamble index %d for UL, TX power %d dBm (PL %d dB), l3msg \n",
		phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot>>1,eNB_id,
		phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex,
		phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER+get_PL(phy_vars_ue->Mod_id,eNB_id),
		get_PL(phy_vars_ue->Mod_id,eNB_id));

	}
      }
      LOG_D(PHY,"[UE %d] frame %d subframe %d : generate_prach %d, prach_cnt %d\n",
	    phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot>>1,phy_vars_ue->generate_prach,phy_vars_ue->prach_cnt);

      phy_vars_ue->prach_cnt++;
      if (phy_vars_ue->prach_cnt==3)
	phy_vars_ue->generate_prach=0;
    } // mode is PRACH
    else {
      phy_vars_ue->generate_prach=0; 
    }
  } // next_slot is even
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
  stop_meas(&phy_vars_ue->phy_proc_tx);
}

void phy_procedures_UE_S_TX(u8 next_slot,PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag,relaying_type_t r_type) {
  int aa;//i,aa;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;

  if (abstraction_flag==0) {
    /*
    if (phy_vars_ue->frame%100==1) {
      LOG_I(PHY,"frame %d, next_slot %d, setting switch to rx\n",phy_vars_ue->frame, next_slot);
    }
    */
    
    
    for (aa=0;aa<frame_parms->nb_antennas_tx;aa++){
#ifdef EXMIMO //this is the EXPRESS MIMO case
      int i;
      // set the whole tx buffer to RX
      for (i=0;i<LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti;i++)
	phy_vars_ue->lte_ue_common_vars.txdata[aa][i] = 0x00010001;
#else //this is the normal case
      memset(&phy_vars_ue->lte_ue_common_vars.txdata[aa][0],0,
	     (LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti)*sizeof(s32));
#endif //else EXMIMO

    }
  }
}

void lte_ue_measurement_procedures(u8 last_slot, u16 l, PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag,runmode_t mode) {
  
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  //  int aa;
#if defined(EXMIMO) && defined(DRIVER2013)
  exmimo_config_t *p_exmimo_config = openair0_exmimo_pci[card].exmimo_config_ptr;
  int aa;
#endif

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_MEASUREMENT_PROCEDURES, VCD_FUNCTION_IN);

#ifdef EMOS
  /*
  u8 aa;

  // first slot in frame is special
  if (((last_slot==0) || (last_slot==1) || (last_slot==12) || (last_slot==13)) && 
      ((l==0) || (l==4-frame_parms->Ncp))) {
    for (eNB_id=0; eNB_id<3; eNB_id++) 
      for (aa=0;aa<frame_parms->nb_antennas_tx_eNB;aa++)
	lte_dl_channel_estimation_emos(emos_dump_UE.channel[eNB_id],
				       phy_vars_ue->lte_ue_common_vars->rxdataF,
				       &phy_vars_ue->lte_frame_parms,
				       last_slot,
				       aa,
				       l,
				       eNB_id);
  }
  */
#endif

  if (l==0) {
    // UE measurements 
    if (abstraction_flag==0) {
      //LOG_D(PHY,"Calling measurements with rxdata %p\n",phy_vars_ue->lte_ue_common_vars.rxdata);

      lte_ue_measurements(phy_vars_ue,
			  ((last_slot>>1)*frame_parms->samples_per_tti+phy_vars_ue->rx_offset)%(frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME),
			  (last_slot == 2) ? 1 : 0,
			  0);
    }
    else {
      lte_ue_measurements(phy_vars_ue,
			  0,
			  0,
			  1);
    }

#ifdef DEBUG_PHY_PROC    
    if ((last_slot == 2)) { // && (phy_vars_ue->frame%100==0)) {
	
      LOG_D(PHY,"[UE  %d] frame %d, slot %d, freq_offset_filt = %d \n",phy_vars_ue->Mod_id,phy_vars_ue->frame, last_slot, phy_vars_ue->lte_ue_common_vars.freq_offset);
      /*	
      LOG_I(PHY,"[UE  %d] frame %d, slot %d, RX RSSI (%d,%d,%d) dBm, digital (%d,%d)(%d,%d)(%d,%d) dB, linear (%d,%d), avg rx power %d dB (%d lin), N0 %d dB (%d lin), RX gain %d dB\n",
	    phy_vars_ue->Mod_id,phy_vars_ue->frame, last_slot,
	    phy_vars_ue->PHY_measurements.rx_rssi_dBm[0],
	    phy_vars_ue->PHY_measurements.rx_rssi_dBm[1],
	    phy_vars_ue->PHY_measurements.rx_rssi_dBm[2],
	    phy_vars_ue->PHY_measurements.rx_power_dB[0][0],
	    phy_vars_ue->PHY_measurements.rx_power_dB[0][1],
	    phy_vars_ue->PHY_measurements.rx_power_dB[1][0],
	    phy_vars_ue->PHY_measurements.rx_power_dB[1][1],
	    phy_vars_ue->PHY_measurements.rx_power_dB[2][0],
	    phy_vars_ue->PHY_measurements.rx_power_dB[2][1],
	    phy_vars_ue->PHY_measurements.rx_power[0][0],
	    phy_vars_ue->PHY_measurements.rx_power[0][1],		  
	    phy_vars_ue->PHY_measurements.rx_power_avg_dB[0],
	    phy_vars_ue->PHY_measurements.rx_power_avg[0],
	    phy_vars_ue->PHY_measurements.n0_power_avg_dB,
	    phy_vars_ue->PHY_measurements.n0_power_avg,
	    phy_vars_ue->rx_total_gain_dB);
      
      LOG_I(PHY,"[UE  %d] frame %d, slot %d, N0 %d dBm digital (%d, %d) dB, linear (%d, %d), avg noise power %d dB (%d lin)\n",
		phy_vars_ue->Mod_id,phy_vars_ue->frame, last_slot,
		phy_vars_ue->PHY_measurements.n0_power_tot_dBm,
		phy_vars_ue->PHY_measurements.n0_power_dB[0],
		phy_vars_ue->PHY_measurements.n0_power_dB[1],
		phy_vars_ue->PHY_measurements.n0_power[0],
		phy_vars_ue->PHY_measurements.n0_power[1],
		phy_vars_ue->PHY_measurements.n0_power_avg_dB,
		phy_vars_ue->PHY_measurements.n0_power_avg); */
    }
#endif
  }

  if (l==(4-frame_parms->Ncp)) {


    ue_rrc_measurements(phy_vars_ue,
			last_slot,
			abstraction_flag);

    phy_vars_ue->sinr_eff =  sinr_eff_cqi_calc(phy_vars_ue, 0);

  }  

  if ((last_slot==1) && (l==(4-frame_parms->Ncp))) {
    
    // AGC
    
    if ((openair_daq_vars.rx_gain_mode == DAQ_AGC_ON) &&
	(mode != rx_calib_ue) && (mode != rx_calib_ue_med) && (mode != rx_calib_ue_byp) )
        phy_adjust_gain (phy_vars_ue,0);
    
    eNB_id = 0;
    
    if (abstraction_flag == 0) 
        lte_adjust_synch(&phy_vars_ue->lte_frame_parms,
                         phy_vars_ue,
                         eNB_id,
                         0,
                         16384);
    
    if (openair_daq_vars.auto_freq_correction == 1) {
      if (phy_vars_ue->frame % 100 == 0) {
	if ((phy_vars_ue->lte_ue_common_vars.freq_offset>100) && (openair_daq_vars.freq_offset < 1000)) {
	  openair_daq_vars.freq_offset+=100;
#if defined(EXMIMO) && defined(DRIVER2013)
	  for (aa = 0; aa<4; aa++) { 
	    p_exmimo_config->rf.rf_freq_rx[aa] = carrier_freq[aa]+=openair_daq_vars.freq_offset;
	    p_exmimo_config->rf.rf_freq_tx[aa] = carrier_freq[aa]+=openair_daq_vars.freq_offset;
	  }
#endif
 	}
	else if ((phy_vars_ue->lte_ue_common_vars.freq_offset<-100) && (openair_daq_vars.freq_offset > -1000)) {
	  openair_daq_vars.freq_offset-=100;
#if defined(EXMIMO) && defined(DRIVER2013)
	  for (aa = 0; aa<4; aa++) { 
	    p_exmimo_config->rf.rf_freq_rx[aa] = carrier_freq[aa]+=openair_daq_vars.freq_offset;
	    p_exmimo_config->rf.rf_freq_tx[aa] = carrier_freq[aa]+=openair_daq_vars.freq_offset;
	  }
#endif
	}
      }
    }

  }
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_MEASUREMENT_PROCEDURES, VCD_FUNCTION_OUT);
}

#ifdef EMOS
void phy_procedures_emos_UE_RX(PHY_VARS_UE *phy_vars_ue,u8 last_slot,u8 eNB_id) {

  u8 i,j;
  //u16 last_slot_emos;
  u32 bytes;

  /*
  if (last_slot<2)
    last_slot_emos = last_slot;
  else if (last_slot>9)
    last_slot_emos = last_slot - 8;
  else {
    LOG_E(PHY,"emos rx last_slot_emos %d, last_slot %d\n", last_slot_emos,last_slot);
    mac_xface->macphy_exit("should never happen");
  }
  */

#ifdef EMOS_CHANNEL
  if ((last_slot==10) || (last_slot==11)) {
    for (i=0; i<phy_vars_ue->lte_frame_parms.nb_antennas_rx; i++)
      for (j=0; j<phy_vars_ue->lte_frame_parms.nb_antennas_tx; j++) { 
	// first OFDM symbol with pilots
	memcpy(&emos_dump_UE.channel[i][j][(last_slot%2)*2*phy_vars_ue->lte_frame_parms.ofdm_symbol_size],
	       &phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNB_id][(j<<1) + i][0],
	       phy_vars_ue->lte_frame_parms.ofdm_symbol_size*sizeof(int));
	// second OFDM symbol with pilots
	memcpy(&emos_dump_UE.channel[i][j][((last_slot%2)*2+1)*phy_vars_ue->lte_frame_parms.ofdm_symbol_size],
	       &phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNB_id][(j<<1) + i][(phy_vars_ue->lte_frame_parms.Ncp == 0 ? 4 : 3)*phy_vars_ue->lte_frame_parms.ofdm_symbol_size],
	       phy_vars_ue->lte_frame_parms.ofdm_symbol_size*sizeof(int));
      }
  }
#endif
  
  if (last_slot==0) {
    emos_dump_UE.timestamp = rt_get_time_ns();
    emos_dump_UE.frame_rx = phy_vars_ue->frame;
    emos_dump_UE.UE_mode = phy_vars_ue->UE_mode[eNB_id];
    emos_dump_UE.mimo_mode = phy_vars_ue->transmission_mode[eNB_id];
    emos_dump_UE.freq_offset = phy_vars_ue->lte_ue_common_vars.freq_offset;
    emos_dump_UE.timing_advance = phy_vars_ue->timing_advance;
    emos_dump_UE.timing_offset  = phy_vars_ue->rx_offset;
    emos_dump_UE.rx_total_gain_dB = phy_vars_ue->rx_total_gain_dB;
    emos_dump_UE.eNb_id = eNB_id;
    memcpy(&emos_dump_UE.PHY_measurements,&phy_vars_ue->PHY_measurements,sizeof(PHY_MEASUREMENTS));
  }
  if (last_slot==1) {
    emos_dump_UE.pbch_errors = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors;
    emos_dump_UE.pbch_errors_last = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_last;
    emos_dump_UE.pbch_errors_conseq = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq;
    emos_dump_UE.pbch_fer = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_fer;
  }
  if (last_slot==19) {
    emos_dump_UE.dlsch_errors = phy_vars_ue->dlsch_errors[eNB_id];
    emos_dump_UE.dlsch_errors_last = phy_vars_ue->dlsch_errors_last[eNB_id];
    emos_dump_UE.dlsch_received = phy_vars_ue->dlsch_received[eNB_id];
    emos_dump_UE.dlsch_received_last = phy_vars_ue->dlsch_received_last[eNB_id];
    emos_dump_UE.dlsch_fer = phy_vars_ue->dlsch_fer[eNB_id];
    emos_dump_UE.dlsch_cntl_errors = phy_vars_ue->dlsch_SI_errors[eNB_id];
    emos_dump_UE.dlsch_ra_errors = phy_vars_ue->dlsch_ra_errors[eNB_id];
    emos_dump_UE.total_TBS = phy_vars_ue->total_TBS[eNB_id];
    emos_dump_UE.total_TBS_last = phy_vars_ue->total_TBS_last[eNB_id];
    emos_dump_UE.bitrate = phy_vars_ue->bitrate[eNB_id];
    emos_dump_UE.total_received_bits = phy_vars_ue->total_received_bits[eNB_id];
    emos_dump_UE.pmi_saved = phy_vars_ue->dlsch_ue[eNB_id][0]->pmi_alloc;
    emos_dump_UE.mcs = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->mcs;
    emos_dump_UE.use_ia_receiver = openair_daq_vars.use_ia_receiver;

    bytes = rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_UE, sizeof(fifo_dump_emos_UE));
    if (bytes!=sizeof(fifo_dump_emos_UE)) {
      LOG_W(PHY,"[UE  %d] frame %d, slot %d, Problem writing EMOS data to FIFO\n",phy_vars_ue->Mod_id,phy_vars_ue->frame, last_slot);
    }
    else {
      if (phy_vars_ue->frame%100==0) {
	LOG_I(PHY,"[UE  %d] frame %d, slot %d, Writing %d bytes EMOS data to FIFO\n",phy_vars_ue->Mod_id,phy_vars_ue->frame, last_slot, bytes);
      }
    }
  }
  
}
#endif


void restart_phy(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag) {

  //  u8 last_slot;
  u8 i;
  LOG_D(PHY,"[UE  %d] frame %d, slot %d, restarting PHY!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame);
  mac_xface->macphy_exit("");
  //   first_run = 1;
  
  if (abstraction_flag ==0 ) {
    openair_daq_vars.mode = openair_NOT_SYNCHED;
    phy_vars_ue->UE_mode[eNB_id] = NOT_SYNCHED;
    openair_daq_vars.sync_state=0;
  }
  else {
    phy_vars_ue->UE_mode[eNB_id] = PRACH;
    phy_vars_ue->prach_resources[eNB_id]=NULL;
  }
  phy_vars_ue->frame = -1;
  openair_daq_vars.synch_wait_cnt=0;
  openair_daq_vars.sched_cnt=-1;
#if defined(EXMIMO) || defined(CBMIMO1)
  //openair_daq_vars.timing_advance = TIMING_ADVANCE_HW;
#endif
  
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
  phy_vars_ue->dlsch_SI_received[eNB_id] = 0;
  phy_vars_ue->dlsch_ra_received[eNB_id] = 0;
  phy_vars_ue->dlsch_SI_errors[eNB_id] = 0;
  phy_vars_ue->dlsch_ra_errors[eNB_id] = 0;

  phy_vars_ue->dlsch_mch_received[eNB_id] = 0;  
  for (i=0; i < MAX_MBSFN_AREA ; i ++){
    phy_vars_ue->dlsch_mch_received_sf[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mcch_received[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mtch_received[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mcch_errors[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mtch_errors[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mcch_trials[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mtch_trials[i][eNB_id] = 0;
}  
  //phy_vars_ue->total_TBS[eNB_id] = 0;
  //phy_vars_ue->total_TBS_last[eNB_id] = 0;
  //phy_vars_ue->bitrate[eNB_id] = 0;
  //phy_vars_ue->total_received_bits[eNB_id] = 0;
}


void lte_ue_pbch_procedures(u8 eNB_id,u8 last_slot, PHY_VARS_UE *phy_vars_ue,u8 abstraction_flag) {

  //  int i;
  int pbch_tx_ant=0;
  u8 pbch_phase;
  u16 frame_tx;
  static u8 first_run = 1;
  u8 pbch_trials = 0;

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PBCH_PROCEDURES, VCD_FUNCTION_IN);

  pbch_phase=(phy_vars_ue->frame%4);
  if (pbch_phase>=4)
    pbch_phase=0;
  for (pbch_trials=0;pbch_trials<4;pbch_trials++) {
  //for (pbch_phase=0;pbch_phase<4;pbch_phase++) {
    //LOG_I(PHY,"[UE  %d] Frame %d, Trying PBCH %d (NidCell %d, eNB_id %d)\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,pbch_phase,phy_vars_ue->lte_frame_parms.Nid_cell,eNB_id);
    if (abstraction_flag == 0) {
      pbch_tx_ant = rx_pbch(&phy_vars_ue->lte_ue_common_vars,
			    phy_vars_ue->lte_ue_pbch_vars[eNB_id],
			    &phy_vars_ue->lte_frame_parms,
			    eNB_id,
			    phy_vars_ue->lte_frame_parms.mode1_flag==1?SISO:ALAMOUTI,
			    pbch_phase);



    }
#ifdef PHY_ABSTRACTION
    else {
      pbch_tx_ant = rx_pbch_emul(phy_vars_ue,
				 eNB_id,
				 pbch_phase);
    }
#endif

    if ((pbch_tx_ant>0) && (pbch_tx_ant<=4)) {
      break;
    }
    pbch_phase++;
    if (pbch_phase>=4)
      pbch_phase=0;
  }



  if ((pbch_tx_ant>0) && (pbch_tx_ant<=4)) {

    if (pbch_tx_ant>2){
      LOG_W(PHY,"[openair][SCHED][SYNCH] PBCH decoding: pbch_tx_ant>2 not supported\n");
      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PBCH_PROCEDURES, VCD_FUNCTION_OUT);
      return;
    }


    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq = 0;
    frame_tx = (((int)(phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[2]&0x03))<<8); 
    frame_tx += ((int)(phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1]&0xfc));
    frame_tx += pbch_phase;

#ifdef OPENAIR2
    mac_xface->dl_phy_sync_success(phy_vars_ue->Mod_id,phy_vars_ue->frame,eNB_id,
				   phy_vars_ue->UE_mode[eNB_id]==NOT_SYNCHED ? 1 : 0);
#endif

#ifdef EMOS
    //emos_dump_UE.frame_tx = frame_tx;
    //emos_dump_UE.mimo_mode = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1];
#endif

    if (first_run) {
      first_run = 0;
      LOG_I(PHY,"[UE %d] frame %d, slot %d: Adjusting frame counter (PBCH ant_tx=%d, frame_tx=%d, phase %d).\n",
	    phy_vars_ue->Mod_id, 
	    phy_vars_ue->frame,
	    last_slot,
	    pbch_tx_ant,
	    frame_tx,
	    pbch_phase);
      phy_vars_ue->frame = (phy_vars_ue->frame & 0xFFFFFC00) | (frame_tx & 0x000003FF);
    }
    else 
      if (((frame_tx & 0x03FF) != (phy_vars_ue->frame & 0x03FF))) { 
	  //(pbch_tx_ant != phy_vars_ue->lte_frame_parms.nb_antennas_tx)) {
	LOG_D(PHY,"[UE %d] frame %d, slot %d: Re-adjusting frame counter (PBCH ant_tx=%d, frame_tx=%d, frame%1024=%d, phase %d).\n",
	      phy_vars_ue->Mod_id, 
	      phy_vars_ue->frame,
	      last_slot,
	      pbch_tx_ant,
	      frame_tx,
	      phy_vars_ue->frame & 0x03FF,
	      pbch_phase);
	/*
#if defined(CBMIMO) || defined(EXMIMO)
	for (i=0;i<20;i++){
	  LOG_D(PHY,"slot %d: frame %d, hw_slot %d, last_slot %d, next_slot %d, time0 %llu, time1 %llu, time2 %llu, mbox0 %d, mbox1 %d, mbox2 %d, mbox_target %d\n",
		i, timing_info[i].frame, timing_info[i].hw_slot, timing_info[i].last_slot, timing_info[i].next_slot, 
		timing_info[i].time0, timing_info[i].time1, timing_info[i].time2, 
		timing_info[i].mbox0, timing_info[i].mbox1, timing_info[i].mbox2, timing_info[i].mbox_target);
	}
#endif
	*/
	phy_vars_ue->frame = (phy_vars_ue->frame & 0xFFFFFC00) | (frame_tx & 0x000003FF);
	/*
	LOG_D(PHY,"[UE  %d] frame %d, slot %d: PBCH PDU does not match, ignoring it (PBCH ant_tx=%d, frame_tx=%d).\n",
	    phy_vars_ue->Mod_id, 
	    phy_vars_ue->frame,
	    last_slot,
	    pbch_tx_ant,
	    frame_tx);
	*/
	//phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq = 21; // this will make it go out of sync
	//phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq += 1; // this will make it go out of sync
      }
        
#ifdef DEBUG_PHY_PROC
    LOG_D(PHY,"[UE %d] frame %d, slot %d, Received PBCH (MIB): mode1_flag %d, tx_ant %d, frame_tx %d. N_RB_DL %d, phich_duration %d, phich_resource %d/6!\n",
	      phy_vars_ue->Mod_id, 
	      phy_vars_ue->frame,
	      last_slot,
	      phy_vars_ue->lte_frame_parms.mode1_flag,
	      pbch_tx_ant,
	      frame_tx,
	      phy_vars_ue->lte_frame_parms.N_RB_DL,
	      phy_vars_ue->lte_frame_parms.phich_config_common.phich_duration,
	      phy_vars_ue->lte_frame_parms.phich_config_common.phich_resource);
    if ((phy_vars_ue->frame%100==0)&&(phy_vars_ue!=NULL)) {
      LOG_I(PHY,"[UE %d] frame %d, slot %d, PBCH: mode1_flag %d, tx_ant %d, frame_tx %d, phase %d. N_RB_DL %d, phich_duration %d, phich_resource %d/6\n",
	      phy_vars_ue->Mod_id, 
	      phy_vars_ue->frame,
	      last_slot,
	      phy_vars_ue->lte_frame_parms.mode1_flag,
	      pbch_tx_ant,
	      frame_tx,
	      pbch_phase,
	      phy_vars_ue->lte_frame_parms.N_RB_DL,
	      phy_vars_ue->lte_frame_parms.phich_config_common.phich_duration,
          phy_vars_ue->lte_frame_parms.phich_config_common.phich_resource);
      //dump_frame_parms(&phy_vars_ue->lte_frame_parms);
      }
#endif
        
  }  
  else {
    LOG_E(PHY,"[UE %d] frame %d, slot %d, Error decoding PBCH!\n",
	phy_vars_ue->Mod_id,phy_vars_ue->frame, last_slot);
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq++;
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors++;
#ifdef OPENAIR2
    mac_xface->out_of_sync_ind(phy_vars_ue->Mod_id,phy_vars_ue->frame,eNB_id);
#else
    if (phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq>=100) {
      LOG_E(PHY,"More that 100 consecutive PBCH errors! Exiting!\n");
      mac_xface->macphy_exit("");
    }
#endif
  }

  if (phy_vars_ue->frame % 100 == 0) {
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_fer = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors - phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_last;
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_last = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors;
  }
  
#ifdef DEBUG_PHY_PROC  
  LOG_D(PHY,"[UE %d] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
	    phy_vars_ue->Mod_id,phy_vars_ue->frame, last_slot, 
	    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors, 
	    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq);
#endif 
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PBCH_PROCEDURES, VCD_FUNCTION_OUT);
}

int lte_ue_pdcch_procedures(u8 eNB_id,u8 last_slot, PHY_VARS_UE *phy_vars_ue,u8 abstraction_flag) {	

  unsigned int dci_cnt=0, i;
  //DCI_PDU *DCI_pdu;
  //u16 ra_RNTI;
  u8 harq_pid;
  s8 UE_id;

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_IN);

  /*
#ifdef DEBUG_PHY_PROC
  debug_LOG_D(PHY,"[UE  %d] Frame %d, slot %d (%d): DCI decoding crnti %x (mi %d)\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot,last_slot>>1,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,get_mi(&phy_vars_ue->lte_frame_parms,last_slot>>1));
#endif
  */
  if (abstraction_flag == 0)  {

    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PDCCH, VCD_FUNCTION_IN);
    rx_pdcch(&phy_vars_ue->lte_ue_common_vars,
	     phy_vars_ue->lte_ue_pdcch_vars,
	     &phy_vars_ue->lte_frame_parms,
	     last_slot>>1,
	     eNB_id,
	     (phy_vars_ue->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
	     phy_vars_ue->is_secondary_ue);
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PDCCH, VCD_FUNCTION_OUT);
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_DCI_DECODING, VCD_FUNCTION_IN);
    dci_cnt = dci_decoding_procedure(phy_vars_ue,
				     dci_alloc_rx,
				     (phy_vars_ue->UE_mode[eNB_id] < PUSCH)? 1 : 0,  // if we're in PUSCH don't listen to common search space, 
				                                                    // later when we need paging or RA during connection, update this ...
				     eNB_id,last_slot>>1);
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_DCI_DECODING, VCD_FUNCTION_OUT);
    //LOG_D(PHY,"[UE  %d][PUSCH] Frame %d subframe %d PHICH RX\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1);
 
    if (is_phich_subframe(&phy_vars_ue->lte_frame_parms,last_slot>>1)) {
      rx_phich(phy_vars_ue,
	       last_slot>>1,eNB_id);
    }
  }
#ifdef PHY_ABSTRACTION
  else {
    for (i=0;i<NB_eNB_INST;i++) {
      if (PHY_vars_eNB_g[i]->lte_frame_parms.Nid_cell == phy_vars_ue->lte_frame_parms.Nid_cell)
	break;
    }
    if (i==NB_eNB_INST) {
      LOG_E(PHY,"[UE  %d] phy_procedures_lte_ue.c: FATAL : Could not find attached eNB for DCI emulation (Nid_cell %d)!!!!\n",phy_vars_ue->Mod_id,phy_vars_ue->lte_frame_parms.Nid_cell);
      mac_xface->macphy_exit("");
      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
      return(-1);
    }
    LOG_D(PHY,"Calling dci_decoding_proc_emul ...\n");
    dci_cnt = dci_decoding_procedure_emul(phy_vars_ue->lte_ue_pdcch_vars,
					  PHY_vars_eNB_g[i]->num_ue_spec_dci[(last_slot>>1)&1],
					  PHY_vars_eNB_g[i]->num_common_dci[(last_slot>>1)&1],
					  PHY_vars_eNB_g[i]->dci_alloc[(last_slot>>1)&1],
					  dci_alloc_rx,
					  eNB_id);
    //    printf("DCI: dci_cnt %d\n",dci_cnt);
    UE_id = (u32)find_ue((s16)phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,PHY_vars_eNB_g[i]);
    if (UE_id>=0) {
      //      msg("Checking PHICH for UE  %d (eNB %d)\n",UE_id,i);
      if (is_phich_subframe(&phy_vars_ue->lte_frame_parms,last_slot>>1)) {
	harq_pid = phich_subframe_to_harq_pid(&phy_vars_ue->lte_frame_parms,phy_vars_ue->frame,last_slot>>1);	
	if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->status == ACTIVE) {
	  // phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->phich_ACK=1;
	  phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag =0;
	  phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->status = IDLE;
	  phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] = 0;
	  phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->Ndi = 1;
	  LOG_D(PHY,"Msg3 inactive\n");
	  /* Phich is not abstracted for the moment
	  if (PHY_vars_eNB_g[i]->ulsch_eNB[(u32)UE_id]->harq_processes[harq_pid]->phich_ACK==0) { // NAK
	    if (phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 1) {
#ifdef DEBUG_PHY_PROC
	      LOG_D(PHY,"[UE  %d][RAPROC] Frame %d, subframe %d: Msg3 PHICH, received NAK\n",
		  phy_vars_ue->Mod_id,
		  phy_vars_ue->frame,
		  last_slot>>1);
#endif	  
	      get_Msg3_alloc_ret(&phy_vars_ue->lte_frame_parms,
				 last_slot>>1,
				 phy_vars_ue->frame,
    				 &phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
				 &phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
	    }
	    //	    PHY_vars_eNB_g[i]->ulsch_eNB[UE_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
	    //	    PHY_vars_eNB_g[i]->ulsch_eNB[UE_id]->harq_processes[harq_pid]->Ndi = 0;
	    //	    PHY_vars_eNB_g[i]->ulsch_eNB[UE_id]->harq_processes[harq_pid]->round++;	  
	  }
	  else {
#ifdef DEBUG_PHY_PROC
	    if (phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 1) 
	      LOG_D(PHY,"[UE  %d][RAPROC] Frame %d, subframe %d: Msg3 PHICH, received ACK\n",
		  phy_vars_ue->Mod_id,
		  phy_vars_ue->frame,
		  last_slot>>1);
#endif
	    	    PHY_vars_eNB_g[i]->ulsch_eNB[UE_id]->harq_processes[harq_pid]->subframe_scheduling_flag =0;
	    	    PHY_vars_eNB_g[i]->ulsch_eNB[UE_id]->harq_processes[harq_pid]->status = IDLE;
	    // inform MAC?
	    phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] = 0;	  
	  } //phich_ACK	*/
	} // harq_pid is ACTIVE
      } // This is a PHICH subframe
    } // UE_id exists
  }
#endif

#ifdef DEBUG_PHY_PROC
  LOG_D(PHY,"[UE  %d] Frame %d, slot %d, Mode %s: DCI found %i\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot,mode_string[phy_vars_ue->UE_mode[eNB_id]],dci_cnt);
#endif

  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received += dci_cnt;
  /*
#ifdef DEBUG_PHY_PROC
  if (last_slot==18)
    debug_LOG_D(PHY,"[UE  %d] Frame %d, slot %d: PDCCH: DCI errors %d, DCI received %d, DCI missed %d, DCI False Detection %d \n",
	      phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot,
	      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors,
	      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received,
	      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_missed,
	      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false);
#endif
  */  
#ifdef EMOS
  //emos_dump_UE.dci_cnt[last_slot>>1] = dci_cnt;
#endif

  /*
    #ifdef DIAG_PHY
    //if (phy_vars_ue->UE_mode[eNB_id] == PUSCH)
    if (dci_cnt > 1) {
    LOG_D(PHY,"[UE  %d][DIAG] frame %d, subframe %d: received %d>1 DCI!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1,dci_cnt);
    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
    } 
    else if (dci_cnt==0) {
    LOG_D(PHY,"[UE  %d][DIAG] frame %d, subframe %d: received %d DCI!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1,dci_cnt);
    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_missed++;
    }
    #endif
  */

  // dci_cnt = 0;
  //  ra_RNTI = (phy_vars_ue->prach_resources[eNB_id]) ? phy_vars_ue->prach_resources[eNB_id]->ra_RNTI : 0;
  for (i=0;i<dci_cnt;i++){

#ifdef DEBUG_PHY_PROC    
    if ((last_slot>>1) == 9) { //( phy_vars_ue->frame % 100 == 0)   {
      LOG_D(PHY,"frame %d, subframe %d, rnti %x: dci %d/%d\n",phy_vars_ue->frame,last_slot>>1,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,i,dci_cnt);
      //dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
    }
#endif
    
    //if ((phy_vars_ue->UE_mode[eNB_id] != PRACH) && 
    //    (dci_alloc_rx[i].rnti != 0x1234) &&
    if((dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti) &&
       (dci_alloc_rx[i].format != format0)) {
#ifdef DEBUG_PHY_PROC
      LOG_I(PHY,"[UE  %d][DCI][PDSCH %x] frame %d, subframe %d: format %d, num_pdcch_symbols %d, nCCE %d, total CCEs %d\n",
	    phy_vars_ue->Mod_id,dci_alloc_rx[i].rnti,
	    phy_vars_ue->frame,last_slot>>1,
	    dci_alloc_rx[i].format,
            phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
            phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[last_slot>>1],
	    get_nCCE(3,&phy_vars_ue->lte_frame_parms,get_mi(&phy_vars_ue->lte_frame_parms,0)));

      /*
      if (((phy_vars_ue->frame%100) == 0) || (phy_vars_ue->frame < 20))
	dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      */

#endif      
#ifdef DIAG_PHY
      if (!((((last_slot>>1) == 7) && (dci_alloc_rx[i].format == format1E_2A_M10PRB)) ||
	    (((last_slot>>1) == 7) && (dci_alloc_rx[i].format == format1)))) {
	LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received C_RNTI Format %d!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1,dci_alloc_rx[i].format);
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
        vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
	return(-1);
      }
#endif
      

      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (void *)&dci_alloc_rx[i].dci_pdu,
					    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
					    dci_alloc_rx[i].format,
					    phy_vars_ue->dlsch_ue[eNB_id],
					    &phy_vars_ue->lte_frame_parms,
					    phy_vars_ue->pdsch_config_dedicated,
					    SI_RNTI,
					    0,
					    P_RNTI)==0) {

#ifdef DIAG_PHY
	if (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->mcs != (((phy_vars_ue->frame%1024)%28))){
	    LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: wrong mcs!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1,phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->mcs);
	    dump_dci(&phy_vars_ue->lte_frame_parms,(void *)&dci_alloc_rx[i]);
	  }
#endif


	phy_vars_ue->dlsch_received[eNB_id]++;
	
#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[UE  %d] Generated UE DLSCH C_RNTI format %d\n",phy_vars_ue->Mod_id,dci_alloc_rx[i].format);
#endif    
	
	// we received a CRNTI, so we're in PUSCH
	if (phy_vars_ue->UE_mode[eNB_id] != PUSCH) {
#ifdef DEBUG_PHY_PROC
	  LOG_I(PHY,"[UE  %d] Frame %d, subframe %d: Received DCI with CRNTI %x => Mode PUSCH\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti);
#endif
	  //dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
	  phy_vars_ue->UE_mode[eNB_id] = PUSCH;
	  //mac_xface->macphy_exit("Connected. Exiting\n");
	}
      }
      else {
	LOG_E(PHY,"[UE  %d] Frame %d, subframe %d: Problem in DCI!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1);
	dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      }
    }

    else if ((dci_alloc_rx[i].rnti == SI_RNTI) && 
	     (dci_alloc_rx[i].format == format1A)) {
      
#ifdef DEBUG_PHY_PROC
      LOG_I(PHY,"[UE  %d] subframe %d: Found rnti %x, format 1A, dci_cnt %d\n",phy_vars_ue->Mod_id,last_slot>>1,dci_alloc_rx[i].rnti,i);
      /*
      if (((phy_vars_ue->frame%100) == 0) || (phy_vars_ue->frame < 20))
	dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      */
#endif      
#ifdef DIAG_PHY
      if (((last_slot>>1) != 5)) {
	LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received SI_RNTI!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1);
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
        vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
	return(-1);
      }
#endif

      if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					    (void *)&dci_alloc_rx[i].dci_pdu,
					    SI_RNTI,
					    format1A,
					    &phy_vars_ue->dlsch_ue_SI[eNB_id], 
					    &phy_vars_ue->lte_frame_parms,
					    phy_vars_ue->pdsch_config_dedicated,
					    SI_RNTI,
					    0,
					    P_RNTI)==0) {

	phy_vars_ue->dlsch_SI_received[eNB_id]++;

#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[UE  %d] Generate UE DLSCH SI_RNTI format 1A\n",phy_vars_ue->Mod_id);
#endif
      }
    }
  
    else if ((phy_vars_ue->prach_resources[eNB_id]) && 
	     (dci_alloc_rx[i].rnti == phy_vars_ue->prach_resources[eNB_id]->ra_RNTI) && 
	     (dci_alloc_rx[i].format == format1A)) {
#ifdef DEBUG_PHY_PROC
	LOG_I(PHY,"[UE  %d][RAPROC] subframe %d: Found RA rnti %x, format 1A, dci_cnt %d\n",phy_vars_ue->Mod_id,last_slot>>1,dci_alloc_rx[i].rnti,i);
	
	//if (((phy_vars_ue->frame%100) == 0) || (phy_vars_ue->frame < 20))
	//dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
	//mac_xface->macphy_exit("so far so good...\n");
#endif      
#ifdef DIAG_PHY
	if ((last_slot>>1) != 9) {
	  LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received RA_RNTI!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1);
	  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
          vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
	  return(-1);
	}
#endif
      
	if (generate_ue_dlsch_params_from_dci(last_slot>>1,
					      (DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
					      phy_vars_ue->prach_resources[eNB_id]->ra_RNTI,
					      format1A,
					      &phy_vars_ue->dlsch_ue_ra[eNB_id], 
					      &phy_vars_ue->lte_frame_parms,
					      phy_vars_ue->pdsch_config_dedicated,
					      SI_RNTI,
					      phy_vars_ue->prach_resources[eNB_id]->ra_RNTI,
					      P_RNTI)==0) {
	  
	  phy_vars_ue->dlsch_ra_received[eNB_id]++;
	  
#ifdef DEBUG_PHY_PROC
	  LOG_I(PHY,"[UE  %d] Generate UE DLSCH RA_RNTI format 1A, rb_alloc %x, dlsch_ue_ra[eNB_id] %p\n",
	      phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->rb_alloc[0],phy_vars_ue->dlsch_ue_ra[eNB_id]);
#endif
	}
    }
      else if( (dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti) && 
	       (dci_alloc_rx[i].format == format0)) {
#ifdef DEBUG_PHY_PROC
	LOG_I(PHY,"[UE  %d][PUSCH] Frame %d subframe %d: Found rnti %x, format 0, dci_cnt %d\n",
	      phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1,dci_alloc_rx[i].rnti,i);
	/*
	  if (((phy_vars_ue->frame%100) == 0) || (phy_vars_ue->frame < 20))
	  dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
	*/
#endif      
#ifdef DIAG_PHY
	if ((last_slot>>1) != 9) {
	  LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received C_RNTI Format 0!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1);
	  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
          vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
	  return(-1);
	}
#endif
	
	phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;
	//dump_dci(&phy_vars_ue->lte_frame_parms,&dci_alloc_rx[i]);

	if (generate_ue_ulsch_params_from_dci((void *)&dci_alloc_rx[i].dci_pdu,
					      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
					      last_slot>>1,
					      format0,
					      phy_vars_ue,
					      SI_RNTI,
					      0,
					      P_RNTI,
					      CBA_RNTI,
					      eNB_id,
					      0)==0) {
	  
#ifdef DEBUG_PHY_PROC
	  LOG_D(PHY,"[UE  %d] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",phy_vars_ue->Mod_id,last_slot>>1);
#endif

	}
      } 
      else if( (dci_alloc_rx[i].rnti == phy_vars_ue->ulsch_ue[eNB_id]->cba_rnti[0]) && 
	       (dci_alloc_rx[i].format == format0)) {
	// UE could belong to more than one CBA group 
       // phy_vars_ue->Mod_id%phy_vars_ue->ulsch_ue[eNB_id]->num_active_cba_groups]
#ifdef DEBUG_PHY_PROC
	LOG_I(PHY,"[UE  %d][PUSCH] Frame %d subframe %d: Found cba rnti %x, format 0, dci_cnt %d\n",
	      phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1,dci_alloc_rx[i].rnti,i);
	/*
	  if (((phy_vars_ue->frame%100) == 0) || (phy_vars_ue->frame < 20))
	  dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
	*/
#endif      
	/*
#ifdef DIAG_PHY
	if ((last_slot>>1) != 8) {
	  LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received CBA RNTI Format 0!\n",
		phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1);
	  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
	  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
          vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
	  return(-1);
	}
#endif
	*/
	
	phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;
	//dump_dci(&phy_vars_ue->lte_frame_parms,&dci_alloc_rx[i]);

	if (generate_ue_ulsch_params_from_dci((void *)&dci_alloc_rx[i].dci_pdu,
					      phy_vars_ue->ulsch_ue[eNB_id]->cba_rnti[0],
					      last_slot>>1,
					      format0,
					      phy_vars_ue,
					      SI_RNTI,
					      0,
					      P_RNTI,
					      CBA_RNTI,
					      eNB_id,
					      0)==0) {
	  
#ifdef DEBUG_PHY_PROC
	  LOG_D(PHY,"[UE  %d] Generate UE ULSCH CBA_RNTI format 0 (subframe %d)\n",phy_vars_ue->Mod_id,last_slot>>1);
#endif
	  phy_vars_ue->ulsch_ue[eNB_id]->num_cba_dci[((last_slot>>1)+4)%10]++;
	}
      }
  
    else {
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[UE  %d] frame %d, subframe %d: received DCI %d with RNTI=%x (C-RNTI:%x, CBA_RNTI %x) and format %d!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1,i,dci_alloc_rx[i].rnti,
	    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
	    phy_vars_ue->ulsch_ue[eNB_id]->cba_rnti[0],
	    dci_alloc_rx[i].format);
      //      dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
#endif
#ifdef DIAG_PHY
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
      return(-1);
#endif
    }
    
  }
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
  return(0);
}

 
 int phy_procedures_UE_RX(u8 last_slot, PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag,runmode_t mode,
			  relaying_type_t r_type,PHY_VARS_RN *phy_vars_rn) {

  u16 l,m,n_symb;
  //  int eNB_id = 0, 
  int eNB_id_i = 1;
  u8 dual_stream_UE = 0;
  int ret=0;
  u8 harq_pid;
  int timing_advance;
  u8 pilot1,pilot2,pilot3;
  u8 i_mod = 0;
  int i;
#ifndef OPENAIR2
  u8 *rar;
#endif
  int pmch_flag=0;
  u8 sync_area=255;
  int pmch_mcs=-1;
  u8 mcch_active=0;
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_IN);
  start_meas(&phy_vars_ue->phy_proc_rx);
#ifdef DEBUG_PHY_PROC
  LOG_D(PHY,"[%s %d] Frame %d subframe %d: Doing phy_procedures_UE_RX(%d)\n", 
	(r_type == multicast_relay) ? "RN/UE" : "UE",
	phy_vars_ue->Mod_id,phy_vars_ue->frame, last_slot>>1, last_slot);
#endif
#ifdef EMOS
  if ((last_slot == 0)) {
    if (phy_vars_ue->frame%1024 == 0)
      openair_daq_vars.use_ia_receiver = 0;
    else
      openair_daq_vars.use_ia_receiver = (openair_daq_vars.use_ia_receiver+1)%3;
    LOG_I(PHY,"[MYEMOS] frame %d, IA receiver %d, MCS %d, bitrate %d\n",phy_vars_ue->frame,openair_daq_vars.use_ia_receiver, phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,phy_vars_ue->bitrate[eNB_id]);
  } 
#endif


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

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (subframe_select(&phy_vars_ue->lte_frame_parms,last_slot>>1) == SF_S) {
    if ((last_slot%2)==0)
      n_symb = 5;//3;
    else
      n_symb = 0;   	
  }
  else {
    if (is_pmch_subframe(phy_vars_ue->frame,last_slot>>1,&phy_vars_ue->lte_frame_parms)) {
      if ((last_slot%2)==0) {
	n_symb=2;
	pmch_flag=1;
      }
      else
	n_symb=0;
    }
    else
      n_symb = phy_vars_ue->lte_frame_parms.symbols_per_tti/2;
  }
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  // This is normal processing (i.e. not MBSFN)
  // RX processing of symbols in last_slot
  for (l=0;l<n_symb;l++) {
    if (abstraction_flag == 0) {
      start_meas(&phy_vars_ue->ofdm_demod_stats);
      slot_fep(phy_vars_ue,
	       l,
	       last_slot,
	       phy_vars_ue->rx_offset,
	       0);
      stop_meas(&phy_vars_ue->ofdm_demod_stats);
    }
  
    //if (subframe_select(&phy_vars_ue->lte_frame_parms,last_slot>>1) == SF_DL)
    lte_ue_measurement_procedures(last_slot,l,phy_vars_ue,eNB_id,abstraction_flag,mode);


    if ((last_slot==1) && (l==4-phy_vars_ue->lte_frame_parms.Ncp)) {

      /*
	phy_vars_ue->ulsch_no_allocation_counter[eNB_id]++;

	if (phy_vars_ue->ulsch_no_allocation_counter[eNB_id] == 10) {
	#ifdef DEBUG_PHY_PROC
	msg("[UE  %d] no_allocation : setting mode to PRACH\n",phy_vars_ue->Mod_id);
	#endif
	phy_vars_ue->UE_mode[eNB_id] = PRACH;
	phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti = 0x1234;
	}
      */

      lte_ue_pbch_procedures(eNB_id,last_slot,phy_vars_ue,abstraction_flag);

      /*
      if (phy_vars_ue->UE_mode[eNB_id] == RA_RESPONSE) {
      	phy_vars_ue->Msg3_timer[eNB_id]--;
      	msg("[UE RAR] frame %d: Msg3_timer %d\n",phy_vars_ue->frame,phy_vars_ue->Msg3_timer);
      
      	if (phy_vars_ue->Msg3_timer[eNB_id] == 0) {
      	  LOG_D(PHY,"[UE  %d] Frame %d: Msg3_timer = 0 : setting mode to PRACH\n",phy_vars_ue->Mod_id,phy_vars_ue->frame);
	  // I guess here we also need to tell the RRC
      	  phy_vars_ue->UE_mode[eNB_id] = PRACH;
	  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti = 0x1234;
	}
      }
      */
    }

#ifdef DLSCH_THREAD
    if (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1)  {
      // activate thread since Chest is now done for slot before last_slot
      if (l==0) {
          LOG_D(PHY,"frame %d, last_slot %d: Calling rx_pdsch_thread for harq_pid %d\n",phy_vars_ue->frame,last_slot, phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid);
          if (pthread_mutex_lock (&rx_pdsch_mutex) != 0) {               // Signal MAC_PHY Scheduler
              LOG_E(PHY,"[UE  %d] ERROR pthread_mutex_lock\n",phy_vars_ue->Mod_id);     // lock before accessing shared resource
              vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
          } else {
              rx_pdsch_instance_cnt++;
              (last_slot == 0) ? (rx_pdsch_slot = 19) : (rx_pdsch_slot = (last_slot-1));
              pthread_mutex_unlock (&rx_pdsch_mutex);
              
              if (rx_pdsch_instance_cnt == 0) {
                  if (pthread_cond_signal(&rx_pdsch_cond) != 0) {
                      LOG_E(PHY,"[UE  %d] ERROR pthread_cond_signal for rx_pdsch_cond\n",phy_vars_ue->Mod_id);
                      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
                  }
              }
              else {
                  LOG_W(PHY,"[UE  %d] Frame=%d, Slot=%d, RX_PDSCH thread for rx_pdsch_thread busy!!!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot);
                  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
              }
          }
          // trigger DLSCH decoding thread
          if (!(last_slot%2)) // odd slots
              phy_vars_ue->dlsch_ue[eNB_id][0]->active = 0;
      }
    }
#endif
  
    // process last DLSCH symbols + invoke decoding
    if (((last_slot%2)==0) && (l==0)) {
      // Regular PDSCH
      if (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1) {
#ifndef DLSCH_THREAD //USER_MODE
	harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid;
	//	printf("PDSCH active in subframe %d, harq_pid %d\n",(last_slot>>1)-1,harq_pid); 
	if ((phy_vars_ue->transmission_mode[eNB_id] == 5) && 
	    (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
	    (openair_daq_vars.use_ia_receiver ==1)) {
	  dual_stream_UE = 1;
	  eNB_id_i = phy_vars_ue->n_connected_eNB;
	  i_mod = get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
      if (phy_vars_ue->frame%100==0) {
          LOG_I(PHY,"using IA receiver\n");
      }
	}
	else {
	  dual_stream_UE = 0;
	  eNB_id_i = eNB_id+1;
	  i_mod = 0;
	}

	// process symbols 10,11,12 and trigger DLSCH decoding
	if (abstraction_flag == 0) {
	  
	  start_meas(&phy_vars_ue->dlsch_llr_stats);
	  for (m=pilot3;m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++) {
	    rx_pdsch(phy_vars_ue,
		     PDSCH,
		     eNB_id,
		     eNB_id_i,
		     (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),  // subframe
		     m,                    // symbol
		     0,                    // first_symbol_flag
		     dual_stream_UE,
		     i_mod,
		     phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid);
	  }
	  stop_meas(&phy_vars_ue->dlsch_llr_stats);
	}
	
	phy_vars_ue->dlsch_ue[eNB_id][0]->active = 0;

#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d Scheduling DLSCH decoding\n",
	      phy_vars_ue->Mod_id,
	      phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
	      harq_pid,
	      phy_vars_ue->frame,last_slot>>1);      
#endif	
       
	if (phy_vars_ue->dlsch_ue[eNB_id][0]) {
        if (abstraction_flag == 0) {
            phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->G = get_G(&phy_vars_ue->lte_frame_parms,
                                                                                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->nb_rb,
                                                                                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rb_alloc,
                                                                                  get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs),
                                                                                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                                                                                  phy_vars_ue->frame,(((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)));
	    start_meas(&phy_vars_ue->dlsch_unscrambling_stats);
	    dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
			       0,
			       phy_vars_ue->dlsch_ue[0][0],
			       phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->G,
			       phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],
			       0,
			       ((((last_slot>>1)==0) ? 9 : ((last_slot>>1))-1))<<1);
	    start_meas(&phy_vars_ue->dlsch_unscrambling_stats);

	    start_meas(&phy_vars_ue->dlsch_decoding_stats);
	    ret = dlsch_decoding(phy_vars_ue,
				 phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],
				 &phy_vars_ue->lte_frame_parms,
				 phy_vars_ue->dlsch_ue[eNB_id][0],
				 phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid],
				 (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
				 harq_pid,
				 1,0);
	    stop_meas(&phy_vars_ue->dlsch_decoding_stats);
	}

	  else {
	    LOG_D(PHY,"Calling dlsch_decoding_emul ...\n");
#ifdef PHY_ABSTRACTION
	    ret = dlsch_decoding_emul(phy_vars_ue,
				      (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
				      2,
				      eNB_id);
#endif
	  }

	  if (ret == (1+phy_vars_ue->dlsch_ue[eNB_id][0]->max_turbo_iterations)) {
	    phy_vars_ue->dlsch_errors[eNB_id]++;
	  
#ifdef DEBUG_PHY_PROC
	    LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d DLSCH in error (rv %d,mcs %d)\n",
		phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
		harq_pid,phy_vars_ue->frame,last_slot>>1,
		phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
		phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
	    
	    //	    if (abstraction_flag ==0 )
	    //	      dump_dlsch(phy_vars_ue,eNB_id,(((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),harq_pid);
#endif
	  }
	  else {
	    LOG_I(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d: Received DLSCH (rv %d,mcs %d,TBS %d)\n",
		  phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
		  harq_pid,phy_vars_ue->frame,last_slot>>1,
		  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
		  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
		  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS);
#ifdef DEBUG_PHY_PROC	 
#ifdef DEBUG_DLSCH
	    int j;
	    LOG_D(PHY,"dlsch harq_pid %d (rx): \n",phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid);
	    for (j=0;j<phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->TBS>>3;j++)
	      LOG_T(PHY,"%x.",phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->b[j]);
	    LOG_T(PHY,"\n");
#endif 
#endif	    
#ifdef OPENAIR2
	    mac_xface->ue_send_sdu(phy_vars_ue->Mod_id,
				   phy_vars_ue->frame,
				   phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->b,
				   phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->TBS>>3,
				   eNB_id);
#endif
	    phy_vars_ue->total_TBS[eNB_id] =  phy_vars_ue->total_TBS[eNB_id] +
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->TBS;
	    phy_vars_ue->total_received_bits[eNB_id] = phy_vars_ue->total_TBS[eNB_id] +
	      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->TBS;
	  }
	}
            

#ifdef DEBUG_PHY_PROC
	LOG_I(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d: PDSCH/DLSCH decoding iter %d (mcs %d, rv %d, TBS %d)\n",
	    phy_vars_ue->Mod_id,
	    phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,harq_pid,
	    phy_vars_ue->frame,last_slot>>1,ret,
	    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
	    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
	    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS);
	if (phy_vars_ue->frame%100==0) {
	  LOG_I(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d dlsch_errors %d, dlsch_received %d, dlsch_fer %d, current_dlsch_cqi %d\n",
		phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
		phy_vars_ue->frame,last_slot>>1,
		phy_vars_ue->dlsch_errors[eNB_id],
		phy_vars_ue->dlsch_received[eNB_id],
		phy_vars_ue->dlsch_fer[eNB_id],
		phy_vars_ue->PHY_measurements.wideband_cqi_tot[eNB_id]);
	}
#endif 
#endif //DLSCH_THREAD
      }
      else {
	//	printf("PDSCH inactive in subframe %d\n",(last_slot>>1)-1); 
	phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[(((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1))].send_harq_status = 0;
      }
    
      // SI_DLSCH
      if (phy_vars_ue->dlsch_ue_SI[eNB_id]->active == 1) {
#ifdef DEBUG_PHY_PROC
	LOG_I(PHY,"SI is active\n");
#endif
	
	// process symbols 10,11,12 (13) of last SF and trigger DLSCH decoding
	if (abstraction_flag==0) {
	  start_meas(&phy_vars_ue->dlsch_llr_stats);
	  for (m=pilot3;m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++) {
#ifdef DEBUG_PHY_PROC
	    /*
	      debug_LOG_D(PHY,"[UE  %d] Frame %d, slot %d: DLSCH (SI) demod between pilot 3 and 4 (2nd slot), m %d\n",
	      phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot,m);
	    */
#endif
	    rx_pdsch(phy_vars_ue,
		     SI_PDSCH,
		     eNB_id,
		     eNB_id+1,
		     last_slot>>1,  // subframe,
		     m,
		     0,
		     0,
             phy_vars_ue->is_secondary_ue,
             phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid);
	  }
	  stop_meas(&phy_vars_ue->dlsch_llr_stats);
	}
	//	write_output("dlsch_ra_llr.m","llr",lte_ue_pdsch_vars_ra[eNB_id]->llr[0],40,1,0);

	phy_vars_ue->dlsch_ue_SI[eNB_id]->active = 0;
      
	if (phy_vars_ue->frame < phy_vars_ue->dlsch_SI_errors[eNB_id])
	  phy_vars_ue->dlsch_SI_errors[eNB_id]=0;

	if (phy_vars_ue->dlsch_ue_SI[eNB_id]) {

	  if (abstraction_flag==0) {

	    //          dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
	    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G =  
	      get_G(&phy_vars_ue->lte_frame_parms,
		    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
		    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc,
		    get_Qm(phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs),
		    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
		    phy_vars_ue->frame,(((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)));
	    
#ifdef DEBUG_PHY_PROC
	    LOG_I(PHY,"Decoding DLSCH_SI : rb_alloc %x : nb_rb %d G %d TBS %d\n",phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc[0],
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G,
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS);
#endif


	    dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
                           0,
                           phy_vars_ue->dlsch_ue_SI[eNB_id],
                           phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G,
                           phy_vars_ue->lte_ue_pdsch_vars_SI[eNB_id]->llr[0],
                           0,
                           ((((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)))<<1);
	    
	    ret = dlsch_decoding(phy_vars_ue,
				 phy_vars_ue->lte_ue_pdsch_vars_SI[eNB_id]->llr[0],
				 &phy_vars_ue->lte_frame_parms,
				 phy_vars_ue->dlsch_ue_SI[eNB_id],
				 phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0],
				 (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
				 harq_pid,
				 0,0);

#ifdef DEBUG_PHY_PROC
	    for (i=0;i<11;i++)
              LOG_D(PHY,"dlsch_output_buffer[%d]=%x\n",i,phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->c[0][i]);
#endif

	  }
#ifdef PHY_ABSTRACTION
	  else {
	    LOG_D(PHY,"Calling dlsch_decoding_emul ...\n");
	    ret = dlsch_decoding_emul(phy_vars_ue,
				      (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
				      0,
				      eNB_id);
	  }
#endif

	  if (ret == (1+phy_vars_ue->dlsch_ue_SI[eNB_id]->max_turbo_iterations)) {
	    phy_vars_ue->dlsch_SI_errors[eNB_id]++;
#ifdef DEBUG_PHY_PROC
	    LOG_I(PHY,"[UE  %d] Frame %d, subframe %d, received SI in error\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,((last_slot==0)?9 : ((last_slot>>1)-1)));
#endif

#ifdef USER_MODE
	    //	    dump_dlsch_SI(phy_vars_ue,eNB_id,(((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)));
#endif
            vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
	    stop_meas(&phy_vars_ue->phy_proc_rx);
	    return(-1);
	  }
	  else {


#ifdef DEBUG_PHY_PROC
	    //if ((phy_vars_ue->frame % 100) == 0)
	    LOG_I(PHY,"[UE  %d] Frame %d, subframe %d, received SI for TBS %d\n",
		  phy_vars_ue->Mod_id,phy_vars_ue->frame,(((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS);
#endif

#ifdef OPENAIR2
        /*
		for (i=0;i<phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS>>3;i++)
		  printf("%x.",phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->b[i]);
		printf("\n");
        */
		mac_xface->ue_decode_si(phy_vars_ue->Mod_id,
					phy_vars_ue->frame,
					eNB_id,
					phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->b,
					phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS>>3);
		/*
	    if ((phy_vars_ue->frame % 160) < 10)
	      printf("sending SI to L2 in frame %d\n",phy_vars_ue->frame);
	    */
#endif
	  }
	}   
	/*
#ifdef DEBUG_PHY_PROC
	debug_LOG_D(PHY,"[UE  %d] Frame %d, slot %d: dlsch_decoding (SI) ret %d (%d errors)\n",
		  phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot,ret,phy_vars_ue->dlsch_SI_errors[eNB_id]);
#endif
	*/
      }
    

      if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active == 1) {
#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[UE  %d] Frame %d, slot %d: DLSCH (RA) demod symbols 10,11,12\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot);
#endif
      
	// process symbols 10,11,12 and trigger DLSCH decoding
	if (abstraction_flag==0) {
	  start_meas(&phy_vars_ue->dlsch_llr_stats);
	  for (m=pilot3;m<phy_vars_ue->lte_frame_parms.symbols_per_tti;m++)
	    rx_pdsch(phy_vars_ue,
		     RA_PDSCH,
		     eNB_id,
		     eNB_id+1,
		     last_slot>>1,  // subframe,
		     m, // symbol
		     0, // first_symbol_flag
		     0,
             phy_vars_ue->is_secondary_ue,
             phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid);
	}
	stop_meas(&phy_vars_ue->dlsch_llr_stats);

	phy_vars_ue->dlsch_ue_ra[eNB_id]->active = 0;
	
	if (phy_vars_ue->frame < phy_vars_ue->dlsch_ra_errors[eNB_id])
	  phy_vars_ue->dlsch_ra_errors[eNB_id]=0;

	if (phy_vars_ue->prach_resources[eNB_id]!=NULL)
	  phy_vars_ue->dlsch_ue_ra[eNB_id]->rnti = phy_vars_ue->prach_resources[eNB_id]->ra_RNTI;
	else {
	  LOG_E(PHY,"[UE %d] Frame %d, subframe %d: FATAL, prach_resources is NULL\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot>>1);
	  mac_xface->macphy_exit("");
	  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
	  stop_meas(&phy_vars_ue->phy_proc_rx);
	  return 0;
	}

	if (abstraction_flag==0) {
        phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G = get_G(&phy_vars_ue->lte_frame_parms,
                                                                       phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,
                                                                       phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->rb_alloc,
                                                                       get_Qm(phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->mcs),
                                                                       phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                                                                       phy_vars_ue->frame,
                                                                       (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)));

#ifdef DEBUG_PHY_PROC
        LOG_I(PHY,"[UE] decoding RA (subframe %d): G %d,rnti %x\n" ,last_slot>>1,
              phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G,
              phy_vars_ue->dlsch_ue_ra[eNB_id]->rnti);
#endif

        dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
                           0,
                           phy_vars_ue->dlsch_ue_ra[eNB_id],
                           phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G,
                           phy_vars_ue->lte_ue_pdsch_vars_ra[eNB_id]->llr[0],
                           0,
                           ((((last_slot>>1)==0) ?9 : ((last_slot>>1)-1)))<<1);
        
        ret = dlsch_decoding(phy_vars_ue,
                             phy_vars_ue->lte_ue_pdsch_vars_ra[eNB_id]->llr[0],
                             &phy_vars_ue->lte_frame_parms,
                             phy_vars_ue->dlsch_ue_ra[eNB_id],
                             phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0],
                             (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),  // subframe
                             harq_pid,
                             0,0);
	}

#ifdef PHY_ABSTRACTION
	else {
	  LOG_D(PHY,"Calling dlsch_decoding_emul ...\n");
	  ret = dlsch_decoding_emul(phy_vars_ue,
				    (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
				    1,
				    eNB_id);
	}
#endif

	if (ret == (1+phy_vars_ue->dlsch_ue_ra[eNB_id]->max_turbo_iterations)) {
	  phy_vars_ue->dlsch_ra_errors[eNB_id]++;
	  LOG_D(PHY,"[UE  %d] Frame %d, subframe %d, received RA in error\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,((last_slot==0)?9 : ((last_slot>>1)-1)));
#ifdef USER_MODE
	  //dump_dlsch_ra(phy_vars_ue,eNB_id,(((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)));
#endif
	  //	  oai_exit=1;
          vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
	  stop_meas(&phy_vars_ue->phy_proc_rx);
	  return(-1);

	}
	else {
#ifdef DEBUG_PHY_PROC
	  LOG_I(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Received RAR  mode %d\n",
		phy_vars_ue->Mod_id,
		phy_vars_ue->frame-(((last_slot>>1)==0) ? 1 : 0),
		(((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)), phy_vars_ue->UE_mode[eNB_id]);
#endif	  

#ifdef OPENAIR2
	  if ((phy_vars_ue->UE_mode[eNB_id] != PUSCH) && (phy_vars_ue->prach_resources[eNB_id]->Msg3!=NULL)) {
	    LOG_I(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Invoking MAC for RAR (current preamble %d)\n",
		  phy_vars_ue->Mod_id,phy_vars_ue->frame-(((last_slot>>1)==0) ? 1 : 0),
		  (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
		  phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);			

	    timing_advance = mac_xface->ue_process_rar(phy_vars_ue->Mod_id,
						       phy_vars_ue->frame-(((last_slot>>1)==0) ? 1 : 0),
						       phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b,
						       &phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
						       phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);


	    if (timing_advance!=0xffff) {

	      LOG_I(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Got rnti %x and timing advance %d from RAR\n",
		    phy_vars_ue->Mod_id,
		    phy_vars_ue->frame-(((last_slot>>1)==0) ? 1 : 0),
		    (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
		    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
		    timing_advance);		
	      
	      //timing_advance = 0;
	      process_timing_advance_rar(phy_vars_ue,timing_advance);
	      if (mode!=debug_prach) {
		phy_vars_ue->ulsch_ue_Msg3_active[eNB_id]=1;
		get_Msg3_alloc(&phy_vars_ue->lte_frame_parms,
			       (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
			       phy_vars_ue->frame-(((last_slot>>1)==0) ? 1 : 0),
			       &phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
			       &phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
		
		LOG_I(PHY,"[UE  %d][RAPROC] Got Msg3_alloc Frame %d subframe %d: Msg3_frame %d, Msg3_subframe %d\n",
		      phy_vars_ue->Mod_id,
		      phy_vars_ue->frame-(((last_slot>>1)==0) ? 1 : 0),
		      (((last_slot>>1)==0) ? 9 : ((last_slot>>1)-1)),
		      phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
		      phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
		harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,
					     phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
					     phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
		phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round = 0;

		phy_vars_ue->UE_mode[eNB_id] = RA_RESPONSE;
		//	    phy_vars_ue->Msg3_timer[eNB_id] = 10;
		phy_vars_ue->ulsch_ue[eNB_id]->power_offset = 6;
		phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;
	      }
	    }
	    else {  // PRACH preamble doesn't match RAR
	      LOG_W(PHY,"[UE  %d][RAPROC] Received RAR preamble (%d) doesn't match !!!\n",
		  phy_vars_ue->Mod_id,
		  phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);
	    }
	  } // mode != PUSCH
#else //OPENAIR2

	  rar = phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b+1;
	  timing_advance = ((((uint16_t)(rar[0]&0x7f))<<4) + (rar[1]>>4));
	  //timing_advance = phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b[0];
	  process_timing_advance_rar(phy_vars_ue,timing_advance);
#endif
	} //ret <= MAX_ITERATIONS
	/*      
#ifdef DEBUG_PHY_PROC	
	debug_LOG_D(PHY,"[UE  %d] Frame %d, slot %d: dlsch_decoding (RA) ret %d (%d errors)\n",
		  phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot,ret,phy_vars_ue->dlsch_ra_errors[eNB_id]);
#endif	
	*/
      } // dlsch_ue_ra[eNB_id]->active == 1
      
    }


    if ((((last_slot%2)==0) && ((l==pilot1))) ||
	((pmch_flag==1)&&(l==1)))  {
      
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[UE  %d] Frame %d, slot %d: Calling pdcch procedures (eNB %d)\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot,eNB_id);
#endif
      //      rt_printk("[PDCCH] Frame %d, slot %d, start %llu\n",phy_vars_ue->frame,last_slot,rt_get_time_ns());
      if (lte_ue_pdcch_procedures(eNB_id,last_slot,phy_vars_ue,abstraction_flag) == -1) {
#ifdef DEBUG_PHY_PROC
	LOG_E(PHY,"[UE  %d] Frame %d, slot %d: Error in pdcch procedures\n",phy_vars_ue->Mod_id,phy_vars_ue->frame,last_slot);
#endif
	return(-1);
      }
      //      rt_printk("[PDCCH] Frame %d, slot %d, stop  %llu\n",phy_vars_ue->frame,last_slot,rt_get_time_ns());
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"num_pdcch_symbols %d\n",phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
#endif
    }
    
    if (abstraction_flag==0) {

      if (((last_slot%2)==1) && (l==0)) {
	
#ifndef DLSCH_THREAD
        if (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1)  {
            harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid;
            if ((phy_vars_ue->transmission_mode[eNB_id] == 5) && 
                (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
                (openair_daq_vars.use_ia_receiver ==1)) {
                dual_stream_UE = 1;
                eNB_id_i = phy_vars_ue->n_connected_eNB;
                i_mod =  get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
            }
            else {
                dual_stream_UE = 0;
                eNB_id_i = eNB_id+1;
                i_mod = 0;
            }
	    // process DLSCH received in first slot
	  
	    start_meas(&phy_vars_ue->dlsch_llr_stats);
	    for (m=phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols;
		 m<pilot2;
		 m++) {      
	  
	      rx_pdsch(phy_vars_ue,
		       PDSCH,
		       eNB_id,
		       eNB_id_i,
		       last_slot>>1,  // subframe,
		       m,
		       (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
		       dual_stream_UE,
		       i_mod,
		       phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid);
	    } 
	    stop_meas(&phy_vars_ue->dlsch_llr_stats);
	}// CRNTI active

#endif	  

	  if (phy_vars_ue->dlsch_ue_SI[eNB_id]->active == 1)  {
	    // process SI DLSCH in first slot
	    for (m=phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols;
		 m<pilot2;
		 m++) {      
	  
	    rx_pdsch(phy_vars_ue,
		     SI_PDSCH,
		     eNB_id,
		     eNB_id+1,
		     last_slot>>1,  // subframe,
		     m,
		     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
		     0,
             phy_vars_ue->is_secondary_ue,
             phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid);
	    }
	  } // SI active
	  
	  if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active == 1)  {
	    for (m=phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols;
		 m<pilot2;
		 m++) {      
	  
	    rx_pdsch(phy_vars_ue,
		     RA_PDSCH,
		     eNB_id,
		     eNB_id+1,
		     last_slot>>1,  // subframe,
		     m,
		     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,
		     0,
             phy_vars_ue->is_secondary_ue,
             phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid);
	    } // loop from first dlsch symbol to end of slot
	  } // RA active
      } // 2nd quarter
    
      if (((last_slot%2)==1) && (l==pilot1)) {
#ifndef DLSCH_THREAD
        if (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1) {
            harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid;
            if ((phy_vars_ue->transmission_mode[eNB_id] == 5) && 
                (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
                (openair_daq_vars.use_ia_receiver ==1)) {
                dual_stream_UE = 1;
                eNB_id_i = phy_vars_ue->n_connected_eNB;
                i_mod = get_Qm(phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);
            }
            else {
                dual_stream_UE = 0;
                eNB_id_i = eNB_id+1;
                i_mod = 0;
            }

	    start_meas(&phy_vars_ue->dlsch_llr_stats);
	    for (m=pilot2;m<pilot3;m++) {
	      rx_pdsch(phy_vars_ue,
		       PDSCH,
		       eNB_id,
		       eNB_id_i,
		       last_slot>>1,  // subframe,
		       m,
		       0,
		       dual_stream_UE,
		       i_mod,
		       phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid);
	    }
	    stop_meas(&phy_vars_ue->dlsch_llr_stats);
	} // CRNTI active
#endif	  
	  if(phy_vars_ue->dlsch_ue_SI[eNB_id]->active == 1) {
	    for (m=pilot2;m<pilot3;m++) {
	    rx_pdsch(phy_vars_ue,
		     SI_PDSCH,
		     eNB_id,
		     eNB_id+1,
		     last_slot>>1,  // subframe,
		     m,
		     0,   // first_symbol_flag
		     0,
             phy_vars_ue->is_secondary_ue,
             phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid);
	    }
	  } // SI active
	  
	  if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active == 1) {
	    for (m=pilot2;m<pilot3;m++) {
	    rx_pdsch(phy_vars_ue,
		     RA_PDSCH,
		     eNB_id,
		     eNB_id+1,
		     last_slot>>1,  // subframe,
		     m,
		     0,   // first_symbol_flag
		     0,
             phy_vars_ue->is_secondary_ue,
             phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid);
	    } // loop over 3rd quarter
	  } // RA active
	  
      } // 3rd quarter of subframe
    } // abstraction_flag==0   
  }// l loop

  // calculate some statistics
  if (last_slot==19) {
    if (phy_vars_ue->frame % 10 == 0) {
      if ((phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]) != 0) 
	phy_vars_ue->dlsch_fer[eNB_id] = (100*(phy_vars_ue->dlsch_errors[eNB_id] - phy_vars_ue->dlsch_errors_last[eNB_id]))/(phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]);
      
      phy_vars_ue->dlsch_errors_last[eNB_id] = phy_vars_ue->dlsch_errors[eNB_id];
      phy_vars_ue->dlsch_received_last[eNB_id] = phy_vars_ue->dlsch_received[eNB_id];
    }
    
    phy_vars_ue->bitrate[eNB_id] = (phy_vars_ue->total_TBS[eNB_id] - phy_vars_ue->total_TBS_last[eNB_id])*100;
    phy_vars_ue->total_TBS_last[eNB_id] = phy_vars_ue->total_TBS[eNB_id];
    LOG_D(PHY,"[UE %d] Calculating bitrate Frame %d: total_TBS = %d, total_TBS_last = %d, bitrate %f kbits\n",
	  phy_vars_ue->Mod_id,phy_vars_ue->frame,phy_vars_ue->total_TBS[eNB_id],
	  phy_vars_ue->total_TBS_last[eNB_id],(float) phy_vars_ue->bitrate[eNB_id]/1000.0);
  
    if ((phy_vars_ue->frame % 100 == 0)) {
      LOG_I(PHY,"Throughput %5.1f kbps\n",(float) phy_vars_ue->bitrate[eNB_id]/1000.0);
    }
  }
  
  if (is_pmch_subframe(((last_slot>>1)==9?-1:0)+phy_vars_ue->frame,last_slot>>1,&phy_vars_ue->lte_frame_parms)) {
 
    if ((last_slot%2)==1) {
      LOG_D(PHY,"[UE %d] Frame %d, subframe %d: Querying for PMCH demodulation(%d)\n",
	    phy_vars_ue->Mod_id,((last_slot>>1)==9?-1:0)+phy_vars_ue->frame,last_slot>>1,last_slot);
#ifdef Rel10
      pmch_mcs = mac_xface->ue_query_mch(phy_vars_ue->Mod_id,((last_slot>>1)==9?-1:0)+phy_vars_ue->frame,last_slot>>1,eNB_id,&sync_area,&mcch_active);
      if (phy_vars_rn)
	phy_vars_rn->mch_avtive[last_slot>>1]=0;
#else
      pmch_mcs=-1;
#endif
      
      if (pmch_mcs>=0) {
	LOG_D(PHY,"[UE %d] Frame %d, subframe %d: Programming PMCH demodulation for mcs %d\n",phy_vars_ue->Mod_id,((last_slot>>1)==9?-1:0)+phy_vars_ue->frame,last_slot>>1,pmch_mcs);
	fill_UE_dlsch_MCH(phy_vars_ue,pmch_mcs,1,0,0);
	
	for (l=2;l<12;l++) {
	  
	  slot_fep_mbsfn(phy_vars_ue,
			 l,
			 last_slot>>1,
			 0,0);//phy_vars_ue->rx_offset,0);
	}
	
	for (l=2;l<12;l++) {
	  rx_pmch(phy_vars_ue,
		  0,
		  last_slot>>1,
		  l);
	  
	  
	}
	/*	printf("PMCH decoding, Frame %d, subframe %d, G %d\n", 
	       ((last_slot>>1)==9?-1:0)+phy_vars_ue->frame,
	       last_slot>>1,
	       phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G);
	*/
	phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G = get_G(&phy_vars_ue->lte_frame_parms,
								   phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->nb_rb,
								   phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->rb_alloc,
								   get_Qm(phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->mcs),
								   2,
								   ((last_slot>>1)==9?-1:0)+phy_vars_ue->frame,last_slot>>1);
					
	dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,1,phy_vars_ue->dlsch_ue_MCH[0],
			   phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G,
			   phy_vars_ue->lte_ue_pdsch_vars_MCH[0]->llr[0],0,last_slot-1);

	ret = dlsch_decoding(phy_vars_ue,
			     phy_vars_ue->lte_ue_pdsch_vars_MCH[0]->llr[0],		 
			     &phy_vars_ue->lte_frame_parms,
			     phy_vars_ue->dlsch_ue_MCH[0],
			     phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0],
			     last_slot>>1,
			     0,
			     0,0);    
	if (mcch_active == 1)
	  phy_vars_ue->dlsch_mcch_trials[sync_area][0]++;
	else 
	  phy_vars_ue->dlsch_mtch_trials[sync_area][0]++;
	
	if (ret == (1+phy_vars_ue->dlsch_ue_MCH[0]->max_turbo_iterations)) {
	  if (mcch_active == 1)
	    phy_vars_ue->dlsch_mcch_errors[sync_area][0]++;
	  else 
	    phy_vars_ue->dlsch_mtch_errors[sync_area][0]++;
	  LOG_D(PHY,"[%s %d] Frame %d, subframe %d: PMCH in error (%d,%d), not passing to L2 (TBS %d, iter %d,G %d)\n",
		(r_type == no_relay)? "UE": "RN/UE", phy_vars_ue->Mod_id,
		((last_slot>>1)==9?-1:0)+phy_vars_ue->frame,last_slot>>1,
		phy_vars_ue->dlsch_mcch_errors[sync_area][0],
		phy_vars_ue->dlsch_mtch_errors[sync_area][0],
		phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3,
		phy_vars_ue->dlsch_ue_MCH[0]->max_turbo_iterations,
		phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G);
	  dump_mch(phy_vars_ue,0,phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G,(last_slot>>1));
#ifdef DEBUG_DLSCH	  
	  for (i=0;i<phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3;i++){
	    LOG_T(PHY,"%2x.",phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->c[0][i]);
	  }
	  LOG_T(PHY,"\n");
#endif 
	  if ((last_slot>>1)==9) exit(-1);
	}
	else {
#ifdef Rel10
	  if ((r_type == no_relay) || (mcch_active == 1)) {
	    mac_xface->ue_send_mch_sdu(phy_vars_ue->Mod_id,
				       ((last_slot>>1)==9?-1:0)+phy_vars_ue->frame,
				       phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->b,
				       phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3,
				       eNB_id,// not relevant in eMBMS context
				       sync_area);
	    /*   for (i=0;i<phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3;i++)
	      msg("%2x.",phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->b[i]); 
	    msg("\n");
	    */
	    
	    if (mcch_active == 1)
	      phy_vars_ue->dlsch_mcch_received[sync_area][0]++;
	    else 
	      phy_vars_ue->dlsch_mtch_received[sync_area][0]++;

	   
	    if (phy_vars_ue->dlsch_mch_received_sf[(last_slot>>1)%5][0] == 1 ){
	      phy_vars_ue->dlsch_mch_received_sf[(last_slot>>1)%5][0]=0;
	    } else {
	      phy_vars_ue->dlsch_mch_received[0]+=1;  
	      phy_vars_ue->dlsch_mch_received_sf[last_slot>>1][0]=1;
	    }

	  } else if (r_type == multicast_relay) { // mcch is not active here 
	    // only 1 harq process exists
	    // Fix me: this could be a pointer copy
	    memcpy (phy_vars_rn->dlsch_rn_MCH[last_slot>>1]->harq_processes[0]->b,
		    phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->b,
		    phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3);
	    // keep the tbs
	    phy_vars_rn->mch_avtive[last_slot>>1] = 1;
	    phy_vars_rn->sync_area[last_slot>>1] = sync_area; // this could also go the harq data struct
	    phy_vars_rn->dlsch_rn_MCH[last_slot>>1]->harq_processes[0]->TBS = phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS;
	    phy_vars_rn->dlsch_rn_MCH[last_slot>>1]->harq_processes[0]->mcs = phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->mcs;
	    LOG_I(PHY,"[RN/UE %d] Frame %d subframe %d: store the MCH PDU for MBSFN sync area %d (MCS %d, TBS %d)\n",
		  phy_vars_ue->Mod_id,frame,last_slot>>1,sync_area,
		  phy_vars_rn->dlsch_rn_MCH[last_slot>>1]->harq_processes[0]->mcs,
		  phy_vars_rn->dlsch_rn_MCH[last_slot>>1]->harq_processes[0]->TBS>>3);
#ifdef DEBUG_PHY
	    for (i=0;i<phy_vars_rn->dlsch_rn_MCH[last_slot>>1]->harq_processes[0]->TBS>>3;i++)
	      msg("%2x.",phy_vars_rn->dlsch_rn_MCH[last_slot>>1]->harq_processes[0]->b[i]); 
	    msg("\n");
#endif 	 
	  } else 
	    LOG_W(PHY,"[UE %d] Frame %d: not supported option\n",phy_vars_ue->Mod_id,frame);
#endif
	}
      }
    }
  }
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
  stop_meas(&phy_vars_ue->phy_proc_rx);
  return (0);
 }

#ifdef Rel10
int phy_procedures_RN_UE_RX(u8 last_slot, u8 next_slot, relaying_type_t r_type) {

  int do_proc =0; // do nothing by default 
  switch(r_type){
  case no_relay:
    do_proc=no_relay; // perform the normal UE operation 
    break;
  case multicast_relay:
    if (last_slot > 12)
      do_proc = 0; // do nothing
    else // SF#1, SF#2, SF3, SF#3, SF#4, SF#5, SF#6(do rx slot 12)
      do_proc = multicast_relay ; // do PHY procedures UE RX 
    break;
  default: // should'not be here
    LOG_W(PHY,"Not supported relay type %d, do nothing \n", r_type);
    do_proc= 0;
    break;
  }
  return do_proc;
}
#endif   
 void phy_procedures_UE_lte(u8 last_slot, u8 next_slot, PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 abstraction_flag,runmode_t mode, 
			    relaying_type_t r_type, PHY_VARS_RN *phy_vars_rn) {

#undef DEBUG_PHY_PROC

#ifdef OPENAIR2
  UE_L2_STATE_t ret;
#endif
#ifndef OPENAIR2
  phy_vars_ue->UE_mode[eNB_id]=PUSCH;
  phy_vars_ue->prach_resources[eNB_id] = &prach_resources_local;
  prach_resources_local.ra_RNTI = 0xbeef;
  prach_resources_local.ra_PreambleIndex = 0;
#endif

  vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLES_SLOT_NUMBER, (last_slot + 1) % 20);
  vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER, phy_vars_ue->frame);

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_LTE,1);
#ifdef EXMIMO
#ifndef USRP
  vcd_signal_dumper_dump_variable_by_name(VCD_SIGNAL_DUMPER_VARIABLES_DAQ_MBOX, *((volatile unsigned int *) openair0_exmimo_pci[card].rxcnt_ptr[0]));
#endif
#endif

  start_meas(&phy_vars_ue->phy_proc);

  if ((subframe_select(&phy_vars_ue->lte_frame_parms,next_slot>>1)==SF_UL)||
      (phy_vars_ue->lte_frame_parms.frame_type == 0)){
    phy_procedures_UE_TX(next_slot,phy_vars_ue,eNB_id,abstraction_flag,mode,r_type);
  }
  if ((subframe_select(&phy_vars_ue->lte_frame_parms,last_slot>>1)==SF_DL) ||
      (phy_vars_ue->lte_frame_parms.frame_type == 0)){
#ifdef Rel10 
    if (phy_procedures_RN_UE_RX(last_slot, next_slot, r_type) != 0 )
#endif 
      phy_procedures_UE_RX(last_slot,phy_vars_ue,eNB_id,abstraction_flag,mode,r_type,phy_vars_rn);
#ifdef EMOS
    phy_procedures_emos_UE_RX(phy_vars_ue,last_slot,eNB_id);
#endif
  }
  if ((subframe_select(&phy_vars_ue->lte_frame_parms,next_slot>>1)==SF_S) &&
      ((next_slot&1)==1)) {
    phy_procedures_UE_S_TX(next_slot,phy_vars_ue,eNB_id,abstraction_flag,r_type);
  }
  if ((subframe_select(&phy_vars_ue->lte_frame_parms,last_slot>>1)==SF_S) &&
      ((last_slot&1)==0)) {
 #ifdef Rel10 
    if (phy_procedures_RN_UE_RX(last_slot, next_slot, r_type) != 0 )
#endif 
      phy_procedures_UE_RX(last_slot,phy_vars_ue,eNB_id,abstraction_flag,mode,r_type,phy_vars_rn);
  }

#ifdef OPENAIR2
  if (last_slot%2==0) {

    ret = mac_xface->ue_scheduler(phy_vars_ue->Mod_id, 
				  phy_vars_ue->frame,
				  last_slot>>1, 
				  subframe_select(&phy_vars_ue->lte_frame_parms,next_slot>>1),
				  eNB_id);
    if (ret == CONNECTION_LOST) {
      LOG_E(PHY,"[UE %d] Frame %d, subframe %d RRC Connection lost, returning to PRACH\n",phy_vars_ue->Mod_id,
	    phy_vars_ue->frame,next_slot>>1);
      phy_vars_ue->UE_mode[eNB_id] = PRACH;
      mac_xface->macphy_exit("Connection lost");
    }
    else if (ret == PHY_RESYNCH) {
      LOG_E(PHY,"[UE %d] Frame %d, subframe %d RRC Connection lost, trying to resynch\n",
	    phy_vars_ue->Mod_id,
	    phy_vars_ue->frame,next_slot>>1);
      phy_vars_ue->UE_mode[eNB_id] = RESYNCH;
      mac_xface->macphy_exit("Connection lost");
      //exit(-1);
    } else if (ret == PHY_HO_PRACH) {
      LOG_I(PHY,"[UE %d] Frame %d, subframe %d, return to PRACH and perform a contention-free access\n",
	    phy_vars_ue->Mod_id,phy_vars_ue->frame,next_slot>>1);
      phy_vars_ue->UE_mode[eNB_id] = PRACH;
    }
  }
#endif

  if (last_slot == 19)
    phy_vars_ue->frame++;

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_LTE,0);
  stop_meas(&phy_vars_ue->phy_proc);
}
