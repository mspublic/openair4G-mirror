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

/*! \file phy_procedures_lte_eNB.c
* \brief Implementation of eNB procedures from 36.213 LTE specifications
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
* \note
* \warning
*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#define DEBUG_PHY_PROC
//#define DEBUG_DLSCH

#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"

//#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#include "UTIL/LOG/log.h"
#include "UTIL/LOG/vcd_signal_dumper.h"
//#endif

//#define DIAG_PHY

#define NS_PER_SLOT 500000

//#define PUCCH 1

#define PUCCH1_THRES 20

extern inline unsigned int taus(void);
extern int exit_openair;

unsigned char dlsch_input_buffer[2700] __attribute__ ((aligned(16)));
int eNB_sync_buffer0[640*6] __attribute__ ((aligned(16)));
int eNB_sync_buffer1[640*6] __attribute__ ((aligned(16)));
int *eNB_sync_buffer[2] = {eNB_sync_buffer0, eNB_sync_buffer1};


unsigned int max_peak_val; 
int max_sect_id, max_sync_pos;

//DCI_ALLOC_t dci_alloc[8];

#ifdef EMOS
fifo_dump_emos_eNB emos_dump_eNB;
#endif

#ifdef DIAG_PHY
extern int rx_sig_fifo;
#endif
static unsigned char I0_clear = 1;

u8 is_SR_subframe(PHY_VARS_eNB *phy_vars_eNB,u8 UE_id,u8 subframe) {
  /*
  msg("[PHY][eNB %d][SR %x] Frame %d subframe %d Checking for SR TXOp(sr_ConfigIndex %d)\n",
      phy_vars_eNB->Mod_id,phy_vars_eNB->ulsch_eNB[UE_id]->rnti,phy_vars_eNB->frame,subframe,
      phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex);
  */
  if (phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex < 5) {        // 5 ms SR period
    if ((subframe%5) == phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex)
      return(1);
  }
  else if (phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex < 14) {  // 10 ms SR period
    if (subframe==(phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex-5))
      return(1);
  }
  else if (phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex < 34) { // 20 ms SR period
    if ((10*(phy_vars_eNB->frame&1)+subframe) == (phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex-15))
      return(1);
  }
  else if (phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex < 74) { // 40 ms SR period
    if ((10*(phy_vars_eNB->frame&3)+subframe) == (phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex-35))
      return(1);
  }
  else if (phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex < 154) { // 80 ms SR period
    if ((10*(phy_vars_eNB->frame&7)+subframe) == (phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex-75))
      return(1);
  }

  return(0);
}

s32 add_ue(s16 rnti, PHY_VARS_eNB *phy_vars_eNB) {
  u8 i;

#ifdef DEBUG_PHY_PROC
  LOG_I(PHY,"[eNB %d] Adding UE with rnti %x\n",phy_vars_eNB->Mod_id,rnti);
#endif
  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if ((phy_vars_eNB->dlsch_eNB[i]==NULL) || (phy_vars_eNB->ulsch_eNB[i]==NULL)) {
      LOG_E(PHY,"Can't add UE, not enough memory allocated\n");
      return(-1);
    }
    else {
      if (phy_vars_eNB->eNB_UE_stats[i].crnti==0) {
	//msg("[PHY] UE_id %d\n",i);
	phy_vars_eNB->dlsch_eNB[i][0]->rnti = rnti;
	phy_vars_eNB->ulsch_eNB[i]->rnti = rnti;
	phy_vars_eNB->eNB_UE_stats[i].crnti = rnti;
	return(i);
      }
    }
  }
  return(-1);
}

s32 remove_ue(u16 rnti, PHY_VARS_eNB *phy_vars_eNB, u8 abstraction_flag) {
  u8 i;

#ifdef DEBUG_PHY_PROC
  LOG_I(PHY,"eNB %d removing UE with rnti %x\n",phy_vars_eNB->Mod_id,rnti);
#endif
  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if ((phy_vars_eNB->dlsch_eNB[i]==NULL) || (phy_vars_eNB->ulsch_eNB[i]==NULL)) {
      LOG_E(PHY,"Can't remove UE, not enough memory allocated\n");
      return(-1);
    }
    else {
      if (phy_vars_eNB->eNB_UE_stats[i].crnti==rnti) {
	//msg("[PHY] UE_id %d\n",i);
	clean_eNb_dlsch(phy_vars_eNB->dlsch_eNB[i][0], abstraction_flag);
	clean_eNb_ulsch(phy_vars_eNB->ulsch_eNB[i],abstraction_flag);
	//phy_vars_eNB->eNB_UE_stats[i].crnti = 0;
	memset(&phy_vars_eNB->eNB_UE_stats[i],0,sizeof(LTE_eNB_UE_stats));
	return(i);
      }
    }
  }

  return(-1);
}

s8 find_next_ue_index(PHY_VARS_eNB *phy_vars_eNB) {
  u8 i;

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if ((phy_vars_eNB->dlsch_eNB[i]) && 
	(phy_vars_eNB->dlsch_eNB[i][0]) && 
	(phy_vars_eNB->dlsch_eNB[i][0]->rnti==0)) {
      return(i);
    }
  }
  return(-1);
}

int get_ue_active_harq_pid(u8 Mod_id,u16 rnti,u8 subframe,u8 *harq_pid,u8 *round,u8 ul_flag) {

  LTE_eNB_DLSCH_t *DLSCH_ptr;  
  LTE_eNB_ULSCH_t *ULSCH_ptr;  
  //  u8 subframe_m4;
  u8 ulsch_subframe,ulsch_frame; 
  u8 i;
  s8 UE_id = find_ue(rnti,PHY_vars_eNB_g[Mod_id]);
  u32 frame = PHY_vars_eNB_g[Mod_id]->frame;

  if (UE_id==-1) {
    msg("[PHY] Cannot find UE with rnti %x\n",rnti);
    *round=0;
    return(-1);
  }

  if (ul_flag == 0)  {// this is a DL request
    DLSCH_ptr = PHY_vars_eNB_g[Mod_id]->dlsch_eNB[(u32)UE_id][0];
    /*    
    if (subframe<4)
      subframe_m4 = subframe+6;
    else
      subframe_m4 = subframe-4;
    */
#ifdef DEBUG_PHY_PROC
    LOG_D(PHY,"[eNB %d] get_ue_active_harq_pid: Frame %d subframe %d, current harq_id %d\n",
	  Mod_id,frame,subframe,DLSCH_ptr->harq_ids[subframe]);
#endif
    // switch on TDD or FDD configuration here later
    // navid: the harq_pid seems inccorect with distributed emulation
    *harq_pid = DLSCH_ptr->harq_ids[subframe];
    if ((*harq_pid<DLSCH_ptr->Mdlharq) && 
	((DLSCH_ptr->harq_processes[*harq_pid]->round > 0))) {

      *round = DLSCH_ptr->harq_processes[*harq_pid]->round;
      LOG_D(PHY,"round %d\n",*round);
    
    //    else if ((subframe_m4==5) || (subframe_m4==6)) {
    //      *harq_pid = 0;//DLSCH_ptr->harq_ids[subframe_m4];//Ankit
    //     *round    = DLSCH_ptr->harq_processes[*harq_pid]->round;
    //    }
    }
    else {
      // get first free harq_pid (i.e. round 0)
      for (i=0;i<DLSCH_ptr->Mdlharq;i++) {
	if (DLSCH_ptr->harq_processes[i]!=NULL) {
	  if (DLSCH_ptr->harq_processes[i]->status != ACTIVE) {
	    *harq_pid = i;//0;//i; //(Ankit)
	    *round = 0;
	    return(0);
	  }
	  else {
	    LOG_D(PHY,"process %d is active\n",i);
	  }
	}
	else {
	  LOG_E(PHY,"[eNB %d] DLSCH process %d for rnti %x (UE_id %d) not allocated\n",Mod_id,i,rnti,UE_id);
	  return(-1);
	}
      }
    }
  }
  else {  // This is a UL request

    ULSCH_ptr = PHY_vars_eNB_g[Mod_id]->ulsch_eNB[(u32)UE_id];
    ulsch_subframe = pdcch_alloc2ul_subframe(&PHY_vars_eNB_g[Mod_id]->lte_frame_parms,subframe);
    ulsch_frame    = pdcch_alloc2ul_frame(&PHY_vars_eNB_g[Mod_id]->lte_frame_parms,((subframe==0)?1:0)+frame,subframe);
    // Note this is for TDD configuration 3,4,5 only
    *harq_pid = subframe2harq_pid(&PHY_vars_eNB_g[Mod_id]->lte_frame_parms,
				  ulsch_frame,
				  ulsch_subframe);
    *round    = ULSCH_ptr->harq_processes[*harq_pid]->round;
    LOG_D(PHY,"[eNB %d][PUSCH %d] Frame %d subframe %d Checking HARQ, round %d\n",Mod_id,*harq_pid,frame+((subframe==0)?1:0),subframe,*round);
  }
  return(0);
}


int CCE_table[800];

void init_nCCE_table() {
  memset(CCE_table,0,800*sizeof(int));
}


int get_nCCE_offset(unsigned char L, int nCCE, int common_dci, unsigned short rnti, unsigned char subframe) {

  int search_space_free,m,nb_candidates,l,i;
  unsigned int Yk;

  
  if (common_dci == 1) {
    // check CCE(0 ... L-1)
    nb_candidates = (L==4) ? 4 : 2;
    for (m = 0 ; m < nb_candidates ; m++) {
      search_space_free = 1;
      for (l=0;l<L;l++) {
	if (CCE_table[(m*L) + l] == 1) {
	  search_space_free = 0;
	  break;
	}
      }
      if (search_space_free == 1) {
	for (l=0;l<L;l++)
	  CCE_table[(m*L)+l]=1;
	return(m*L);
      }
    }
    return(-1);

  }
  else {  // Find first available in ue specific search space
          // according to procedure in Section 9.1.1 of 36.213 (v. 8.6)
    // compute Yk
    Yk = (unsigned int)rnti;

    for (i=0;i<subframe;i++)
      Yk = (Yk*39827)%65537;

    Yk = Yk % (nCCE/L);

    switch (L) {
    case 1:
    case 2:
      nb_candidates = 6;
      break;
    case 4:
    case 8:
      nb_candidates = 2;
      break;
    }

    for (m = 0 ; m < nb_candidates ; m++) {
      search_space_free = 1;
      for (l=0;l<L;l++) {
	if (CCE_table[((Yk+m)*L) + l] == 1) {
	  search_space_free = 0;
	  break;
	}
      }
      if (search_space_free == 1) {
	for (l=0;l<L;l++)
	  CCE_table[((Yk+m)*L)+l]=1;
	return((Yk+m)*L);
      }
    }
    return(-1);
  }
}
#ifdef EMOS
void phy_procedures_emos_eNB_TX(unsigned char next_slot) {

  unsigned char sect_id,i;

  if (next_slot==1) {
    emos_dump_eNB.timestamp = rt_get_time_ns();
    emos_dump_eNB.frame_tx = phy_vars_eNB->frame;
  }
  if (next_slot%2==0) {
    for (i=0; i<2; i++) 
      memcpy(&emos_dump_eNB.DCI_alloc[i][next_slot>>1], &CH_mac_inst[0].DCI_pdu.dci_alloc[i], sizeof(DCI_ALLOC_t));
  }
  if (next_slot==19) {
    LOG_D(PHY,"[eNB %d] Frame %d, slot %d, Writing EMOS data to FIFO\n",
	      phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot);
    if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_eNB, sizeof(fifo_dump_emos_eNB))!=sizeof(fifo_dump_emos_eNB)) {
      LOG_D(PHY,"[eNB %d] Frame %d, slot %d, Problem writing EMOS data to FIFO\n",
		phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot);
      return;
    }
  }
}
#endif

/*
void phy_procedures_eNB_S_TX(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {

  int sect_id = 0, aa;

  if (next_slot%2==0) {
#ifdef DEBUG_PHY_PROC
    msg("[PHY][eNB %d] Frame %d, slot %d: Generating pilots for DL-S\n",
	phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot);
#endif
    
    for (sect_id=0;sect_id<number_of_cards;sect_id++) {
      if (abstraction_flag == 0) {

	for (aa=0; aa<phy_vars_eNB->lte_frame_parms.nb_antennas_tx; aa++) {
	  
	  
#ifdef IFFT_FPGA
	  memset(&phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id][aa][next_slot*(phy_vars_eNB->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNB->lte_frame_parms.symbols_per_tti>>1)],
		 0,(phy_vars_eNB->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNB->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
#else
	  memset(&phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id][aa][next_slot*phy_vars_eNB->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNB->lte_frame_parms.symbols_per_tti>>1)],
		 0,phy_vars_eNB->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNB->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
#endif
	}

	generate_pilots_slot(phy_vars_eNB,
			     phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
			     AMP,
			     next_slot);

	msg("[PHY][eNB] Frame %d, subframe %d Generating PSS\n",
	    phy_vars_eNB->frame,next_slot>>1);
	
	generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		     4*AMP,
		     &phy_vars_eNB->lte_frame_parms,
		     2,
		     next_slot);
      
      }
      else {
#ifdef PHY_ABSTRACTION
	generate_pss_emul(phy_vars_eNB,sect_id);
#endif
      }
    }
  }
}
*/ 

void phy_procedures_eNB_S_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {

  unsigned char sect_id=0; 


  if (last_slot%2 == 0) {
    for (sect_id=0;sect_id<number_of_cards;sect_id++) {
    
      if (abstraction_flag == 0) {
	lte_eNB_I0_measurements(phy_vars_eNB,
				sect_id,
				phy_vars_eNB->first_run_I0_measurements);
      }
#ifdef PHY_ABSTRACTION
      else {
//#ifdef PHY_ABSTRACTION
	lte_eNB_I0_measurements_emul(phy_vars_eNB,
				     sect_id);
//#endif
      }
#endif
    }
    
    if (I0_clear == 1)
      I0_clear = 0;
  }
  
}

#ifdef EMOS
void phy_procedures_emos_eNB_RX(unsigned char last_slot) {
  
    unsigned char sect_id,i,aa;

  if (last_slot%2==1) {
    memcpy(&emos_dump_eNB.phy_vars_eNB->eNB_UE_stats[(last_slot>>1)-2],&PHY_vars_eNB_g->eNB_UE_stats,sizeof(PHY_vars_eNB_g->eNB_UE_stats));
  }

  if (last_slot==4) {
    emos_dump_eNB.rx_total_gain_dB = PHY_vars_eNB_g->rx_total_gain_eNB_dB;
    emos_dump_eNB.mimo_mode = openair_daq_vars.dlsch_transmission_mode;
  }

  if (last_slot==8) {
    emos_dump_eNB.ulsch_errors = PHY_vars_eNB_g->eNB_UE_stats[1].ulsch_errors;
    for (sect_id = 0; sect_id<3; sect_id++)  
      memcpy(&emos_dump_eNB.PHY_measurements_eNB[sect_id],
	     &PHY_vars_eNB_g->PHY_measurements_eNB[sect_id],
	     sizeof(PHY_MEASUREMENTS_eNB));

  }

  if (last_slot%2==1) {
    for (sect_id = 0; sect_id<3; sect_id++)  
      for (aa=0; aa<PHY_vars_eNB_g->lte_frame_parms.nb_antennas_rx; aa++) 
	memcpy(&emos_dump_eNB.channel[(last_slot>>1)-2][sect_id][aa][0],	       PHY_vars_eNB_g->lte_eNB_common_vars.srs_ch_estimates[sect_id][aa],
	       PHY_vars_eNB_g->lte_frame_parms.ofdm_symbol_size*sizeof(int));
  }

}

#endif

#ifndef OPENAIR2
void fill_dci(DCI_PDU *DCI_pdu, u8 subframe, u8 cooperation_flag) {

  int i;
  //u32 rballoc = (((1<<(openair_daq_vars.target_ue_ul_mcs/2))-1)<<(openair_daq_vars.target_ue_dl_mcs/2)) & 0x1FFF;
  u32 rballoc = 0x00F0;
  u32 rballoc2 = 0x000F;
  /*
  u32 rand = taus();
  if ((subframe==8) || (subframe==9) || (subframe==0))
    rand = (rand%5)+5;
  else
    rand = (rand%4)+5;
  */

  DCI_pdu->Num_common_dci = 0;
  DCI_pdu->Num_ue_spec_dci=0;

  switch (subframe) {
    /*
  case 5:
    DCI_pdu->Num_common_dci = 1;
    DCI_pdu->dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
    DCI_pdu->dci_alloc[0].L          = 2;
    DCI_pdu->dci_alloc[0].rnti       = SI_RNTI;
    DCI_pdu->dci_alloc[0].format     = format1A;

    BCCH_alloc_pdu.type              = 1;
    BCCH_alloc_pdu.vrb_type          = 0;
    BCCH_alloc_pdu.rballoc           = computeRIV(25,10,3);
    BCCH_alloc_pdu.ndi               = 1;
    BCCH_alloc_pdu.rv                = 1;
    BCCH_alloc_pdu.mcs               = 1;
    BCCH_alloc_pdu.harq_pid          = 0;
    BCCH_alloc_pdu.TPC               = 1;      // set to 3 PRB
    memcpy((void*)&DCI_pdu->dci_alloc[0].dci_pdu[0],&BCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
    break;
    */
  case 6:
    /*
    DCI_pdu->Num_ue_spec_dci = 1;
    DCI_pdu->dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    DCI_pdu->dci_alloc[0].L          = 2;
    DCI_pdu->dci_alloc[0].rnti       = 0x1236;
    DCI_pdu->dci_alloc[0].format     = format2_2A_M10PRB;

    DLSCH_alloc_pdu1.rballoc          = 0x00ff;
    DLSCH_alloc_pdu1.TPC              = 0;
    DLSCH_alloc_pdu1.dai              = 0;
    DLSCH_alloc_pdu1.harq_pid         = 0;
    DLSCH_alloc_pdu1.tb_swap          = 0;
    DLSCH_alloc_pdu1.mcs1             = 0;
    DLSCH_alloc_pdu1.ndi1             = 1;
    DLSCH_alloc_pdu1.rv1              = 0;
    DLSCH_alloc_pdu1.tpmi             = 0;
    memcpy((void*)&DCI_pdu->dci_alloc[0].dci_pdu[0],(void *)&DLSCH_alloc_pdu1,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
    */
    break;
  case 5:
    DCI_pdu->Num_ue_spec_dci = 2;

    DCI_pdu->dci_alloc[0].dci_length = sizeof_DCI1_5MHz_TDD_t; 
    DCI_pdu->dci_alloc[0].L          = 2;
    DCI_pdu->dci_alloc[0].rnti       = 0x1235;
    DCI_pdu->dci_alloc[0].format     = format1;
    
    DLSCH_alloc_pdu.rballoc          = rballoc;
    DLSCH_alloc_pdu.TPC              = 0;
    DLSCH_alloc_pdu.dai              = 0;
    DLSCH_alloc_pdu.harq_pid         = 1;
    DLSCH_alloc_pdu.mcs              = openair_daq_vars.target_ue_dl_mcs;
    DLSCH_alloc_pdu.ndi              = 1;
    DLSCH_alloc_pdu.rv               = 0;
    memcpy((void*)&DCI_pdu->dci_alloc[0].dci_pdu[0],(void *)&DLSCH_alloc_pdu,sizeof(DCI1_5MHz_TDD_t));

    DCI_pdu->dci_alloc[1].dci_length = sizeof_DCI1_5MHz_TDD_t; 
    DCI_pdu->dci_alloc[1].L          = 2;
    DCI_pdu->dci_alloc[1].rnti       = 0x1236;
    DCI_pdu->dci_alloc[1].format     = format1;

    DLSCH_alloc_pdu.rballoc          = rballoc2;
    DLSCH_alloc_pdu.TPC              = 0;
    DLSCH_alloc_pdu.dai              = 0;
    DLSCH_alloc_pdu.harq_pid         = 1;
    DLSCH_alloc_pdu.mcs              = openair_daq_vars.target_ue_dl_mcs;
    DLSCH_alloc_pdu.ndi              = 1;
    DLSCH_alloc_pdu.rv               = 0;
    memcpy((void*)&DCI_pdu->dci_alloc[1].dci_pdu[0],(void *)&DLSCH_alloc_pdu,sizeof(DCI1_5MHz_TDD_t));
    break;

  case 9:
    /*
    DCI_pdu->Num_common_dci = 1;
    DCI_pdu->dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
    DCI_pdu->dci_alloc[0].L          = 2;
    DCI_pdu->dci_alloc[0].rnti       = RA_RNTI;
    DCI_pdu->dci_alloc[0].format     = format1A;

    RA_alloc_pdu.type                = 1;
    RA_alloc_pdu.vrb_type            = 0;
    RA_alloc_pdu.rballoc             = computeRIV(25,10,4);
    RA_alloc_pdu.ndi      = 1;
    RA_alloc_pdu.rv       = 1;
    RA_alloc_pdu.mcs      = 1;
    RA_alloc_pdu.harq_pid = 0;
    RA_alloc_pdu.TPC      = 1;

    memcpy((void*)&DCI_pdu->dci_alloc[0].dci_pdu[0],&RA_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
    */
    break;

  case 8:
    DCI_pdu->Num_ue_spec_dci = 2;

    //user 1
    DCI_pdu->dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_1_6_t ; 
    DCI_pdu->dci_alloc[0].L          = 2;
    DCI_pdu->dci_alloc[0].rnti       = 0x1235;
    DCI_pdu->dci_alloc[0].format     = format0;

    UL_alloc_pdu.type    = 0;
    UL_alloc_pdu.hopping = 0;
    UL_alloc_pdu.rballoc = computeRIV(25,4,openair_daq_vars.ue_ul_nb_rb);
    UL_alloc_pdu.mcs     = openair_daq_vars.target_ue_ul_mcs;
    UL_alloc_pdu.ndi     = 1;
    UL_alloc_pdu.TPC     = 0;
    UL_alloc_pdu.cshift  = 0;
    UL_alloc_pdu.dai     = 0;
    UL_alloc_pdu.cqi_req = 1;
    memcpy((void*)&DCI_pdu->dci_alloc[0].dci_pdu[0],(void *)&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD_1_6_t));

    //user 2
    DCI_pdu->dci_alloc[1].dci_length = sizeof_DCI0_5MHz_TDD_1_6_t ; 
    DCI_pdu->dci_alloc[1].L          = 2;
    DCI_pdu->dci_alloc[1].rnti       = 0x1236;
    DCI_pdu->dci_alloc[1].format     = format0;

    UL_alloc_pdu.type    = 0;
    UL_alloc_pdu.hopping = 0;
    if (cooperation_flag==0)
      UL_alloc_pdu.rballoc = computeRIV(25,4+openair_daq_vars.ue_ul_nb_rb,openair_daq_vars.ue_ul_nb_rb);
    else 
      UL_alloc_pdu.rballoc = computeRIV(25,4,openair_daq_vars.ue_ul_nb_rb);
    UL_alloc_pdu.mcs     = openair_daq_vars.target_ue_ul_mcs;
    UL_alloc_pdu.ndi     = 1;
    UL_alloc_pdu.TPC     = 0;
    if ((cooperation_flag==0) || (cooperation_flag==1))
      UL_alloc_pdu.cshift  = 0;
    else
      UL_alloc_pdu.cshift  = 1;
    UL_alloc_pdu.dai     = 0;
    UL_alloc_pdu.cqi_req = 1;
    memcpy((void*)&DCI_pdu->dci_alloc[1].dci_pdu[0],(void *)&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD_1_6_t));

    break;

  default:
    break;
  }

  DCI_pdu->nCCE = 0;
  for (i=0;i<DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci;i++) {
    DCI_pdu->nCCE += (1<<(DCI_pdu->dci_alloc[i].L));
  }

}
#endif


void phy_procedures_eNB_TX(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {

  u8 *pbch_pdu=&phy_vars_eNB->pbch_pdu[0];
  //  unsigned int nb_dci_ue_spec = 0, nb_dci_common = 0;
  u16 input_buffer_length, re_allocated=0;
  u32 sect_id = 0,i,aa;
  u8 harq_pid;
  DCI_PDU *DCI_pdu;
  u8 *DLSCH_pdu=NULL;
#ifndef OPENAIR2
  DCI_PDU DCI_pdu_tmp;
  u8 DLSCH_pdu_tmp[768*8];
#endif
  s8 UE_id;
  u8 num_pdcch_symbols=0;
  s16 crnti;
  u16 frame_tx;
  s16 amp;
  u8 ul_subframe;
  u32 ul_frame;


  for (sect_id = 0 ; sect_id < number_of_cards; sect_id++) {

    if (abstraction_flag==0) {
      if (next_slot%2 == 0) {
	for (aa=0; aa<phy_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
	 
#ifdef IFFT_FPGA
	  memset(&phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id][aa][next_slot*(phy_vars_eNB->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNB->lte_frame_parms.symbols_per_tti>>1)],
		 0,(phy_vars_eNB->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNB->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#else
	  memset(&phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id][aa][next_slot*phy_vars_eNB->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNB->lte_frame_parms.symbols_per_tti>>1)],
		 0,phy_vars_eNB->lte_frame_parms.ofdm_symbol_size*(phy_vars_eNB->lte_frame_parms.symbols_per_tti)*sizeof(mod_sym_t));
#endif
	}
      }
      generate_pilots_slot(phy_vars_eNB,
			   phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
			   AMP,
			   next_slot);
    
  

      if (next_slot == 0) {
	
	// First half of PSS/SSS (FDD)
	if (phy_vars_eNB->lte_frame_parms.frame_type == 0) {
	  generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       4*AMP,
		       &phy_vars_eNB->lte_frame_parms,
		       (phy_vars_eNB->lte_frame_parms.Ncp==0) ? 6 : 5,
		       next_slot);
	  generate_sss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       AMP,
		       &phy_vars_eNB->lte_frame_parms,
		       (phy_vars_eNB->lte_frame_parms.Ncp==0) ? 5 : 4,
		       next_slot);
	}
      }
    }      
    if (next_slot == 1) {
      
      if ((phy_vars_eNB->frame&3) == 0) {
	((u8*) pbch_pdu)[0] = 0;
	switch (phy_vars_eNB->lte_frame_parms.N_RB_DL) {
	case 6:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (0<<5);
	  break;
	case 15:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (1<<5);
	  break;
	case 25:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (2<<5);
	  break;
	case 50:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (3<<5);
	  break;
	case 100:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (4<<5);
	  break;
	default:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (2<<5);
	  break;
	}
	((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xef) | 
	  ((phy_vars_eNB->lte_frame_parms.phich_config_common.phich_duration << 4)&0x10);
	
	switch (phy_vars_eNB->lte_frame_parms.phich_config_common.phich_resource) {
	case oneSixth:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (0<<2);
	  break;
	case half:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (1<<2);
	  break;
	case one:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (2<<2);
	  break;
	case two:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (3<<2);
	  break;
	default:
	  break;
	}

	((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xfc) | ((phy_vars_eNB->frame>>8)&0x3);
	((u8*) pbch_pdu)[1] = phy_vars_eNB->frame&0xfc;
	((u8*) pbch_pdu)[2] = 0;
      }
      /// First half of SSS (TDD)
      if (abstraction_flag==0) {
	
	if (phy_vars_eNB->lte_frame_parms.frame_type == 1) {
	  generate_sss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       AMP,
		       &phy_vars_eNB->lte_frame_parms,
		       (phy_vars_eNB->lte_frame_parms.Ncp==0) ? 6 : 5,
		       next_slot);
	}
      }
      

      
      frame_tx = (((int) (pbch_pdu[0]&0x3))<<8) + ((int) (pbch_pdu[1]&0xfc)) + phy_vars_eNB->frame%4;
   
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d] Frame %d, slot %d: Calling generate_pbch, mode1_flag=%d, frame_tx=%d, pdu=%02x%02x%02x\n",
	    phy_vars_eNB->Mod_id,
	    phy_vars_eNB->frame, 
	    next_slot,
	    phy_vars_eNB->lte_frame_parms.mode1_flag,
	    frame_tx,
	    ((u8*) pbch_pdu)[0],
	    ((u8*) pbch_pdu)[1],
	    ((u8*) pbch_pdu)[2]);
#endif
      
      if (abstraction_flag==0) {

	generate_pbch(&phy_vars_eNB->lte_eNB_pbch,
		      phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		      AMP,
		      &phy_vars_eNB->lte_frame_parms,
		      pbch_pdu,
		      phy_vars_eNB->frame&3);

      }
#ifdef PHY_ABSTRACTION
      else {
//#ifdef PHY_ABSTRACTION
	generate_pbch_emul(phy_vars_eNB,pbch_pdu); 
//#endif
      }
#endif
    }


    if (next_slot == 2) {
	
      if (abstraction_flag==0) {
	
          if (phy_vars_eNB->lte_frame_parms.frame_type == 1) {
              //	  printf("Generating PSS (frame %d, subframe %d)\n",phy_vars_eNB->frame,next_slot>>1);
              generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
                           4*AMP,
                           &phy_vars_eNB->lte_frame_parms,
                           2,
                           next_slot);
          }
      }
    } 

    // Second half of PSS/SSS (FDD)
    if (next_slot == 10) {
     
      if (abstraction_flag==0) {
       
	if (phy_vars_eNB->lte_frame_parms.frame_type == 0) {
	  generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       4*AMP,
		       &phy_vars_eNB->lte_frame_parms,
		       (phy_vars_eNB->lte_frame_parms.Ncp==0) ? 6 : 5,
		       next_slot);
	  generate_sss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       AMP,
		       &phy_vars_eNB->lte_frame_parms,
		       (phy_vars_eNB->lte_frame_parms.Ncp==0) ? 5 : 4,
		       next_slot);

	}
      }
    }
    //  Second-half of SSS (TDD)
    if (next_slot == 11) {
      if (abstraction_flag==0) {
       
	if (phy_vars_eNB->lte_frame_parms.frame_type == 1) {
	  generate_sss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       AMP,
		       &phy_vars_eNB->lte_frame_parms,
		       (phy_vars_eNB->lte_frame_parms.Ncp==0) ? 6 : 5,
		       next_slot);
	}
      }
    }
    // Second half of PSS (TDD)
    if (next_slot == 12) {
     
      if (abstraction_flag==0) {
       
	if (phy_vars_eNB->lte_frame_parms.frame_type == 1) {
	  //	    printf("Generating PSS (frame %d, subframe %d)\n",phy_vars_eNB->frame,next_slot>>1);
	  generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       4*AMP,
		       &phy_vars_eNB->lte_frame_parms,
		       2,
		       next_slot);
	}
      }
    }
  }

  //return;


  sect_id=0;

  if ((next_slot % 2)==0) {
    //#ifdef DEBUG_PHY_PROC
    //    msg("[PHY][eNB %d] UE %d: Mode %s\n",phy_vars_eNB->Mod_id,0,mode_string[phy_vars_eNB->eNB_UE_stats[0].mode]);
    //#endif

#ifdef OPENAIR2
      // if there are two users and we want to do cooperation
    //    if ((phy_vars_eNB->eNB_UE_stats[0].mode == PUSCH) && (phy_vars_eNB->eNB_UE_stats[1].mode == PUSCH))
    //      mac_xface->eNB_dlsch_ulsch_scheduler(phy_vars_eNB->Mod_id,phy_vars_eNB->cooperation_flag,phy_vars_eNB->frame,next_slot>>1);//,1);
    //    else
    mac_xface->eNB_dlsch_ulsch_scheduler(phy_vars_eNB->Mod_id,0,phy_vars_eNB->frame,next_slot>>1);//,1);

    // Parse DCI received from MAC
    DCI_pdu = mac_xface->get_dci_sdu(phy_vars_eNB->Mod_id,
				     phy_vars_eNB->frame,
				     next_slot>>1);
#else
    DCI_pdu = &DCI_pdu_tmp;
    fill_dci(DCI_pdu,next_slot>>1,phy_vars_eNB->cooperation_flag);
#endif

    // clear existing ulsch dci allocations before applying info from MAC  (this is table
    ul_subframe = pdcch_alloc2ul_subframe(&phy_vars_eNB->lte_frame_parms,next_slot>>1);
    ul_frame = pdcch_alloc2ul_frame(&phy_vars_eNB->lte_frame_parms,(((next_slot>>1)==0)?1:0)+phy_vars_eNB->frame,next_slot>>1);

    if ((subframe_select(&phy_vars_eNB->lte_frame_parms,ul_subframe)==SF_UL) ||
	(phy_vars_eNB->lte_frame_parms.frame_type == 0)) {
      harq_pid = subframe2harq_pid(&phy_vars_eNB->lte_frame_parms,ul_frame,ul_subframe);
      for (i=0;i<NUMBER_OF_UE_MAX;i++)
	if (phy_vars_eNB->ulsch_eNB[i]) {
	  phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->dci_alloc=0;
	  phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->rar_alloc=0;
	}
    }
  
#ifdef EMOS
    emos_dump_eNB.dci_cnt[next_slot>>1] = DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci; //nb_dci_common+nb_dci_ue_spec;
#endif
    // clear previous allocation information for all UEs
    for (i=0;i<NUMBER_OF_UE_MAX;i++) {
      phy_vars_eNB->dlsch_eNB[i][0]->subframe_tx[next_slot>>1] = 0;
    }

    init_nCCE_table();

    for (i=0;i<DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci ; i++) {
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB] Subframe %d : Doing DCI index %d/%d\n",next_slot>>1,i,DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci);
      //if (phy_vars_eNB->frame == 1)
      //dump_dci(&phy_vars_eNB->lte_frame_parms,&DCI_pdu->dci_alloc[i]);
#endif

      if (DCI_pdu->dci_alloc[i].rnti == SI_RNTI) {
	//	LOG_D(PHY,"[eNB %d] SI generate_eNB_dlsch_params_from_dci\n", phy_vars_eNB->Mod_id);
	generate_eNB_dlsch_params_from_dci(next_slot>>1,
					   &DCI_pdu->dci_alloc[i].dci_pdu[0],
					   DCI_pdu->dci_alloc[i].rnti,
					   DCI_pdu->dci_alloc[i].format,
					   &phy_vars_eNB->dlsch_eNB_SI,
					   &phy_vars_eNB->lte_frame_parms,
					   phy_vars_eNB->pdsch_config_dedicated,
					   SI_RNTI,
					   0,
					   P_RNTI,
					   phy_vars_eNB->eNB_UE_stats[0].DL_pmi_single);

	if ((phy_vars_eNB->dlsch_eNB_SI->nCCE[next_slot>>1] = get_nCCE_offset(1<<DCI_pdu->dci_alloc[i].L,
									      DCI_pdu->dci_alloc[i].nCCE,
									      1,
									      SI_RNTI,
									      next_slot>>1)) == -1) {
	  LOG_E(PHY,"[eNB %d] Frame %d subframe %d : No available CCE resources for common DCI (SI)!!!\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot>>1);
      	} 
	else {
	  LOG_D(PHY,"[eNB %d] Frame %d subframe %d : CCE resource for common DCI (SI)  => %d/%d\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot>>1,
		phy_vars_eNB->dlsch_eNB_SI->nCCE[next_slot>>1],DCI_pdu->nCCE);
	}
	DCI_pdu->dci_alloc[i].nCCE = phy_vars_eNB->dlsch_eNB_SI->nCCE[next_slot>>1];

      }
      else if (DCI_pdu->dci_alloc[i].ra_flag == 1) {
#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[eNB %d] RA generate_eNB_dlsch_params_from_dci\n", phy_vars_eNB->Mod_id);
#endif
	generate_eNB_dlsch_params_from_dci(next_slot>>1,
					   &DCI_pdu->dci_alloc[i].dci_pdu[0],
					   DCI_pdu->dci_alloc[i].rnti,
					   DCI_pdu->dci_alloc[i].format,
					   &phy_vars_eNB->dlsch_eNB_ra,
					   &phy_vars_eNB->lte_frame_parms,
					   phy_vars_eNB->pdsch_config_dedicated,
					   SI_RNTI,
					   DCI_pdu->dci_alloc[i].rnti,
					   P_RNTI,
					   phy_vars_eNB->eNB_UE_stats[0].DL_pmi_single);


	if ((phy_vars_eNB->dlsch_eNB_ra->nCCE[next_slot>>1] = get_nCCE_offset(1<<DCI_pdu->dci_alloc[i].L,
									      DCI_pdu->nCCE,
									      1,
									      DCI_pdu->dci_alloc[i].rnti,
									      next_slot>>1)) == -1) {
	  LOG_E(PHY,"[eNB %d] Frame %d subframe %d : No available CCE resources for common DCI (RA) !!!\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot>>1);
	}
	else {
	  LOG_D(PHY,"[eNB %d] Frame %d subframe %d : CCE resource for common DCI (RA)  => %d/%d\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot>>1,
		phy_vars_eNB->dlsch_eNB_ra->nCCE[next_slot>>1],DCI_pdu->nCCE);
	}
	DCI_pdu->dci_alloc[i].nCCE = phy_vars_eNB->dlsch_eNB_ra->nCCE[next_slot>>1];
      }

      else if (DCI_pdu->dci_alloc[i].format != format0){ // this is a normal DLSCH allocation

#ifdef OPENAIR2
#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[eNB] Searching for RNTI %x\n",DCI_pdu->dci_alloc[i].rnti);
#endif
	UE_id = find_ue((s16)DCI_pdu->dci_alloc[i].rnti,phy_vars_eNB);
#else
	UE_id = i;
#endif
	if (UE_id>=0) {
	  //	  dump_dci(&phy_vars_eNB->lte_frame_parms,&DCI_pdu->dci_alloc[i]);
	  generate_eNB_dlsch_params_from_dci(next_slot>>1,
					     &DCI_pdu->dci_alloc[i].dci_pdu[0],
					     DCI_pdu->dci_alloc[i].rnti,
					     DCI_pdu->dci_alloc[i].format,
					     phy_vars_eNB->dlsch_eNB[(u8)UE_id],
					     &phy_vars_eNB->lte_frame_parms,
					     phy_vars_eNB->pdsch_config_dedicated,
					     SI_RNTI,
					     0,
					     P_RNTI,
					     phy_vars_eNB->eNB_UE_stats[(u8)UE_id].DL_pmi_single);

	  if ((phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->nCCE[next_slot>>1] = get_nCCE_offset(1<<DCI_pdu->dci_alloc[i].L,
											   DCI_pdu->nCCE,
											   0,
											   DCI_pdu->dci_alloc[i].rnti,
											   next_slot>>1)) == -1) {
	  LOG_E(PHY,"[eNB %d] Frame %d subframe %d : No available CCE resources for UE spec DCI (PDSCH %x) !!!\n",
		phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot>>1,DCI_pdu->dci_alloc[i].rnti);
	}
	else {
	  LOG_D(PHY,"[eNB %d] Frame %d subframe %d : CCE resource for ue DCI (PDSCH %x)  => %d/%d\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot>>1,
		DCI_pdu->dci_alloc[i].rnti,phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->nCCE[next_slot>>1],DCI_pdu->nCCE);
	}
#ifdef DEBUG_PHY_PROC      
	DCI_pdu->dci_alloc[i].nCCE = phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->nCCE[next_slot>>1];
	  LOG_D(PHY,"[eNB %d][PDSCH %x] Frame %d subframe %d UE_id %d Generated DCI format %d, aggregation %d\n",
	      phy_vars_eNB->Mod_id, DCI_pdu->dci_alloc[i].rnti,
	      phy_vars_eNB->frame, next_slot>>1,UE_id,
	      DCI_pdu->dci_alloc[i].format,
	      1<<DCI_pdu->dci_alloc[i].L);
#endif
	}
	else {
	  LOG_D(PHY,"[eNB %d][PDSCH] Frame %d : No UE_id with corresponding rnti %x, dropping DLSCH\n",
	      phy_vars_eNB->Mod_id,phy_vars_eNB->frame,(s16)DCI_pdu->dci_alloc[i].rnti);
	}
      }

    }

    // Apply physicalConfigDedicated if needed
    phy_config_dedicated_eNB_step2(phy_vars_eNB);

    for (i=0;i<DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci ; i++) {    
      if (DCI_pdu->dci_alloc[i].format == format0) {  // this is a ULSCH allocation

	harq_pid = subframe2harq_pid(&phy_vars_eNB->lte_frame_parms,
				     pdcch_alloc2ul_frame(&phy_vars_eNB->lte_frame_parms,(((next_slot>>1)==0)?1:0)+phy_vars_eNB->frame,next_slot>>1),
				     pdcch_alloc2ul_subframe(&phy_vars_eNB->lte_frame_parms,next_slot>>1));
	if (harq_pid==255) {
	  LOG_E(PHY,"[eNB %d] Frame %d: Bad harq_pid for ULSCH allocation\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame);
#ifdef USER_MODE
	  exit(-1);
#else
	  exit_openair=1;
#endif
	}
#ifdef OPENAIR2
	UE_id = find_ue((s16)DCI_pdu->dci_alloc[i].rnti,phy_vars_eNB);
#else
	UE_id = i;
#endif
	if (UE_id<0) {
	  LOG_E(PHY,"[eNB %d] Frame %d: Unknown UE_id for rnti %x\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame,(s16)DCI_pdu->dci_alloc[i].rnti);
	  mac_xface->macphy_exit("");
	}
#ifdef DEBUG_PHY_PROC
	LOG_I(PHY,"[eNB %d][PUSCH %d] Frame %d subframe %d Generated format0 DCI (rnti %x, dci %x) (DCI pos %d/%d), aggregation %d\n",
	    phy_vars_eNB->Mod_id, 
	    subframe2harq_pid(&phy_vars_eNB->lte_frame_parms,
			      pdcch_alloc2ul_frame(&phy_vars_eNB->lte_frame_parms,(((next_slot>>1)==0)?1:0)+phy_vars_eNB->frame,next_slot>>1),
			      pdcch_alloc2ul_subframe(&phy_vars_eNB->lte_frame_parms,next_slot>>1)),
	    pdcch_alloc2ul_frame(&phy_vars_eNB->lte_frame_parms,(((next_slot>>1)==0)?1:0)+phy_vars_eNB->frame,next_slot>>1),
	    next_slot>>1,DCI_pdu->dci_alloc[i].rnti,
	    *(unsigned int *)&DCI_pdu->dci_alloc[i].dci_pdu[0],
	    i,DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci,
	    1<<DCI_pdu->dci_alloc[i].L);
	//dump_dci(&phy_vars_eNB->lte_frame_parms,&DCI_pdu->dci_alloc[i]);
#endif
	
	generate_eNB_ulsch_params_from_dci(&DCI_pdu->dci_alloc[i].dci_pdu[0],
					   DCI_pdu->dci_alloc[i].rnti,
					   (next_slot>>1),
					   format0,
					   UE_id,
					   phy_vars_eNB,
					   SI_RNTI,
					   0,
					   P_RNTI,
					   0);  // do_srs

	if ((DCI_pdu->dci_alloc[i].nCCE=get_nCCE_offset(1<<DCI_pdu->dci_alloc[i].L,
							DCI_pdu->nCCE,
							0,
							DCI_pdu->dci_alloc[i].rnti,
							next_slot>>1)) == -1) {
	  LOG_E(PHY,"[eNB %d] Frame %d subframe %d : No available CCE resources (%d) for UE spec DCI (PUSCH %x) !!!\n",
		phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot>>1,DCI_pdu->nCCE,DCI_pdu->dci_alloc[i].rnti);
	}
	else {
	  LOG_D(PHY,"[eNB %d] Frame %d subframe %d : CCE resources for UE spec DCI (PUSCH %x) => %d/%d\n",
		phy_vars_eNB->Mod_id,phy_vars_eNB->frame,next_slot>>1,DCI_pdu->dci_alloc[i].rnti,
		DCI_pdu->dci_alloc[i].nCCE,DCI_pdu->nCCE);
	}
#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[eNB %d][PUSCH %d] frame %d subframe %d Setting subframe_scheduling_flag for UE %d harq_pid %d (ul subframe %d)\n",
	      phy_vars_eNB->Mod_id,harq_pid,
	      ((next_slot>>1)==0 ? 1 : 0) +phy_vars_eNB->frame,next_slot>>1,i,harq_pid,
	      pdcch_alloc2ul_subframe(&phy_vars_eNB->lte_frame_parms,next_slot>>1));
#endif
	phy_vars_eNB->ulsch_eNB[(u32)UE_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
	
      }
    }

    




    // if we have DCI to generate do it now
    if ((DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci)>0) {

 
    }
    else {  // for emulation!!
      phy_vars_eNB->num_ue_spec_dci[(next_slot>>1)&1]=0;
      phy_vars_eNB->num_common_dci[(next_slot>>1)&1]=0;
    }
 
    if (abstraction_flag == 0) {
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d] Frame %d, subframe %d: Calling generate_dci_top (common %d,ue_spec %d)\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot>>1,DCI_pdu->Num_common_dci,DCI_pdu->Num_ue_spec_dci);
#endif
      for (sect_id=0;sect_id<number_of_cards;sect_id++) 
	num_pdcch_symbols = generate_dci_top(DCI_pdu->Num_ue_spec_dci,
					     DCI_pdu->Num_common_dci,
					     DCI_pdu->dci_alloc,
					     0,
					     AMP,
					     &phy_vars_eNB->lte_frame_parms,
					     phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
					     next_slot>>1);
    }
#ifdef PHY_ABSTRACTION
    else {
//#ifdef PHY_ABSTRACTION
      LOG_D(PHY,"[eNB %d] Frame %d, subframe %d: Calling generate_dci_top_emul\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot>>1);
      num_pdcch_symbols = generate_dci_top_emul(phy_vars_eNB,DCI_pdu->Num_ue_spec_dci,DCI_pdu->Num_common_dci,DCI_pdu->dci_alloc,next_slot>>1);
//#endif
    }
#endif



#ifdef DEBUG_PHY_PROC
    //LOG_D(PHY,"[eNB %d] Frame %d, slot %d: num_pdcch_symbols=%d\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot,num_pdcch_symbols);
#endif

    // Check for SI activity

    if (phy_vars_eNB->dlsch_eNB_SI->active == 1) {
      input_buffer_length = phy_vars_eNB->dlsch_eNB_SI->harq_processes[0]->TBS/8;


#ifdef OPENAIR2
      DLSCH_pdu = mac_xface->get_dlsch_sdu(phy_vars_eNB->Mod_id,
					   phy_vars_eNB->frame,
					   SI_RNTI,
					   0);
#else
      DLSCH_pdu = DLSCH_pdu_tmp;
      for (i=0;i<input_buffer_length;i++)
	DLSCH_pdu[i] = (unsigned char)(taus()&0xff);
#endif      


      
#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_DLSCH
      LOG_D(PHY,"[eNB %d][SI] Frame %d, slot %d: Calling generate_dlsch (SI) with input size = %d, num_pdcch_symbols %d\n",
	  phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot, input_buffer_length,num_pdcch_symbols);
      for (i=0;i<input_buffer_length;i++)
	LOG_T(PHY,"%x.",i,DLSCH_pdu[i]);
      LOG_D(PHY,"\n");
#endif
#endif

      if (abstraction_flag == 0) {

	dlsch_encoding(DLSCH_pdu,
		       &phy_vars_eNB->lte_frame_parms,
		       num_pdcch_symbols,
		       phy_vars_eNB->dlsch_eNB_SI,
		       next_slot>>1);
	
	dlsch_scrambling(&phy_vars_eNB->lte_frame_parms,
			 num_pdcch_symbols,
			 phy_vars_eNB->dlsch_eNB_SI,
			 get_G(&phy_vars_eNB->lte_frame_parms,
			       phy_vars_eNB->dlsch_eNB_SI->nb_rb,
			       phy_vars_eNB->dlsch_eNB_SI->rb_alloc,
			       get_Qm(phy_vars_eNB->dlsch_eNB_SI->harq_processes[0]->mcs),
			       num_pdcch_symbols,next_slot>>1),
			 0,
			 next_slot);      
	
	for (sect_id=0;sect_id<number_of_cards;sect_id++) 
	  re_allocated = dlsch_modulation(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
					  AMP,
					  next_slot/2,
					  &phy_vars_eNB->lte_frame_parms,
					  num_pdcch_symbols,
					  phy_vars_eNB->dlsch_eNB_SI);
      } 
#ifdef PHY_ABSTRACTION
      else {
//#ifdef PHY_ABSTRACTION
	dlsch_encoding_emul(phy_vars_eNB,
			    DLSCH_pdu,
			    phy_vars_eNB->dlsch_eNB_SI);
//#endif
      }
#endif
      phy_vars_eNB->dlsch_eNB_SI->active = 0;
      
    }

    // Check for RA activity
    if (phy_vars_eNB->dlsch_eNB_ra->active == 1) {
#ifdef DEBUG_PHY_PROC
      LOG_I(PHY,"[eNB %d][RAPROC] Frame %d, slot %d, RA active, filling RAR:\n",
	  phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot);
#endif

      input_buffer_length = phy_vars_eNB->dlsch_eNB_ra->harq_processes[0]->TBS/8;

#ifdef OPENAIR2
      crnti = mac_xface->fill_rar(phy_vars_eNB->Mod_id,
				  phy_vars_eNB->frame,
				  dlsch_input_buffer,
				  phy_vars_eNB->lte_frame_parms.N_RB_UL,
				  input_buffer_length);
      /*
      for (i=0;i<input_buffer_length;i++)
	msg("%x.",dlsch_input_buffer[i]);
      msg("\n");
      */
      UE_id = add_ue(crnti,phy_vars_eNB);
      if (UE_id==-1) {
	//mac_xface->macphy_exit("[PHY][eNB] Max user count reached.\n");
	LOG_W(PHY,"[eNB] Max user count reached.\n");
      }
      else {
      phy_vars_eNB->eNB_UE_stats[(u32)UE_id].mode = RA_RESPONSE;
      // Initialize indicator for first SR (to be cleared after ConnectionSetup is acknowledged)
      phy_vars_eNB->first_sr[(u32)UE_id] = 1;

      generate_eNB_ulsch_params_from_rar(dlsch_input_buffer,
					 phy_vars_eNB->frame,
					 (next_slot>>1),
					 phy_vars_eNB->ulsch_eNB[(u32)UE_id],
					 &phy_vars_eNB->lte_frame_parms);

      phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_active = 1;

      get_Msg3_alloc(&phy_vars_eNB->lte_frame_parms,
		     next_slot>>1,
		     phy_vars_eNB->frame,
		     &phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_frame,
		     &phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_subframe);
#else
      for (i=0;i<input_buffer_length;i++)
      	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
#endif

#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d][RAPROC] Frame %d, next slot %d: Calling generate_dlsch (RA) with input size = %d,Msg3 frame %d, Msg3 subframe %d\n",
	  phy_vars_eNB->Mod_id,
	  phy_vars_eNB->frame, next_slot,input_buffer_length, 
	  phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_frame,
	  phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_subframe);
#endif

      if (abstraction_flag == 0) {

	dlsch_encoding(dlsch_input_buffer,
		       &phy_vars_eNB->lte_frame_parms,
		       num_pdcch_symbols,
		       phy_vars_eNB->dlsch_eNB_ra,
		       next_slot>>1);

	//	phy_vars_eNB->dlsch_eNB_ra->rnti = RA_RNTI;
	dlsch_scrambling(&phy_vars_eNB->lte_frame_parms,
			 num_pdcch_symbols,
			 phy_vars_eNB->dlsch_eNB_ra,
			 get_G(&phy_vars_eNB->lte_frame_parms,
			       phy_vars_eNB->dlsch_eNB_ra->nb_rb,
			       phy_vars_eNB->dlsch_eNB_ra->rb_alloc,
			       get_Qm(phy_vars_eNB->dlsch_eNB_ra->harq_processes[0]->mcs),
			       num_pdcch_symbols,next_slot>>1),
			 0,
			 next_slot);
	for (sect_id=0;sect_id<number_of_cards;sect_id++) 
	  re_allocated = dlsch_modulation(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
					  AMP,
					  next_slot/2,
					  &phy_vars_eNB->lte_frame_parms,
					  num_pdcch_symbols,
					  phy_vars_eNB->dlsch_eNB_ra);
      }
#ifdef PHY_ABSTRACTION
      else {
	dlsch_encoding_emul(phy_vars_eNB,
			    dlsch_input_buffer,
			    phy_vars_eNB->dlsch_eNB_ra);
      }
#endif
      LOG_D(PHY,"[eNB %d][RAPROC] Frame %d subframe %d Deactivating DLSCH RA\n",phy_vars_eNB->Mod_id,
	  phy_vars_eNB->frame,next_slot>>1);
      phy_vars_eNB->dlsch_eNB_ra->active = 0;
	
#ifdef DEBUG_PHY_PROC    
      LOG_D(PHY,"[eNB %d] Frame %d, slot %d, DLSCH (RA) re_allocated = %d\n",phy_vars_eNB->Mod_id,
	  phy_vars_eNB->frame, next_slot, re_allocated);
#endif

#ifdef OPENAIR2
      } //max user count
#endif
    }

    // Now scan UE specific DLSCH
    for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
      if ((phy_vars_eNB->dlsch_eNB[(u8)UE_id][0])&&
	  (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti>0)&&
	  (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->active == 1)) {
	harq_pid = phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->current_harq_pid;
	input_buffer_length = phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->TBS/8;
      

#ifdef DEBUG_PHY_PROC
	LOG_I(PHY,"[eNB %d][PDSCH %x/%d] Frame %d, subframe %d: Generating PDSCH/DLSCH with input size = %d, G %d, nb_rb %d, mcs %d, Ndi %d, rv %d (round %d)\n",
	      phy_vars_eNB->Mod_id, phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti,harq_pid,
	      phy_vars_eNB->frame, next_slot>>1, input_buffer_length,
	      get_G(&phy_vars_eNB->lte_frame_parms,
		    phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->nb_rb,
		    phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rb_alloc,
		    get_Qm(phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->mcs),
		    num_pdcch_symbols,next_slot>>1),
	      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->nb_rb,
	      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->mcs,
	      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->Ndi,
	      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->rvidx,
	      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->round);
#endif

	phy_vars_eNB->eNB_UE_stats[(u8)UE_id].dlsch_sliding_cnt++;
	if (phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->harq_processes[harq_pid]->Ndi == 1) {

	  phy_vars_eNB->eNB_UE_stats[(u32)UE_id].dlsch_trials[0]++;
	  
#ifdef OPENAIR2
	  DLSCH_pdu = mac_xface->get_dlsch_sdu(phy_vars_eNB->Mod_id,
					       phy_vars_eNB->frame,
					       phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti,
					       0);
#else
	  DLSCH_pdu = DLSCH_pdu_tmp;
	  for (i=0;i<input_buffer_length;i++)
	    DLSCH_pdu[i] = (unsigned char)(taus()&0xff);
#endif      

#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_DLSCH
	  LOG_D(PHY,"eNB DLSCH SDU: \n");
	  for (i=0;i<phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->TBS>>3;i++)
	    LOG_D(PHY,"%x.",(u8)DLSCH_pdu[i]);
	  LOG_D(PHY,"\n");
#endif
#endif
	}
	else {
	  phy_vars_eNB->eNB_UE_stats[(u32)UE_id].dlsch_trials[phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->round]++;	
#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_DLSCH  
	  LOG_D(PHY,"[eNB] This DLSCH is a retransmission\n");
#endif
#endif
	}
	if (abstraction_flag==0) {

	  // 36-212
	  dlsch_encoding(DLSCH_pdu,
			 &phy_vars_eNB->lte_frame_parms,
			 num_pdcch_symbols,
			 phy_vars_eNB->dlsch_eNB[(u8)UE_id][0],
			 next_slot>>1);
	  // 36-211
	  dlsch_scrambling(&phy_vars_eNB->lte_frame_parms,
			   num_pdcch_symbols,
			   phy_vars_eNB->dlsch_eNB[(u8)UE_id][0],
			   get_G(&phy_vars_eNB->lte_frame_parms,
				 phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->nb_rb,
				 phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rb_alloc,
				 get_Qm(phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->mcs),
				 num_pdcch_symbols,next_slot>>1),
			   0,
			   next_slot);      
	  for (sect_id=0;sect_id<number_of_cards;sect_id++) {
	    
          /*          if ((phy_vars_eNB->transmission_mode[(u8)UE_id] == 5) &&
		  (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->dl_power_off == 0)) 
              amp = (s16)(((s32)AMP*(s32)ONE_OVER_SQRT2_Q15)>>15);
              else*/
          //              amp = AMP;

	    re_allocated = dlsch_modulation(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
					    AMP,
					    next_slot/2,
					    &phy_vars_eNB->lte_frame_parms,
					    num_pdcch_symbols,
					    phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]);
	  }
	}
#ifdef PHY_ABSTRACTION
	else {
//#ifdef PHY_ABSTRACTION
	  dlsch_encoding_emul(phy_vars_eNB,
			      DLSCH_pdu,
			      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]);
//#endif
	}
#endif
	phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->active = 0;
	
	//mac_xface->macphy_exit("first dlsch transmitted\n");
      }

      else if ((phy_vars_eNB->dlsch_eNB[(u8)UE_id][0])&&
	       (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti>0)&&
	       (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->active == 0)) {
      
	// clear subframe TX flag since UE is not scheduled for PDSCH in this subframe (so that we don't look for PUCCH later)
	phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->subframe_tx[next_slot>>1]=0;
#ifdef DEBUG_PHY_PROC
	//LOG_D(PHY,"[eNB %d] DCI: Clearing subframe_tx for subframe %d, UE %d\n",phy_vars_eNB->Mod_id,next_slot>>1,UE_id);
#endif
      }
    } 
  


    // if we have PHICH to generate
    //    printf("[PHY][eNB] Frame %d subframe %d Checking for phich\n",phy_vars_eNB->frame,next_slot>>1); 
    if (is_phich_subframe(&phy_vars_eNB->lte_frame_parms,next_slot>>1)) {
#ifdef DEBUG_PHY_PROC
      //      LOG_D(PHY,"[eNB %d] Frame %d, subframe %d: Calling generate_phich_top\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot>>1);
#endif
	for (sect_id=0;sect_id<number_of_cards;sect_id++) {
	  generate_phich_top(phy_vars_eNB,
			     next_slot>>1,
			     AMP,
			     sect_id,
			     abstraction_flag);
	}
    }
  }





#ifdef EMOS
  phy_procedures_emos_eNB_TX(next_slot);
#endif
}
  
void process_Msg3(PHY_VARS_eNB *phy_vars_eNB,u8 last_slot,u8 UE_id, u8 harq_pid) {
  // this prepares the demodulation of the first PUSCH of a new user, containing Msg3

  
  LOG_D(PHY,"[eNB %d][RAPROC] frame %d : subframe %d (last_slot %d): process_Msg3 UE_id %d (active %d, subframe %d, frame %d)\n",
      phy_vars_eNB->Mod_id,
      phy_vars_eNB->frame,last_slot>>1,last_slot,
      UE_id,phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_active,
      phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_subframe,
      phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_frame);
  phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_flag = 0;

  if ((phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_active == 1) && 
      ((last_slot%2)==1) && 
      (phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_subframe == (last_slot>>1)) &&
      (phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_frame == (phy_vars_eNB->frame)))   {

    //    harq_pid = 0;

    phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_active = 0;
    phy_vars_eNB->ulsch_eNB[(u32)UE_id]->Msg3_flag = 1;
    phy_vars_eNB->ulsch_eNB[(u32)UE_id]->harq_processes[harq_pid]->subframe_scheduling_flag=1;
    LOG_D(PHY,"[eNB %d][RAPROC] frame %d, slot %d, subframe %d: Setting subframe_scheduling_flag (Msg3) for UE %d\n",
	phy_vars_eNB->Mod_id,
	phy_vars_eNB->frame,last_slot,last_slot>>1,UE_id);
  }
}


// This function retrieves the harq_pid of the corresponding DLSCH process
// and updates the error statistics of the DLSCH based on the received ACK
// info from UE along with the round index.  It also performs the fine-grain
// rate-adaptation based on the error statistics derived from the ACK/NAK process

void process_HARQ_feedback(u8 UE_id, 
			   u8 subframe, 
			   PHY_VARS_eNB *phy_vars_eNB,
			   u8 pusch_flag, 
			   u8 *pucch_payload, 
			   u8 pucch_sel,
			   u8 SR_payload) {

  u8 dl_harq_pid[8],dlsch_ACK[8],j,dl_subframe;
  LTE_eNB_DLSCH_t *dlsch             =  phy_vars_eNB->dlsch_eNB[(u32)UE_id][0];
  LTE_eNB_UE_stats *ue_stats         =  &phy_vars_eNB->eNB_UE_stats[(u32)UE_id];
  LTE_DL_eNB_HARQ_t *dlsch_harq_proc;
  u8 subframe_m4,M,m;
  int mp;
  int all_ACKed=1,nb_alloc=0,nb_ACK=0;

  if (phy_vars_eNB->lte_frame_parms.frame_type == 0){ //FDD
    subframe_m4 = (subframe<4) ? subframe+6 : subframe-4;

    dl_harq_pid[0] = dlsch->harq_ids[subframe_m4];
    M=1;
    if (pusch_flag == 1)
      dlsch_ACK[0] = phy_vars_eNB->ulsch_eNB[(u8)UE_id]->o_ACK[0];
    else
      dlsch_ACK[0] = pucch_payload[0];
    LOG_D(PHY,"[eNB %d] Frame %d: Received ACK/NAK %d for subframe %d\n",phy_vars_eNB->Mod_id,
	      phy_vars_eNB->frame,dlsch_ACK[0],subframe_m4);
  }
  else {  // TDD Handle M=1,2 cases only
    
    M=ul_ACK_subframe2_M(&phy_vars_eNB->lte_frame_parms,
			 subframe);
    // Now derive ACK information for TDD
    if (pusch_flag == 1) { // Do PUSCH ACK/NAK first
      // detect missing DAI
      //FK: this code is just a guess
      //RK: not exactly, yes if scheduled from PHICH (i.e. no DCI format 0)
      //    otherwise, it depends on how many of the PDSCH in the set are scheduled, we can leave it like this,
      //    but we have to adapt the code below.  For example, if only one out of 2 are scheduled, only 1 bit o_ACK is used

       dlsch_ACK[0] = phy_vars_eNB->ulsch_eNB[(u8)UE_id]->o_ACK[0];
       dlsch_ACK[1] = phy_vars_eNB->ulsch_eNB[(u8)UE_id]->o_ACK[1];
       //      printf("UE %d: ACK %d,%d\n",UE_id,dlsch_ACK[0],dlsch_ACK[1]);
    }

    else {  // PUCCH ACK/NAK
      if ((SR_payload == 1)&&(pucch_sel!=2)) {  // decode Table 7.3 if multiplexing and SR=1 
	nb_ACK = 0;
	if (M == 2) {
	  if ((pucch_payload[0] == 1) && (pucch_payload[1] == 1)) // b[0],b[1]
	    nb_ACK = 1;
	  else if ((pucch_payload[0] == 1) && (pucch_payload[1] == 0))
	    nb_ACK = 2;
	}
	else if (M == 3) {
	  if ((pucch_payload[0] == 1) && (pucch_payload[1] == 1)) 
	    nb_ACK = 1;
	  else if ((pucch_payload[0] == 1) && (pucch_payload[1] == 0))
	    nb_ACK = 2;
	  else if ((pucch_payload[0] == 0) && (pucch_payload[1] == 1))
	    nb_ACK = 3;
	}
      }
      else if (pucch_sel == 2) {  // bundling or M=1
	//	printf("*** (%d,%d)\n",pucch_payload[0],pucch_payload[1]);
	dlsch_ACK[0] = pucch_payload[0];
	dlsch_ACK[1] = pucch_payload[0];
      }
      else {  // multiplexing with no SR, this is table 10.1
	if (M==1) 
	  dlsch_ACK[0] = pucch_payload[0];
	else if (M==2) {
	  if (((pucch_sel == 1) && (pucch_payload[0] == 1) && (pucch_payload[1] == 1)) ||
	      ((pucch_sel == 0) && (pucch_payload[0] == 0) && (pucch_payload[1] == 1)))
	    dlsch_ACK[0] = 1;
	  else
	    dlsch_ACK[0] = 0;

	  if (((pucch_sel == 1) && (pucch_payload[0] == 1) && (pucch_payload[1] == 1)) ||
	      ((pucch_sel == 1) && (pucch_payload[0] == 0) && (pucch_payload[1] == 0)))
	    dlsch_ACK[1] = 1;
	  else
	    dlsch_ACK[1] = 0;
	}
      }
    }
  }

  // handle case where positive SR was transmitted with multiplexing
  if ((SR_payload == 1)&&(pucch_sel!=2)&&(pusch_flag == 0)) {
    nb_alloc = 0;
    for (m=0;m<M;m++) {
      dl_subframe = ul_ACK_subframe2_dl_subframe(&phy_vars_eNB->lte_frame_parms,
						 subframe,
						 m);
      
      if (dlsch->subframe_tx[dl_subframe]==1) 
	nb_alloc++;
    }
    if (nb_alloc == nb_ACK)
      all_ACKed = 1;
    else 
      all_ACKed = 0;

    //    printf("nb_alloc %d, all_ACKed %d\n",nb_alloc,all_ACKed);
  }


  for (m=0,mp=-1;m<M;m++) {

    dl_subframe = ul_ACK_subframe2_dl_subframe(&phy_vars_eNB->lte_frame_parms,
					       subframe,
					       m);

    if (dlsch->subframe_tx[dl_subframe]==1) {
      if (pusch_flag == 1)
	mp++;
      else
	mp = m;

      dl_harq_pid[m]     = dlsch->harq_ids[dl_subframe];

      if ((pucch_sel != 2)&&(pusch_flag == 0)) { // multiplexing
	if ((SR_payload == 1)&&(all_ACKed == 1))
	  dlsch_ACK[m] = 1;
	else	
	  dlsch_ACK[m] = 0;
      }
      if (dl_harq_pid[m]<dlsch->Mdlharq) {
	dlsch_harq_proc = dlsch->harq_processes[dl_harq_pid[m]];
#ifdef DEBUG_PHY_PROC	
	LOG_D(PHY,"[eNB %d][PDSCH %x/%d] subframe %d, status %d, round %d\n",phy_vars_eNB->Mod_id,
	      dlsch->rnti,dl_harq_pid[m],dl_subframe,
	      dlsch_harq_proc->status,dlsch_harq_proc->round);
#endif
	if ((dl_harq_pid[m]<dlsch->Mdlharq) &&
	    (dlsch_harq_proc->status == ACTIVE)) {
	  // dl_harq_pid of DLSCH is still active
	  
	  //	  msg("[PHY] eNB %d Process %d is active (%d)\n",phy_vars_eNB->Mod_id,dl_harq_pid[m],dlsch_ACK[m]);
	  if ( dlsch_ACK[mp]==0) {
	    // Received NAK 
#ifdef DEBUG_PHY_PROC	
	    LOG_I(PHY,"[eNB %d][PDSCH %x/%d] NAK Received in round %d, requesting retransmission\n",phy_vars_eNB->Mod_id,
		dlsch->rnti,dl_harq_pid[m],dlsch_harq_proc->round);
#endif
	    
	    //	    if (dlsch_harq_proc->round == 0)
	    ue_stats->dlsch_NAK[dlsch_harq_proc->round]++;
	    
	    // then Increment DLSCH round index 
	    dlsch_harq_proc->round++;
	    
	    if (dlsch_harq_proc->round == dlsch->Mdlharq) {
	      // This was the last round for DLSCH so reset round and increment l2_error counter
#ifdef DEBUG_PHY_PROC	
	      LOG_I(PHY,"[eNB %d][PDSCH %x/%d] DLSCH retransmissions exhausted, dropping packet\n",phy_vars_eNB->Mod_id,
		    dlsch->rnti,dl_harq_pid[m]);
#endif
	      dlsch_harq_proc->round = 0;
	      ue_stats->dlsch_l2_errors++;
	      dlsch_harq_proc->status = SCH_IDLE;
	      dlsch->harq_ids[dl_subframe] = dlsch->Mdlharq;
	    }
	  }
	  else {
#ifdef DEBUG_PHY_PROC	
	    LOG_I(PHY,"[eNB %d][PDSCH %x/%d] ACK Received in round %d, resetting process\n",phy_vars_eNB->Mod_id,
		dlsch->rnti,dl_harq_pid[m],dlsch_harq_proc->round);
#endif
	    ue_stats->dlsch_ACK[dlsch_harq_proc->round]++;

	    // Received ACK so set round to 0 and set dlsch_harq_pid IDLE
	    dlsch_harq_proc->round  = 0;
	    dlsch_harq_proc->status = SCH_IDLE; 
	    dlsch->harq_ids[dl_subframe] = dlsch->Mdlharq;

	    ue_stats->total_TBS = ue_stats->total_TBS + 
	      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[dl_harq_pid[m]]->TBS;
	    ue_stats->total_transmitted_bits = ue_stats->total_transmitted_bits +
	      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[dl_harq_pid[m]]->TBS;
	  
	  }
	  
	  phy_vars_eNB->eNB_UE_stats[(u8)UE_id].dlsch_bitrate = (phy_vars_eNB->eNB_UE_stats[(u8)UE_id].total_TBS - 
						       phy_vars_eNB->eNB_UE_stats[(u8)UE_id].total_TBS_last)*10;
	
	  phy_vars_eNB->eNB_UE_stats[(u8)UE_id].total_TBS_last = phy_vars_eNB->eNB_UE_stats[(u8)UE_id].total_TBS;
	  
	  // Do fine-grain rate-adaptation for DLSCH 
	  if (ue_stats->dlsch_NAK[0] > dlsch->error_threshold) {
	    if (ue_stats->dlsch_mcs_offset == 1)
	      ue_stats->dlsch_mcs_offset=0;
	    else
	      ue_stats->dlsch_mcs_offset=-1;
	  }
#ifdef DEBUG_PHY_PROC	  
	  LOG_D(PHY,"[process_HARQ_feedback] Frame %d Setting round to %d for pid %d (subframe %d)\n",phy_vars_eNB->frame,
		 dlsch_harq_proc->round,dl_harq_pid[m],subframe);
#endif
	  
	  // Clear NAK stats and adjust mcs offset
	  // after measurement window timer expires
	  if ((ue_stats->dlsch_sliding_cnt == dlsch->ra_window_size) ) {
	    if ((ue_stats->dlsch_mcs_offset == 0) && (ue_stats->dlsch_NAK[0] < 2))
	      ue_stats->dlsch_mcs_offset = 1;
	    if ((ue_stats->dlsch_mcs_offset == 1) && (ue_stats->dlsch_NAK[0] > 2))
	      ue_stats->dlsch_mcs_offset = 0;
	    if ((ue_stats->dlsch_mcs_offset == 0) && (ue_stats->dlsch_NAK[0] > 2))
	      ue_stats->dlsch_mcs_offset = -1;
	    if ((ue_stats->dlsch_mcs_offset == -1) && (ue_stats->dlsch_NAK[0] < 2))
	      ue_stats->dlsch_mcs_offset = 0;
	    
	    for (j=0;j<phy_vars_eNB->dlsch_eNB[j][0]->Mdlharq;j++)
	      ue_stats->dlsch_NAK[j] = 0;
	    ue_stats->dlsch_sliding_cnt = 0;
	  }
	  
	  
	}
      }
    }
  }
}

void get_n1_pucch_eNB(PHY_VARS_eNB *phy_vars_eNB,
		      u8 UE_id,
		      u8 subframe,
		      s16 *n1_pucch0,
		      s16 *n1_pucch1,
		      s16 *n1_pucch2,
		      s16 *n1_pucch3) {

  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_eNB->lte_frame_parms;
  u8 nCCE0,nCCE1;

  if (frame_parms->frame_type == FDD ) { 
    if (phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[(subframe-4)%10]>0) {
      *n1_pucch0 = frame_parms->pucch_config_common.n1PUCCH_AN + phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->nCCE[(subframe-4)%10];
      *n1_pucch1 = -1;
    }
    else {
      *n1_pucch0 = -1;
      *n1_pucch1 = -1;
    }
  }
  else {

    switch (frame_parms->tdd_config) {
    case 1:  // DL:S:UL:UL:DL:DL:S:UL:UL:DL
      if (subframe == 2) {  // ACK subframes 5 and 6
	//	harq_ack[5].nCCE;  
	//harq_ack[6].nCCE;
	
      }
      else if (subframe == 3) {   // ACK subframe0
	//harq_ack[9].nCCE;
	
      }
      else if (subframe == 4) {  // nothing
	
      }
      else if (subframe == 7) {  // ACK subframes 0 and 1
	//harq_ack[0].nCCE;  
	//harq_ack[1].nCCE;
	
      }
      else if (subframe == 8) {   // ACK subframes 4
	//harq_ack[4].nCCE;
      }
      else {
	LOG_D(PHY,"[eNB %d] frame %d: phy_procedures_lte.c: get_n1pucch, illegal subframe %d for tdd_config %d\n",
	    phy_vars_eNB->Mod_id,
	    phy_vars_eNB->frame,
	    subframe,frame_parms->tdd_config);
	return;
      }
      break;
    case 3:  // DL:S:UL:UL:UL:DL:DL:DL:DL:DL
      if (subframe == 2) {  // ACK subframes 5,6 and 1 (S in frame-2), forget about n-11 for the moment (S-subframe)
	if (phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[6]>0) {
	  nCCE1 = phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->nCCE[6];
	  *n1_pucch1 = get_Np(frame_parms->N_RB_DL,nCCE1,1) + nCCE1 + frame_parms->pucch_config_common.n1PUCCH_AN; 
	}
	else
	  *n1_pucch1 = -1;

	if (phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[5]>0) {
	  nCCE0 = phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->nCCE[5];
	  *n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0+ frame_parms->pucch_config_common.n1PUCCH_AN; 
	}
	else
	  *n1_pucch0 = -1;
      }
      else if (subframe == 3) {   // ACK subframes 7 and 8
	LOG_D(PHY,"get_n1_pucch_eNB : subframe 3, subframe_tx[7] %d, subframe_tx[8] %d\n",
	      phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[7],phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[8]);

	if (phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[8]>0) {
	  nCCE1 = phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->nCCE[8];
	  *n1_pucch1 = get_Np(frame_parms->N_RB_DL,nCCE1,1) + nCCE1 + frame_parms->pucch_config_common.n1PUCCH_AN;
	  LOG_D(PHY,"nCCE1 %d, n1_pucch1 %d\n",nCCE1,*n1_pucch1);
	}
	else
	  *n1_pucch1 = -1;

	if (phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[7]>0) {
	  nCCE0 = phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->nCCE[7];
	  *n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0 +frame_parms->pucch_config_common.n1PUCCH_AN; 
	  LOG_D(PHY,"nCCE0 %d, n1_pucch0 %d\n",nCCE0,*n1_pucch0);
	}
	else
	  *n1_pucch0 = -1;
      }
      else if (subframe == 4) {  // ACK subframes 9 and 0
	if (phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[0]>0) {
	  nCCE1 = phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->nCCE[0];
	  *n1_pucch1 = get_Np(frame_parms->N_RB_DL,nCCE1,1) + nCCE1 + frame_parms->pucch_config_common.n1PUCCH_AN; 
	}
	else
	  *n1_pucch1 = -1;

	if (phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->subframe_tx[9]>0) {
	  nCCE0 = phy_vars_eNB->dlsch_eNB[(u32)UE_id][0]->nCCE[9];
	  *n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0 +frame_parms->pucch_config_common.n1PUCCH_AN; 
	}
	else
	  *n1_pucch0 = -1;
      }
      else {
	LOG_D(PHY,"[eNB %d] Frame %d: phy_procedures_lte.c: get_n1pucch, illegal subframe %d for tdd_config %d\n",
	    phy_vars_eNB->Mod_id,phy_vars_eNB->frame,subframe,frame_parms->tdd_config);
	return;
      }
      break;
    }  // switch tdd_config     
    // Don't handle the case M>2
    *n1_pucch2 = -1;
    *n1_pucch3 = -1;
  }
}


void prach_procedures(PHY_VARS_eNB *phy_vars_eNB,u8 subframe,u8 abstraction_flag) {

  u16 preamble_energy_list[64],preamble_delay_list[64];
  u16 preamble_max,preamble_energy_max;
  u16 i;
  u8 UE_id;

  if (abstraction_flag == 0) {
    LOG_D(PHY,"[eNB %d][RAPROC] Frame %d, Subframe %d : PRACH RX Signal Power : %d dBm\n",phy_vars_eNB->Mod_id,
	  phy_vars_eNB->frame,subframe,
	  dB_fixed(signal_energy(&phy_vars_eNB->lte_eNB_common_vars.rxdata[0][0][subframe*phy_vars_eNB->lte_frame_parms.samples_per_tti],
				 512)) - phy_vars_eNB->rx_total_gain_eNB_dB);
    
    rx_prach(phy_vars_eNB,
	     subframe,
	     preamble_energy_list,
	     preamble_delay_list,
	     phy_vars_eNB->frame,
	     0);
  }
  else {
    memset(&preamble_energy_list[0],0,64*sizeof(u16));
    memset(&preamble_delay_list[0],0,64*sizeof(u16));
    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
      
      LOG_D(PHY,"[RAPROC] UE_id %d, generate_prach %d, UE RSI %d, eNB RSI %d preamble index %d\n",
	     UE_id,PHY_vars_UE_g[UE_id]->generate_prach,
	     PHY_vars_UE_g[UE_id]->lte_frame_parms.prach_config_common.rootSequenceIndex,
	    phy_vars_eNB->lte_frame_parms.prach_config_common.rootSequenceIndex,
	    PHY_vars_UE_g[UE_id]->prach_PreambleIndex);
      
      if ((PHY_vars_UE_g[UE_id]->generate_prach==1) &&
	  (PHY_vars_UE_g[UE_id]->lte_frame_parms.prach_config_common.rootSequenceIndex ==
	   phy_vars_eNB->lte_frame_parms.prach_config_common.rootSequenceIndex) ) {
	preamble_energy_list[PHY_vars_UE_g[UE_id]->prach_PreambleIndex] = 80;
	preamble_delay_list[PHY_vars_UE_g[UE_id]->prach_PreambleIndex] = 5;
	
      }
    }
  }
  preamble_energy_max = preamble_energy_list[0];
  preamble_max = 0;

  for (i=1;i<64;i++) {
    if (preamble_energy_max < preamble_energy_list[i]) {
      preamble_energy_max = preamble_energy_list[i];
      preamble_max = i;
    }
  }

  /*  
  msg("[PHY][RAPROC] Most likely preamble %d, energy %d dB delay %d\n",
      preamble_max,
      preamble_energy_list[preamble_max],
      preamble_delay_list[preamble_max]);
  */
  if (preamble_energy_list[preamble_max] > 60) {
    UE_id = find_next_ue_index(phy_vars_eNB);
    if (UE_id>=0) {
      phy_vars_eNB->eNB_UE_stats[(u32)UE_id].UE_timing_offset = preamble_delay_list[preamble_max];
      //phy_vars_eNb->eNB_UE_stats[(u32)UE_id].mode = PRACH;
      phy_vars_eNB->eNB_UE_stats[(u32)UE_id].sector = 0;
      LOG_I(PHY,"[eNB %d][RAPROC] Initiating RA procedure with preamble %d, energy %d, delay %d\n",
	  phy_vars_eNB->Mod_id,
	  preamble_max,
	  preamble_energy_max,
	  preamble_delay_list[preamble_max]);
	  
      mac_xface->initiate_ra_proc(phy_vars_eNB->Mod_id,
				  phy_vars_eNB->frame,
				  preamble_max,
				  preamble_delay_list[preamble_max],
				  0,subframe,0);
    }
    else {
      LOG_D(PHY,"[eNB %d] frame %d, subframe %d: Unable to add user, max user count reached\n", 
	    phy_vars_eNB->Mod_id,phy_vars_eNB->frame, subframe);
    }
  }
}

void ulsch_decoding_procedures(unsigned char last_slot, unsigned int i, PHY_VARS_eNB *phy_vars_eNB, unsigned char abstraction_flag)
{
    LOG_D(PHY,"ulsch_decoding_procedures not yet implemented. should not be called");
}


void phy_procedures_eNB_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {
  //RX processing
  u32 l, ret,i,j;
  u32 sect_id=0;
  u32 harq_pid, round;
  u8 SR_payload,*pucch_payload=NULL,pucch_payload0[2]={0,0},pucch_payload1[2]={0,0};
  s16 n1_pucch0,n1_pucch1,n1_pucch2,n1_pucch3;
  u8 do_SR=0,pucch_sel;
  s16 metric0=0,metric1=0;
  ANFBmode_t bundling_flag;
  PUCCH_FMT_t format;
  u8 nPRS;
  u8 two_ues_connected = 0;
  u8 pusch_active = 0;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_eNB->lte_frame_parms;

  if (abstraction_flag == 0) {
    remove_7_5_kHz(phy_vars_eNB,last_slot);
  }
#ifdef OPENAIR2
  // check if we have to detect PRACH first
  if ((last_slot&1)==1){
    //    printf("Checking PRACH for eNB %d, subframe %d\n",phy_vars_eNB->Mod_id,last_slot>>1);
    if (is_prach_subframe(&phy_vars_eNB->lte_frame_parms,phy_vars_eNB->frame,last_slot>>1)>0) {
      //      printf("Running prach procedures\n");
      prach_procedures(phy_vars_eNB,last_slot>>1,abstraction_flag);
    }
  }
#endif
  if (abstraction_flag == 0) {
    for (l=0;l<phy_vars_eNB->lte_frame_parms.symbols_per_tti/2;l++) {
      
      for (sect_id=0;sect_id<number_of_cards;sect_id++) {
	slot_fep_ul(&phy_vars_eNB->lte_frame_parms,
		    &phy_vars_eNB->lte_eNB_common_vars,
		    l,
		    last_slot,
		    sect_id,
#ifdef HW_PREFIX_REMOVAL
		    1
#else
		    0
#endif
		    );
      }
    }
  }
  sect_id = 0;

  /*
  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
    
    if ((phy_vars_eNB->eNB_UE_stats[(u32)UE_id].mode>PRACH) && (last_slot%2==1)) {
#ifdef DEBUG_PHY_PROC	
      LOG_D(PHY,"[eNB %d] frame %d, slot %d: Doing SRS estimation and measurements for UE_id %d (UE_mode %d)\n",
	    phy_vars_eNB->Mod_id,
	    phy_vars_eNB->frame, last_slot, 
	    UE_id,phy_vars_eNB->eNB_UE_stats[(u32)UE_id].mode);
#endif
      for (sect_id=0;sect_id<number_of_cards;sect_id++) {
	
	lte_srs_channel_estimation(&phy_vars_eNB->lte_frame_parms,
				   &phy_vars_eNB->lte_eNB_common_vars,
				   &phy_vars_eNB->lte_eNB_srs_vars[(u32)UE_id],
				   &phy_vars_eNB->soundingrs_ul_config_dedicated[(u32)UE_id],
				   last_slot>>1,
				   sect_id);
	lte_eNB_srs_measurements(phy_vars_eNB,
				 sect_id,
				 UE_id,
				 1);
#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[eNB %d] frame %d, slot %d: UE_id %d, sect_id %d: RX RSSI %d (from SRS)\n",
	      phy_vars_eNB->Mod_id,
	      phy_vars_eNB->frame, last_slot, 
	      UE_id,sect_id,
	      phy_vars_eNB->PHY_measurements_eNB[sect_id].rx_rssi_dBm[(u32)UE_id]);
#endif
      }
      
      sect_id=0;
#ifdef USER_MODE
      //write_output("srs_est0.m","srsest0",phy_vars_eNB->lte_eNB_common_vars.srs_ch_estimates[0][0],512,1,1);
      //write_output("srs_est1.m","srsest1",phy_vars_eNB->lte_eNB_common_vars.srs_ch_estimates[0][1],512,1,1);
#endif
      
      //msg("timing advance in\n");
      sync_pos = lte_est_timing_advance(&phy_vars_eNB->lte_frame_parms,
					&phy_vars_eNB->lte_eNB_srs_vars[(u32)UE_id],
					&sect_id,
					phy_vars_eNB->first_run_timing_advance[(u32)UE_id],
					number_of_cards,
					24576);
      
      //msg("timing advance out\n");
      
      //phy_vars_eNB->eNB_UE_stats[(u32)UE_id].UE_timing_offset = sync_pos - phy_vars_eNB->lte_frame_parms.nb_prefix_samples/8;
      phy_vars_eNB->eNB_UE_stats[(u32)UE_id].UE_timing_offset = 0;
      phy_vars_eNB->eNB_UE_stats[(u32)UE_id].sector = sect_id;
#ifdef DEBUG_PHY_PROC	
      LOG_D(PHY,"[eNB %d] frame %d, slot %d: user %d in sector %d: timing_advance = %d\n",
	    phy_vars_eNB->Mod_id,
	    phy_vars_eNB->frame, last_slot, 
	    UE_id, sect_id,
	    phy_vars_eNB->eNB_UE_stats[(u32)UE_id].UE_timing_offset);
#endif
    }
  }
  else {
    
  }
  */
  // Check for active processes in current subframe
  harq_pid = subframe2harq_pid(&phy_vars_eNB->lte_frame_parms,
			       ((last_slot>>1)==9 ? -1 : 0 )+ phy_vars_eNB->frame,last_slot>>1);
  //  printf("[eNB][PUSCH] subframe %d => harq_pid %d\n",last_slot>>1,harq_pid);

#ifdef OPENAIR2
  if ((phy_vars_eNB->eNB_UE_stats[0].mode == PUSCH) && 
      (phy_vars_eNB->eNB_UE_stats[1].mode == PUSCH))
    two_ues_connected = 1;
#else
    two_ues_connected = 1;
#endif

  pusch_active = 0;
  for (i=0;i<NUMBER_OF_UE_MAX;i++) { 

    /*
    if ((i == 1) && (phy_vars_eNB->cooperation_flag > 0) && (two_ues_connected == 1))
      break;
    */
#ifdef OPENAIR2
    if (phy_vars_eNB->eNB_UE_stats[i].mode == RA_RESPONSE)
      process_Msg3(phy_vars_eNB,last_slot,i,harq_pid);
#endif

    /*
#ifdef DEBUG_PHY_PROC
    if (phy_vars_eNB->ulsch_eNB[i]) {
      printf("[PHY][eNB %d][PUSCH %d] frame %d, subframe %d rnti %x, alloc %d\n",phy_vars_eNB->Mod_id,
	     harq_pid,phy_vars_eNB->frame,last_slot>>1,
	     (phy_vars_eNB->ulsch_eNB[i]->rnti),
	     (phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->subframe_scheduling_flag) 
	     );
    }
#endif
    */
    if ((phy_vars_eNB->ulsch_eNB[i]) &&
	(phy_vars_eNB->ulsch_eNB[i]->rnti>0) &&
	(phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->subframe_scheduling_flag==1) && 
	((last_slot%2)==1)) {
 
      pusch_active = 1;
      round = phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round;

#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d][PUSCH %d] frame %d subframe %d Scheduling PUSCH/ULSCH Reception for rnti %x\n",
	  phy_vars_eNB->Mod_id,harq_pid,
	  (((last_slot>>1)==9) ? -1 : 0)+phy_vars_eNB->frame,last_slot>>1,phy_vars_eNB->ulsch_eNB[i]->rnti);
#endif
  
#ifdef DEBUG_PHY_PROC
      if (phy_vars_eNB->ulsch_eNB[i]->Msg3_flag == 1){
	LOG_I(PHY,"[eNB %d] frame %d, slot %d, subframe %d: Scheduling ULSCH Reception for Msg3 in Sector %d\n",
	      phy_vars_eNB->Mod_id,
	      (((last_slot>>1)==9)?-1:0)+phy_vars_eNB->frame,
	      last_slot,last_slot>>1,
	      phy_vars_eNB->eNB_UE_stats[i].sector);
      } else {
	LOG_F(PHY,"[eNB %d] frame %d, slot %d, subframe %d: Scheduling ULSCH Reception for UE %d Mode %s sect_id %d\n",
	    phy_vars_eNB->Mod_id,
	    (((last_slot>>1)==9)?-1:0)+phy_vars_eNB->frame,
	      last_slot,last_slot>>1,
	      i,
	      mode_string[phy_vars_eNB->eNB_UE_stats[i].mode],
	      phy_vars_eNB->eNB_UE_stats[i].sector);
      }
#endif

      nPRS = 0;

      phy_vars_eNB->ulsch_eNB[i]->cyclicShift = (phy_vars_eNB->ulsch_eNB[i]->n_DMRS2 + phy_vars_eNB->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift + nPRS)%12;

#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d][PUSCH %d] Frame %d Subframe %d Demodulating PUSCH: dci_alloc %d, rar_alloc %d, round %d, Ndi %d, first_rb %d, nb_rb %d, mcs %d, rv %d, cyclic_shift %d (n_DMRS2 %d, cyclicShift %d, nprs %d), O_ACK %d \n",
	    phy_vars_eNB->Mod_id,harq_pid,(((last_slot>>1)==9)?-1:0)+phy_vars_eNB->frame,last_slot>>1,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->dci_alloc,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->rar_alloc,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->Ndi,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->first_rb,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->nb_rb,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->mcs,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->rvidx,
	    phy_vars_eNB->ulsch_eNB[i]->cyclicShift,
	    phy_vars_eNB->ulsch_eNB[i]->n_DMRS2,
	    phy_vars_eNB->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift,
	    nPRS,
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->O_ACK);
#endif
      if (abstraction_flag==0) {
	rx_ulsch(phy_vars_eNB,
		 last_slot>>1,
		 phy_vars_eNB->eNB_UE_stats[i].sector,  // this is the effective sector id
		 i,
		 phy_vars_eNB->ulsch_eNB,
		 0);
      }
#ifdef PHY_ABSTRACTION
      else {
	rx_ulsch_emul(phy_vars_eNB,
		      last_slot>>1,
		      phy_vars_eNB->eNB_UE_stats[i].sector,  // this is the effective sector id
		      i);
      }
#endif

      for (j=0;j<phy_vars_eNB->lte_frame_parms.nb_antennas_rx;j++)
	phy_vars_eNB->eNB_UE_stats[i].UL_rssi[j] = dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power[j]) - phy_vars_eNB->rx_total_gain_eNB_dB;

      if (abstraction_flag == 0) {
	ret = ulsch_decoding(phy_vars_eNB,
			     i,
			     last_slot>>1,
			     0, // control_only_flag
			     0  //Nbundled, to be updated!!!!
			     );  
      }
#ifdef PHY_ABSTRACTION
      else {
	ret = ulsch_decoding_emul(phy_vars_eNB,
				  last_slot>>1,
				  i);
      }
#endif

#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d][PUSCH %d] frame %d subframe %d RX power (%d,%d) N0 (%d,%d) dB ACK (%d,%d), decoding iter %d\n",
	  phy_vars_eNB->Mod_id,harq_pid,
	  phy_vars_eNB->frame,last_slot>>1,
	  dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power[0]),
	  dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power[1]),
	  phy_vars_eNB->PHY_measurements_eNB->n0_power_dB[0],
	  phy_vars_eNB->PHY_measurements_eNB->n0_power_dB[1],
	  phy_vars_eNB->ulsch_eNB[i]->o_ACK[0],
	  phy_vars_eNB->ulsch_eNB[i]->o_ACK[1],
	  ret);
#endif //DEBUG_PHY_PROC
      /*
      if ((two_ues_connected==1) && (phy_vars_eNB->cooperation_flag==2)) {
	for (j=0;j<phy_vars_eNB->lte_frame_parms.nb_antennas_rx;j++) {
	  phy_vars_eNB->eNB_UE_stats[i].UL_rssi[j] = dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power_0[j]) 
	    - phy_vars_eNB->rx_total_gain_eNB_dB;
	  phy_vars_eNB->eNB_UE_stats[i+1].UL_rssi[j] = dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power_1[j]) 
	    - phy_vars_eNB->rx_total_gain_eNB_dB;
	}
#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[eNB %d] Frame %d subframe %d: ULSCH %d RX power UE0 (%d,%d) dB RX power UE1 (%d,%d)\n",
	      phy_vars_eNB->Mod_id,phy_vars_eNB->frame,last_slot>>1,i,
	      dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power_0[0]),
	      dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power_0[1]),
	      dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power_1[0]),
	      dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power_1[1]));
#endif
      }
      else {
      */
	for (j=0;j<phy_vars_eNB->lte_frame_parms.nb_antennas_rx;j++)
	  phy_vars_eNB->eNB_UE_stats[i].UL_rssi[j] = dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power[j]) 
	    - phy_vars_eNB->rx_total_gain_eNB_dB;
#ifdef DEBUG_PHY_PROC
	LOG_D(PHY,"[eNB %d] Frame %d subframe %d: ULSCH %d RX power (%d,%d) dB\n",
	      phy_vars_eNB->Mod_id,phy_vars_eNB->frame,last_slot>>1,i,
	      dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power[0]),
	      dB_fixed(phy_vars_eNB->lte_eNB_pusch_vars[i]->ulsch_power[1]));
#endif

	//      }
      
    
      phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round]++;
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d][PUSCH %d] frame %d subframe %d UE %d harq_pid %d Clearing subframe_scheduling_flag\n",
	    phy_vars_eNB->Mod_id,harq_pid,phy_vars_eNB->frame,last_slot>>1,i,harq_pid);
#endif
      phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->subframe_scheduling_flag=0;

      
      if (phy_vars_eNB->ulsch_eNB[i]->cqi_crc_status == 1) {
#ifdef DEBUG_PHY_PROC
	//if (((phy_vars_eNB->frame%10) == 0) || (phy_vars_eNB->frame < 50)) 
	  print_CQI(phy_vars_eNB->ulsch_eNB[i]->o,phy_vars_eNB->ulsch_eNB[i]->uci_format,0);
#endif
	extract_CQI(phy_vars_eNB->ulsch_eNB[i]->o,phy_vars_eNB->ulsch_eNB[i]->uci_format,&phy_vars_eNB->eNB_UE_stats[i]);
	phy_vars_eNB->eNB_UE_stats[i].rank = phy_vars_eNB->ulsch_eNB[i]->o_RI[0];
      }
    
      if (ret == (1+MAX_TURBO_ITERATIONS)) {
	phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors[harq_pid][phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round]++;
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->phich_active = 1;
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->phich_ACK = 0;
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round++;
	//	printf("[UE][PUSCH %d] Increasing to round %d\n",harq_pid,phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round);

	//	phy_vars_eNB->ulsch_eNB[i]->o_ACK[0] = 0;
	//	phy_vars_eNB->ulsch_eNB[i]->o_ACK[1] = 0;




	if (phy_vars_eNB->ulsch_eNB[i]->Msg3_flag == 1) {
	  LOG_I(PHY,"[eNB %d][RAPROC] frame %d, slot %d, subframe %d, UE %d: Error receiving ULSCH (Msg3), round %d/%d\n",
	      phy_vars_eNB->Mod_id,
	      phy_vars_eNB->frame,last_slot,last_slot>>1, i,
	      phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round-1,
	      phy_vars_eNB->lte_frame_parms.maxHARQ_Msg3Tx-1);

	  if (phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round == 
	      phy_vars_eNB->lte_frame_parms.maxHARQ_Msg3Tx) {
	    LOG_I(PHY,"[eNB %d][RAPROC] maxHARQ_Msg3Tx reached, abandoning RA procedure for UE %d\n",
		  phy_vars_eNB->Mod_id, i);
	    phy_vars_eNB->eNB_UE_stats[i].mode = PRACH;
#ifdef OPENAIR2
	    mac_xface->cancel_ra_proc(phy_vars_eNB->Mod_id,
				      phy_vars_eNB->frame,
				      phy_vars_eNB->eNB_UE_stats[i].crnti);
#endif
	    remove_ue(phy_vars_eNB->eNB_UE_stats[i].crnti,phy_vars_eNB,abstraction_flag);
	    phy_vars_eNB->ulsch_eNB[(u32)i]->Msg3_active = 0;
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->phich_active = 0;

	    /*
#ifdef USER_MODE
	    if (abstraction_flag == 0)
	      dump_ulsch(phy_vars_eNB);
	    exit(-1);
#endif
	    */
	  }
	  else {
	    // activate retransmission for Msg3 (signalled to UE PHY by PHICH (not MAC/DCI)
	    phy_vars_eNB->ulsch_eNB[(u32)i]->Msg3_active = 1;
	    
	    get_Msg3_alloc_ret(&phy_vars_eNB->lte_frame_parms,
			       last_slot>>1,
			       phy_vars_eNB->frame,
			       &phy_vars_eNB->ulsch_eNB[i]->Msg3_frame,
			       &phy_vars_eNB->ulsch_eNB[i]->Msg3_subframe);
	  }
	} // This is Msg3 error
	else { //normal ULSCH
	  LOG_D(PHY,"[eNB %d][PUSCH %d] frame %d subframe %d UE %d Error receiving ULSCH, round %d/%d (ACK %d,%d)\n",
		phy_vars_eNB->Mod_id,harq_pid,
		phy_vars_eNB->frame,last_slot>>1, i,
		phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round-1,
		phy_vars_eNB->ulsch_eNB[i]->Mdlharq,
		phy_vars_eNB->ulsch_eNB[i]->o_ACK[0],
		phy_vars_eNB->ulsch_eNB[i]->o_ACK[1]);

	  if (phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round==
	      phy_vars_eNB->ulsch_eNB[i]->Mdlharq) {
	    LOG_I(PHY,"[eNB %d][PUSCH %d] frame %d subframe %d UE %d ULSCH Mdlharq %d reached\n",
		phy_vars_eNB->Mod_id,harq_pid,
		phy_vars_eNB->frame,last_slot>>1, i,
		phy_vars_eNB->ulsch_eNB[i]->Mdlharq);
	    
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round=0;
	    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->phich_active=0;
	    phy_vars_eNB->eNB_UE_stats[i].ulsch_errors[harq_pid]++;
	    phy_vars_eNB->eNB_UE_stats[i].ulsch_consecutive_errors[harq_pid]++;
	  }
	
	  // If we've dropped the UE, go back to PRACH mode for this UE
	  if (phy_vars_eNB->eNB_UE_stats[i].ulsch_consecutive_errors[harq_pid] == 20) {
	    LOG_I(PHY,"[eNB %d] frame %d, subframe %d, UE %d: ULSCH consecutive error count reached, removing UE\n",
		  phy_vars_eNB->Mod_id,phy_vars_eNB->frame,last_slot>>1, i);
	    phy_vars_eNB->eNB_UE_stats[i].mode = PRACH;
#ifdef OPENAIR2
	    mac_xface->cancel_ra_proc(phy_vars_eNB->Mod_id,
				      phy_vars_eNB->frame,
				      phy_vars_eNB->eNB_UE_stats[i].crnti);
#endif
	    remove_ue(phy_vars_eNB->eNB_UE_stats[i].crnti,phy_vars_eNB,abstraction_flag);
	    phy_vars_eNB->eNB_UE_stats[i].ulsch_consecutive_errors[harq_pid]=0;
	  }
	}
      }  // ulsch in error
      else {
	LOG_I(PHY,"[eNB %d][PUSCH %d] Frame %d subframe %d ULSCH received, setting round to 0, PHICH ACK\n",
	    phy_vars_eNB->Mod_id,harq_pid,
	    phy_vars_eNB->frame,last_slot>>1);	    

	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->phich_active = 1;
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->phich_ACK = 1;
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round = 0;
	phy_vars_eNB->eNB_UE_stats[i].ulsch_consecutive_errors[harq_pid] = 0;

	if (phy_vars_eNB->ulsch_eNB[i]->Msg3_flag == 1) {
#ifdef OPENAIR2
#ifdef DEBUG_PHY_PROC
	  LOG_I(PHY,"[eNB %d][RAPROC] Frame %d Terminating ra_proc for harq %d, UE %d\n",
		phy_vars_eNB->Mod_id,
		phy_vars_eNB->frame,harq_pid,i);
#endif
	  mac_xface->terminate_ra_proc(phy_vars_eNB->Mod_id,
				       phy_vars_eNB->frame,
				       phy_vars_eNB->ulsch_eNB[i]->rnti,
				       phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->b,
				       phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->TBS>>3);
#endif

	  phy_vars_eNB->eNB_UE_stats[i].mode = PUSCH;
	  phy_vars_eNB->ulsch_eNB[i]->Msg3_flag = 0;

#ifdef DEBUG_PHY_PROC
	  LOG_I(PHY,"[eNB %d][RAPROC] Frame %d : RX Subframe %d Setting UE %d mode to PUSCH\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame,last_slot>>1,i);
#endif //DEBUG_PHY_PROC

	  for (j=0;j<phy_vars_eNB->dlsch_eNB[i][0]->Mdlharq;j++) {
	    phy_vars_eNB->eNB_UE_stats[i].dlsch_NAK[j]=0;
	    phy_vars_eNB->eNB_UE_stats[i].dlsch_sliding_cnt=0;
	  }

	  //mac_xface->macphy_exit("Mode PUSCH. Exiting.\n");
	}
	else {
	  /*
#ifdef DEBUG_PHY_PROC
	  LOG_D(PHY,"[eNB] Frame %d, Subframe %d : ULSCH SDU (RX) %d bytes:",phy_vars_eNB->frame,last_slot>>1,phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->TBS>>3);
	  for (j=0;j<phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->TBS>>3;j++)
	    msg("%x.",phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->b[j]);
	  msg("\n");
#endif
	  */

#ifdef OPENAIR2
	  //	  if (phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->calibration_flag == 0) {
	  mac_xface->rx_sdu(phy_vars_eNB->Mod_id,
			    phy_vars_eNB->frame,
			    phy_vars_eNB->ulsch_eNB[i]->rnti,
			    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->b,
			    phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->TBS>>3);
	  //}
	  /*
	  else {
	    // Retrieve calibration information and do whatever
	    LOG_D(PHY,"[eNB][Auto-Calibration] Frame %d, Subframe %d : ULSCH PDU (RX) %d bytes\n",phy_vars_eNB->frame,last_slot>>1,phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->TBS>>3);	    
	  }
	  */
#endif
	}
      }  // ulsch not in error
      
      // process HARQ feedback
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d][PDSCH %x] Frame %d subframe %d, Processing HARQ feedback for UE %d\n",phy_vars_eNB->Mod_id,
	    phy_vars_eNB->dlsch_eNB[i][0]->rnti,
	    phy_vars_eNB->frame,last_slot>>1,
	    i);
#endif
      process_HARQ_feedback(i,
			    last_slot>>1,
			    phy_vars_eNB,
			    1, // pusch_flag
			    0,
			    0,
			    0);
      
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[eNB %d] Frame %d subframe %d, sect %d: received ULSCH harq_pid %d for UE %d, ret = %d, CQI CRC Status %d, ACK %d,%d, ulsch_errors %d/%d\n", 
	    phy_vars_eNB->Mod_id,
	    phy_vars_eNB->frame, last_slot>>1, 
	    phy_vars_eNB->eNB_UE_stats[i].sector, 
	    harq_pid, 
	    i, 
	    ret, 
	    phy_vars_eNB->ulsch_eNB[i]->cqi_crc_status, 
	    phy_vars_eNB->ulsch_eNB[i]->o_ACK[0],	
	    phy_vars_eNB->ulsch_eNB[i]->o_ACK[1], 
	    phy_vars_eNB->eNB_UE_stats[i].ulsch_errors[harq_pid],
	    phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][0]);
#endif


      if (phy_vars_eNB->frame % 100 == 0) {
	if ((phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][round] - 
	     phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts_last[harq_pid][round]) != 0)
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_round_fer[harq_pid][round] = 
	    (100*(phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors[harq_pid][round] - 
		  phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors_last[harq_pid][round]))/
	    (phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][round] - 
	     phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts_last[harq_pid][round]);

	//phy_vars_eNB->eNB_UE_stats[i].dlsch_bitrate = phy_vars_eNB->eNB_UE_stats[i].total_TBS - phy_vars_eNB->eNB_UE_stats[i].total_TBS_last;

	//phy_vars_eNB->eNB_UE_stats[i].total_TBS_last = phy_vars_eNB->eNB_UE_stats[i].total_TBS;
	phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts_last[harq_pid][round] = 
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][round];
	phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors_last[harq_pid][round] = 
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors[harq_pid][round];
      }

      /*      if(phy_vars_eNB->frame % 10 == 0) {
	phy_vars_eNB->eNB_UE_stats[i].dlsch_bitrate = (phy_vars_eNB->eNB_UE_stats[i].total_TBS - 
						       phy_vars_eNB->eNB_UE_stats[i].total_TBS_last)*10;
	
	phy_vars_eNB->eNB_UE_stats[i].total_TBS_last = phy_vars_eNB->eNB_UE_stats[i].total_TBS;
     } */ 
    }
  

#ifdef PUCCH
    else if ((phy_vars_eNB->dlsch_eNB[i][0]) &&
	     (phy_vars_eNB->dlsch_eNB[i][0]->rnti>0) &&
	     ((last_slot%2)==1)){ // check for PUCCH

      // check SR availability
      do_SR = is_SR_subframe(phy_vars_eNB,i,last_slot>>1);
      //      do_SR = 0;
    
      // Now ACK/NAK
      // First check subframe_tx flag for earlier subframes
      get_n1_pucch_eNB(phy_vars_eNB,
		       i,
		       last_slot>>1,
		       &n1_pucch0,
		       &n1_pucch1,
		       &n1_pucch2,
		       &n1_pucch3);

      if ((n1_pucch0==-1) && (n1_pucch1==-1) && (do_SR==0)) {  // no TX PDSCH that have to be checked and no SR for this UE_id
      }
      else {
	// otherwise we have some PUCCH detection to do
      
	if (do_SR == 1) {
	
	  if (abstraction_flag == 0)
	    metric0 = rx_pucch(phy_vars_eNB,
			       pucch_format1,
			       i,
			       phy_vars_eNB->scheduling_request_config[i].sr_PUCCH_ResourceIndex,
			       0, // n2_pucch
			       1, // shortened format
			       &SR_payload,
			       last_slot>>1,
			       PUCCH1_THRES);
#ifdef PHY_ABSTRACTION
	  else {
	    metric0 = rx_pucch_emul(phy_vars_eNB,
				    i,
				    pucch_format1,
				    0,
				    &SR_payload,
				    last_slot>>1);
	    LOG_D(PHY,"[eNB %d][SR %x] Frame %d subframe %d Checking SR (UE SR %d/%d)\n",phy_vars_eNB->Mod_id,
		  phy_vars_eNB->ulsch_eNB[i]->rnti,phy_vars_eNB->frame,last_slot>>1,SR_payload,phy_vars_eNB->scheduling_request_config[i].sr_PUCCH_ResourceIndex);
#endif
	  }

	  if (SR_payload == 1) {
	    LOG_I(PHY,"[eNB %d][SR %x] Frame %d subframe %d Got SR for PUSCH, transmitting to MAC\n",phy_vars_eNB->Mod_id,
		phy_vars_eNB->ulsch_eNB[i]->rnti,phy_vars_eNB->frame,last_slot>>1);
	    if (phy_vars_eNB->first_sr[i] == 1) { // this is the first request for uplink after Connection Setup, so clear HARQ process 0 use for Msg4
	      phy_vars_eNB->first_sr[i] = 0;
	      phy_vars_eNB->dlsch_eNB[i][0]->harq_processes[0]->round=0;
	      phy_vars_eNB->dlsch_eNB[i][0]->harq_processes[0]->status=SCH_IDLE;
	      LOG_D(PHY,"[eNB %d][SR %x] Frame %d subframe %d First SR\n",
		    phy_vars_eNB->Mod_id,
		    phy_vars_eNB->ulsch_eNB[i]->rnti,phy_vars_eNB->frame,last_slot>>1);
	    }
	    mac_xface->SR_indication(phy_vars_eNB->Mod_id,
				     phy_vars_eNB->frame,
				     phy_vars_eNB->dlsch_eNB[i][0]->rnti,last_slot>>1);
	    
	  } 
	}// do_SR==1
	if ((n1_pucch0==-1) && (n1_pucch1==-1)) { // just check for SR
	}
	else if (phy_vars_eNB->lte_frame_parms.frame_type==0) { // FDD
	  // if SR was detected, use the n1_pucch from SR, else use n1_pucch0
	  n1_pucch0 = (SR_payload==1) ? phy_vars_eNB->scheduling_request_config[i].sr_PUCCH_ResourceIndex:n1_pucch0;
	  if (abstraction_flag == 0)
	    metric0 = rx_pucch(phy_vars_eNB,
			       pucch_format1a,
			       i,
			       (u16)n1_pucch0,
			       0, //n2_pucch
			       1, // shortened format
			       pucch_payload0,
			       last_slot>>1,
			       PUCCH1_THRES);
	  else {
#ifdef PHY_ABSTRACTION
	    metric0 = rx_pucch_emul(phy_vars_eNB,i,
				    pucch_format1a,
				    0,
				    pucch_payload0,
				    last_slot>>1);
#endif
	  }	
	} // FDD
	else {  //TDD
	
	  bundling_flag = phy_vars_eNB->pucch_config_dedicated[i].tdd_AckNackFeedbackMode;
	
	  // fix later for 2 TB case and format1b

	  if ((frame_parms->frame_type==0) || 
	      (bundling_flag==bundling)    || 
	      ((frame_parms->frame_type==1)&&(frame_parms->tdd_config==1)&&((last_slot!=4)||(last_slot!=14)))) {
	    format = pucch_format1a;
	    //	  msg("PUCCH 1a\n");
	  }
	  else {
	    format = pucch_format1b;
	    //	  msg("PUCCH 1b\n");
	  }
	
	  // if SR was detected, use the n1_pucch from SR
	  if (SR_payload==1) {
#ifdef DEBUG_PHY_PROC	  
	    LOG_D(PHY,"[eNB %d][PDSCH %x] Frame %d subframe %d Checking ACK/NAK (%d,%d,%d,%d) with SR\n",phy_vars_eNB->Mod_id,
		phy_vars_eNB->dlsch_eNB[i][0]->rnti,
		phy_vars_eNB->frame,last_slot>>1,
		n1_pucch0,n1_pucch1,n1_pucch2,n1_pucch3);
#endif
	    if (abstraction_flag == 0) 
	      metric0 = rx_pucch(phy_vars_eNB,
				 format,
				 i,
				 phy_vars_eNB->scheduling_request_config[i].sr_PUCCH_ResourceIndex,
				 0, //n2_pucch
				 1, // shortened format
				 pucch_payload0,
				 last_slot>>1,
				 PUCCH1_THRES);
	    else {
#ifdef PHY_ABSTRACTION
	      metric0 = rx_pucch_emul(phy_vars_eNB,i,
				      format,
				      0,
				      pucch_payload0,
				      last_slot>>1);
#endif
	    } 
	  }
	  else {  //using n1_pucch0/n1_pucch1 resources
#ifdef DEBUG_PHY_PROC	  
	    LOG_D(PHY,"[eNB %d][PDSCH %x] Frame %d subframe %d Checking ACK/NAK (%d,%d,%d,%d)\n",phy_vars_eNB->Mod_id,
		phy_vars_eNB->dlsch_eNB[i][0]->rnti,
		phy_vars_eNB->frame,last_slot>>1,
		n1_pucch0,n1_pucch1,n1_pucch2,n1_pucch3);
#endif
	    metric0=0;
	    metric1=0;
	    
	    // Check n1_pucch0 metric
	    if (n1_pucch0 != -1) {
	      if (abstraction_flag == 0) 
		metric0 = rx_pucch(phy_vars_eNB,
				   format,
				   i,
				   (u16)n1_pucch0,
				   0, // n2_pucch
				   1, // shortened format
				   pucch_payload0,
				   last_slot>>1,
				   PUCCH1_THRES);
	      else {
#ifdef PHY_ABSTRACTION
		metric0 = rx_pucch_emul(phy_vars_eNB,i,
					format,
					0,
					pucch_payload0,
					last_slot>>1);
#endif
	      }
	    }

	    // Check n1_pucch1 metric
	    if (n1_pucch1 != -1) {
	      if (abstraction_flag == 0)
		metric1 = rx_pucch(phy_vars_eNB,
				   format,
				   i,
				   (u16)n1_pucch1,
				   0, //n2_pucch
				   1, // shortened format
				   pucch_payload1,
				   last_slot>>1,
				   PUCCH1_THRES);
	      else {
#ifdef PHY_ABSTRACTION
		metric1 = rx_pucch_emul(phy_vars_eNB,i,
					format,
					1,
					pucch_payload1,
					last_slot>>1);

		
#endif
	      }
	    }
	  }

	  if (SR_payload == 1) {
	    pucch_payload = pucch_payload0;
	    if (bundling_flag == bundling)
	      pucch_sel = 2;
	  }
	  else if (bundling_flag == multiplexing) {  // multiplexing + no SR
	    pucch_payload = (metric1>metric0) ? pucch_payload1 : pucch_payload0;
	    pucch_sel     = (metric1>metric0) ? 1 : 0;
	  }
	  else { // bundling + no SR
	    if (n1_pucch1 != -1)
	      pucch_payload = pucch_payload1;
	    else if (n1_pucch0 != -1)
	      pucch_payload = pucch_payload0;
	    pucch_sel = 2;  // indicate that this is a bundled ACK/NAK  
	  }
#ifdef DEBUG_PHY_PROC	  
	  LOG_D(PHY,"[eNB %d][PDSCH %x] Frame %d subframe %d ACK/NAK metric 0 %d, metric 1 %d, sel %d, (%d,%d)\n",phy_vars_eNB->Mod_id,
		phy_vars_eNB->dlsch_eNB[i][0]->rnti,
		phy_vars_eNB->frame,last_slot>>1,
		metric0,metric1,pucch_sel,pucch_payload[0],pucch_payload[1]);
#endif	  
	  process_HARQ_feedback(i,last_slot>>1,phy_vars_eNB,
				0,// pusch_flag
				pucch_payload,
				pucch_sel,
				SR_payload);
	}
      }
    } // PUCCH processing
    
#endif //PUCCH

    if (last_slot==0) {
      phy_vars_eNB->eNB_UE_stats[i].dlsch_bitrate = (phy_vars_eNB->eNB_UE_stats[i].total_TBS - 
						     phy_vars_eNB->eNB_UE_stats[i].total_TBS_last)*100;
      
      phy_vars_eNB->eNB_UE_stats[i].total_TBS_last = phy_vars_eNB->eNB_UE_stats[i].total_TBS;
    }
    
  } // loop i=0 ... NUMBER_OF_UE_MAX-1

  if (((last_slot&1) == 1 ) &&
      (pusch_active == 0)){
    if (abstraction_flag == 0) {
      //      LOG_D(PHY,"[eNB] Frame %d, subframe %d Doing I0_measurements\n",
      //	  (((last_slot>>1)==9)?-1:0) + phy_vars_eNB->frame,last_slot>>1);
      lte_eNB_I0_measurements(phy_vars_eNB,
			      0,
			      phy_vars_eNB->first_run_I0_measurements);
    }
#ifdef PHY_ABSTRACTION
    else {
//#ifdef PHY_ABSTRACTION
      lte_eNB_I0_measurements_emul(phy_vars_eNB,
				   sect_id);
//#endif
    }
#endif
  
    
    if (I0_clear == 1)
      I0_clear = 0;
  }

#ifdef EMOS
  phy_procedures_emos_eNB_RX(last_slot);
#endif
    
}

#undef DEBUG_PHY_PROC

void phy_procedures_eNB_lte(unsigned char last_slot, unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {

  /*
  if (phy_vars_eNB->frame >= 1000)
    mac_xface->macphy_exit("Exiting after 1000 Frames\n");
  */
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_LTE,1);
  
  if (((phy_vars_eNB->lte_frame_parms.frame_type == 1)&&(subframe_select(&phy_vars_eNB->lte_frame_parms,next_slot>>1)==SF_DL))||
      (phy_vars_eNB->lte_frame_parms.frame_type == 0)){
    //    LOG_D(PHY,"[eNB %d] Frame %d: Calling phy_procedures_eNB_TX(%d)\n", phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot);
    phy_procedures_eNB_TX(next_slot,phy_vars_eNB,abstraction_flag);
  }
  if (((phy_vars_eNB->lte_frame_parms.frame_type == 1 )&&(subframe_select(&phy_vars_eNB->lte_frame_parms,last_slot>>1)==SF_UL))||
      (phy_vars_eNB->lte_frame_parms.frame_type == 0)){
    //    LOG_D(PHY,"[eNB %d] Frame %d: Calling phy_procedures_eNB_RX(%d)\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame, last_slot);
    phy_procedures_eNB_RX(last_slot,phy_vars_eNB,abstraction_flag);
  }
  if ((subframe_select(&phy_vars_eNB->lte_frame_parms,next_slot>>1)==SF_S) &&
      ((next_slot&1)==0)) {
    //    LOG_D(PHY,"[eNB %d] Frame %d: Calling phy_procedures_eNB_S_TX(%d)\n",phy_vars_eNB->Mod_id,phy_vars_eNB->frame, next_slot);
    phy_procedures_eNB_TX(next_slot,phy_vars_eNB,abstraction_flag);
  }
  if ((subframe_select(&phy_vars_eNB->lte_frame_parms,last_slot>>1)==SF_S) &&
      ((last_slot&1)==0)){
    //    LOG_D(PHY,"[eNB %d] Frame %d: Calling phy_procedures_eNB_S_RX(%d)\n", phy_vars_eNB->Mod_id,phy_vars_eNB->frame, last_slot);
    phy_procedures_eNB_S_RX(last_slot,phy_vars_eNB,abstraction_flag);
  }
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_LTE,0);

  if (next_slot == 19)
    phy_vars_eNB->frame++;

}

