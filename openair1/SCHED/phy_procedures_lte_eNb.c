/*________________________phy_procedures_lte_eNB.c________________

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

#define DEBUG_PHY_PROC

#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"

//#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
//#endif

//#define DIAG_PHY

#define NS_PER_SLOT 500000

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


s32 add_ue(s16 rnti, PHY_VARS_eNB *phy_vars_eNB) {
  u8 i;

#ifdef DEBUG_PHY_PROC
  msg("[PHY][eNB %d] Adding UE with rnti %x\n",phy_vars_eNB->Mod_id,rnti);
#endif
  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if ((phy_vars_eNB->dlsch_eNB[i]==NULL) || (phy_vars_eNB->ulsch_eNB[i]==NULL)) 
      msg("[PHY] Can't add UE, not enough memory allocated\n");
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
  msg("[PHY] eNB %d removing UE with rnti %x\n",phy_vars_eNB->Mod_id,rnti);
#endif
  for (i=0;i<NUMBER_OF_UE_MAX;i++) {
    if ((phy_vars_eNB->dlsch_eNB[i]==NULL) || (phy_vars_eNB->ulsch_eNB[i]==NULL)) 
      msg("[PHY] Can't remove UE, not enough memory allocated\n");
    else {
      if (phy_vars_eNB->eNB_UE_stats[i].crnti==rnti) {
	//msg("[PHY] UE_id %d\n",i);
	clean_eNb_dlsch(phy_vars_eNB->dlsch_eNB[i][0], abstraction_flag);
	clean_eNb_ulsch(phy_vars_eNB->ulsch_eNB[i],abstraction_flag);
	phy_vars_eNB->eNB_UE_stats[i].crnti = 0;
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
  u8 subframe_m4,subframe_p4; 
  u8 i;
  s8 UE_id = find_ue(rnti,PHY_vars_eNB_g[Mod_id]);

  if (UE_id==-1) {
    msg("[PHY] Cannot find UE with rnti %x\n",rnti);
    *round=0;
    return(-1);
  }

  if (ul_flag == 0)  {// this is a DL request
    DLSCH_ptr = PHY_vars_eNB_g[Mod_id]->dlsch_eNB[UE_id][0];
    
    if (subframe<4)
      subframe_m4 = subframe+6;
    else
      subframe_m4 = subframe-4;
#ifdef DEBUG_PHY_PROC
    //msg("get_ue_active_harq_pid: subframe_m4 %d\n",subframe_m4);
#endif
    // switch on TDD or FDD configuration here later
    *harq_pid = DLSCH_ptr->harq_ids[subframe];
    if ((*harq_pid<DLSCH_ptr->Mdlharq) && 
	((DLSCH_ptr->harq_processes[*harq_pid]->round > 0)))
      *round = DLSCH_ptr->harq_processes[*harq_pid]->round;
    else if ((subframe_m4==5) || (subframe_m4==6)) {
      *harq_pid = DLSCH_ptr->harq_ids[subframe_m4];
      *round    = DLSCH_ptr->harq_processes[*harq_pid]->round;
    }
    else {
      // get first free harq_pid (i.e. round 0)
      for (i=0;i<DLSCH_ptr->Mdlharq;i++) {
	if (DLSCH_ptr->harq_processes[i]!=NULL) {
	  if (DLSCH_ptr->harq_processes[i]->status != ACTIVE) {
	    *harq_pid = i;
	    *round = 0;
	    return(0);
	  }
	}
	else {
	  msg("[PHY] eNB %d DLSCH process %d for rnti %x (UE_id %d) not allocated\n",Mod_id,i,rnti,UE_id);
	  return(-1);
	}
      }
    }
  }
  else {  // This is a UL request

    ULSCH_ptr = PHY_vars_eNB_g[Mod_id]->ulsch_eNB[UE_id];
    subframe_p4 = subframe+4;
    if (subframe_p4>9)
      subframe_p4-=10;

    // Note this is for TDD configuration 3,4,5 only
    *harq_pid = subframe_p4-2;
    *round    = ULSCH_ptr->harq_processes[*harq_pid]->round;
  }
  return(0);
}


#ifdef EMOS
void phy_procedures_emos_eNB_TX(unsigned char next_slot) {

  unsigned char sect_id,i;

  if (next_slot==1) {
    emos_dump_eNB.timestamp = rt_get_time_ns();
    emos_dump_eNB.frame_tx = mac_xface->frame;
  }
  if (next_slot%2==0) {
    for (i=0; i<2; i++) 
      memcpy(&emos_dump_eNB.DCI_alloc[i][next_slot>>1], &CH_mac_inst[0].DCI_pdu.dci_alloc[i], sizeof(DCI_ALLOC_t));
  }
  if (next_slot==19) {
    debug_msg("[PHY][eNB%d] Frame %d, slot %d, Writing EMOS data to FIFO\n",
	      phy_vars_eNB->Mod_id,mac_xface->frame, next_slot);
    if (rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_eNB, sizeof(fifo_dump_emos_eNB))!=sizeof(fifo_dump_emos_eNB)) {
      debug_msg("[PHY][eNB%d] Frame %d, slot %d, Problem writing EMOS data to FIFO\n",
		phy_vars_eNB->Mod_id,mac_xface->frame, next_slot);
      return;
    }
  }
}
#endif

void phy_procedures_eNB_S_TX(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {

  int sect_id = 0, aa;

  if (next_slot%2==0) {
#ifdef DEBUG_PHY_PROC
    msg("[PHY][eNB%d] Frame %d, slot %d: Generating pilots for DL-S\n",
	phy_vars_eNB->Mod_id,mac_xface->frame,next_slot);
#endif
    
    for (sect_id=0;sect_id<number_of_cards;sect_id++) {
      if (abstraction_flag == 0) {

	for (aa=0; aa<phy_vars_eNB->lte_frame_parms.nb_antennas_tx; aa++) {
	  
	  
	  /*
	    #ifdef DEBUG_PHY
	    printf("Clearing TX buffer %d at %p, length %d \n",aa,
	    &phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id][aa][next_slot*(phy_vars_eNB->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNB->lte_frame_parms.symbols_per_tti>>1)],
	    (phy_vars_eNB->lte_frame_parms.N_RB_DL*12)*(phy_vars_eNB->lte_frame_parms.symbols_per_tti>>1)*sizeof(mod_sym_t));
	    #endif
	  */
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
	
	generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		     AMP,
		     &phy_vars_eNB->lte_frame_parms,
		     2,
		     next_slot);
      
      }
      else {
	generate_pss_emul(phy_vars_eNB,sect_id);
      }
    }
  }
}
 
void phy_procedures_eNB_S_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {

  int aa,l,sync_pos,sync_pos_slot;
  unsigned int sync_val;
  unsigned char sect_id=0; 
  char UE_id=0;
  int time_in, time_out;
  short *x, *y;
  u8 eNB_id = phy_vars_eNB->Mod_id;
  //  char fname[100],vname[100];


  if (last_slot%2==1) {
    
    /*
      for (sect_id = 0; sect_id < number_of_cards; sect_id++) {
      for (l=0;l<phy_vars_eNB->lte_frame_parms.symbols_per_tti/2;l++) {
	
      slot_fep_ul(&phy_vars_eNB->lte_frame_parms,
      phy_vars_eNB->lte_eNB_common_vars,
      l,
      last_slot,
      sect_id,
      #ifdef USER_MODE
      0
      #else
      1
      #endif
      );
      }
      }
    */

#ifndef USER_MODE
    time_in = openair_get_mbox();
#endif


    // we alternately process the signals from the three different sectors
#ifdef USER_MODE
    sect_id = 0;
#else
    sect_id = mac_xface->frame % number_of_cards; 
#endif
    //sect_id = 2;

    if (sect_id == 0) {
      max_peak_val = 0;
      max_sect_id = 0;
      max_sync_pos = 0;
    }


    if (abstraction_flag == 0) {
      // look for PSS in the last 3 symbols of the last slot
      // but before we need to zero pad the gaps that the HW removed
      // also add the signals from all antennas of all eNBs

      bzero(eNB_sync_buffer[0],640*6*sizeof(int));
      bzero(eNB_sync_buffer[1],640*6*sizeof(int));
      
      
      for (aa=0; aa<phy_vars_eNB->lte_frame_parms.nb_antennas_rx; aa++) {
	for (l=PRACH_SYMBOL; l<phy_vars_eNB->lte_frame_parms.symbols_per_tti/2; l++) {
	  
	  x = (short*) &eNB_sync_buffer[aa][(l-PRACH_SYMBOL)*(phy_vars_eNB->lte_frame_parms.ofdm_symbol_size+phy_vars_eNB->lte_frame_parms.nb_prefix_samples)+phy_vars_eNB->lte_frame_parms.nb_prefix_samples];
#ifdef USER_MODE
	  y = (short*) &phy_vars_eNB->lte_eNB_common_vars.rxdata[sect_id][aa][(last_slot*phy_vars_eNB->lte_frame_parms.symbols_per_tti/2+l)*(phy_vars_eNB->lte_frame_parms.ofdm_symbol_size+phy_vars_eNB->lte_frame_parms.nb_prefix_samples)+phy_vars_eNB->lte_frame_parms.nb_prefix_samples];
#else
	  y = (short*) &phy_vars_eNB->lte_eNB_common_vars.rxdata[sect_id][aa][(last_slot*phy_vars_eNB->lte_frame_parms.symbols_per_tti/2+l)*phy_vars_eNB->lte_frame_parms.ofdm_symbol_size];
#endif
	  //z = x;
	  //add_vector16(x,y,z,phy_vars_eNB->lte_frame_parms.ofdm_symbol_size*2);
	  memcpy(x,y,phy_vars_eNB->lte_frame_parms.ofdm_symbol_size*sizeof(int));
	}
      }

      /*      
	      #ifdef USER_MODE
	      #ifdef DEBUG_PHY
	      write_output("eNB_sync_rx0.m","eNB_sync_rx0",&phy_vars_eNB->lte_eNB_common_vars.rxdata[sect_id][0][(last_slot*phy_vars_eNB->lte_frame_parms.symbols_per_tti/2+PRACH_SYMBOL)*(phy_vars_eNB->lte_frame_parms.ofdm_symbol_size+phy_vars_eNB->lte_frame_parms.nb_prefix_samples)*6+phy_vars_eNB->lte_frame_parms.nb_prefix_samples],(phy_vars_eNB->lte_frame_parms.ofdm_symbol_size+phy_vars_eNB->lte_frame_parms.nb_prefix_samples),1,1);
	      write_output("eNB_sync_buffer0.m","eNB_sync_buf0",eNB_sync_buffer[0],(phy_vars_eNB->lte_frame_parms.ofdm_symbol_size+phy_vars_eNB->lte_frame_parms.nb_prefix_samples)*(phy_vars_eNB->lte_frame_parms.symbols_per_tti/2-PRACH_SYMBOL),1,1);
	      write_output("eNB_sync_buffer1.m","eNB_sync_buf1",eNB_sync_buffer[1],(phy_vars_eNB->lte_frame_parms.ofdm_symbol_size+phy_vars_eNB->lte_frame_parms.nb_prefix_samples)*(phy_vars_eNB->lte_frame_parms.symbols_per_tti/2-PRACH_SYMBOL),1,1);
	      #endif      
	      #endif
      */
    
      sync_pos_slot = 0; //this is where the sync pos should be wrt eNB_sync_buffer
      
      sync_pos = lte_sync_time_eNB(eNB_sync_buffer, 
				   &phy_vars_eNB->lte_frame_parms, 
				   (phy_vars_eNB->lte_frame_parms.symbols_per_tti/2 - PRACH_SYMBOL) * 
				   (phy_vars_eNB->lte_frame_parms.ofdm_symbol_size+phy_vars_eNB->lte_frame_parms.nb_prefix_samples),
				   &sync_val,
				   phy_vars_eNB->lte_eNB_common_vars.sync_corr[sect_id]);
      
#ifdef USER_MODE
#ifdef DEBUG_PHY_PROC    
      if (sect_id==0)
      	write_output("sync_corr_eNB.m","synccorr",phy_vars_eNB->lte_eNB_common_vars.sync_corr[sect_id],
      		     (phy_vars_eNB->lte_frame_parms.symbols_per_tti/2 - PRACH_SYMBOL) * 
      		     (phy_vars_eNB->lte_frame_parms.ofdm_symbol_size+phy_vars_eNB->lte_frame_parms.nb_prefix_samples),1,2);
#endif    
#endif
    }
    else {
      sync_pos = lte_sync_time_eNB_emul(phy_vars_eNB,
					0,//sect_id,
					&sync_val);

    }

    if ((sync_pos>=0) && (sync_val > max_peak_val)) {
      max_peak_val = sync_val;
      max_sect_id = sect_id;
      max_sync_pos = sync_pos;
    }
#ifdef DEBUG_PHY_PROC    
    msg("[PHY][eNB%d] Frame %d, found sect_id=%d, sync_pos=%d, sync_val=%u, max_peak_val=%u, max_peak_pos=%d\n",
	phy_vars_eNB->Mod_id,mac_xface->frame,sect_id,sync_pos,sync_val,max_peak_val,max_sync_pos);
#endif
    
#ifndef USER_MODE
    time_out = openair_get_mbox();
#endif
    
    if (max_peak_val>0) {
      if (sect_id==number_of_cards-1) {
	//#ifdef DEBUG_PHY
	msg("[PHY_PROCEDURES_eNB] frame %d, slot %d: Found user %x in sector %d at pos %d val %d (time_in %d, time_out %d)\n",
	    mac_xface->frame, last_slot, 
	    UE_id, max_sect_id,
	    max_sync_pos, 
	    max_peak_val,
	    time_in, time_out);
	//#endif

	// this should be handled by the MAC
	UE_id = find_next_ue_index(phy_vars_eNB);
	if (UE_id>=0) {
	  phy_vars_eNB->eNB_UE_stats[UE_id].UE_timing_offset = cmax(max_sync_pos - sync_pos_slot - phy_vars_eNB->lte_frame_parms.nb_prefix_samples/8,0);
	  //phy_vars_eNb->eNB_UE_stats[UE_id].mode = PRACH;
	  phy_vars_eNB->eNB_UE_stats[UE_id].sector = max_sect_id;
	  
#ifdef OPENAIR2
#ifdef DEBUG_PHY_PROC
	msg("[PHY][eNB%d] frame %d, slot %d: Calling initiate_ra_proc (%p(%d,%d,%d))\n",
	    phy_vars_eNB->Mod_id,mac_xface->frame, last_slot, mac_xface->initiate_ra_proc,
	    0,
	    cmax(max_sync_pos - sync_pos_slot - phy_vars_eNB->lte_frame_parms.nb_prefix_samples/8,0),
	    max_sect_id);
#endif

	mac_xface->initiate_ra_proc(phy_vars_eNB->Mod_id,
				    0,
				    cmax(max_sync_pos - sync_pos_slot - phy_vars_eNB->lte_frame_parms.nb_prefix_samples/8,0),
				    max_sect_id);
#endif
	}
	else {
	  msg("[PHY_PROCEDURES_eNB] frame %d, slot %d: Unable to add user, max user count reached\n", mac_xface->frame, last_slot);
	}	  
	
	max_peak_val = 0;
	max_sect_id = 0;
	max_sync_pos = 0;
	
      }
    }
    else {
#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB%d] Frame %d, slot %d: No user found\n",
	  phy_vars_eNB->Mod_id,mac_xface->frame,last_slot);
#endif
      max_peak_val = 0;
      max_sect_id = 0;
      max_sync_pos = 0;
      
    }
    
    
    // Get noise levels
    
    
    for (sect_id=0;sect_id<number_of_cards;sect_id++) {
      /*
	sprintf(fname,"rxsigF0_%d.m",sect_id);
	sprintf(vname,"rxsF0_%d",sect_id);
	write_output(fname,vname, &phy_vars_eNB->lte_eNB_common_vars.rxdataF[sect_id][0][(19*phy_vars_eNB->lte_frame_parms.ofdm_symbol_size)<<1],512*2,2,1);
	sprintf(fname,"rxsigF1_%d.m",sect_id);
	sprintf(vname,"rxsF1_%d",sect_id);
	write_output(fname,vname, &phy_vars_eNB->lte_eNB_common_vars.rxdataF[sect_id][1][(19*phy_vars_eNB->lte_frame_parms.ofdm_symbol_size)<<1],512*2,2,1);
      */
      if (abstraction_flag == 0) {
	lte_eNB_I0_measurements(phy_vars_eNB,
				sect_id,
				phy_vars_eNB->first_run_I0_measurements);
      }
      else {
	lte_eNB_I0_measurements_emul(phy_vars_eNB,
				     sect_id);
      }
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

  //debug_msg("fill_dci: rballoc = %x\n",rballoc);

  switch (subframe) {

  case 5:
    /*
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
    */
    break;
  case 6:
    /*
    DCI_pdu->Num_ue_spec_dci = 1;
    DCI_pdu->dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
    DCI_pdu->dci_alloc[0].L          = 2;
    DCI_pdu->dci_alloc[0].rnti       = 0x1235;
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
  case 7:
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

}
#endif

void phy_procedures_eNB_TX(unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {

  u8 *pbch_pdu=&phy_vars_eNB->pbch_pdu[0];
  //  unsigned int nb_dci_ue_spec = 0, nb_dci_common = 0;
  u16 input_buffer_length, re_allocated;
  u32 sect_id = 0,i,aa;
  u8 harq_pid, num_dci;
  DCI_PDU *DCI_pdu;
  u8 *DLSCH_pdu;
#ifndef OPENAIR2
  DCI_PDU DCI_pdu_tmp;
  u8 DLSCH_pdu_tmp[768*8];
#endif
  s8 UE_id;
  u8 num_pdcch_symbols;
  s16 crnti;
  u16 frame_tx;

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
     
    }
   
    if (next_slot == 0) {

      // First half of PSS/SSS (FDD)
      if (phy_vars_eNB->lte_frame_parms.frame_type == 0) {
	generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		     AMP,
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

    if (next_slot == 1) {
      
      if ((mac_xface->frame&3) == 0) {
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
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (0<<3);
	  break;
	case half:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (1<<3);
	  break;
	case one:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (2<<3);
	  break;
	case two:
	  ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (3<<3);
	  break;
	default:
	  break;
	}

	((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xfc) | ((mac_xface->frame>>8)&0x3);
	((u8*) pbch_pdu)[1] = mac_xface->frame&0xfc;
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
      
   
      frame_tx = (((int) (pbch_pdu[0]&0x3))<<8) + ((int) (pbch_pdu[1]&0xfc)) + mac_xface->frame%4;
   
#ifdef DEBUG_PHY_PROC
      debug_msg("[PHY][eNB%d] Frame %d, slot %d: Calling generate_pbch, mode1_flag=%d, frame_tx=%d, pdu=%02x%02x%02x\n",
		phy_vars_eNB->Mod_id,
		mac_xface->frame, 
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
		      mac_xface->frame&3);
	

      }
      else {
	generate_pbch_emul(phy_vars_eNB,pbch_pdu); 
      }
    }
  
    // Second half of PSS/SSS (FDD)
    if (next_slot == 10) {
     
      if (abstraction_flag==0) {
       
	if (phy_vars_eNB->lte_frame_parms.frame_type == 0) {
	  generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       AMP,
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
    // Second half of PSS (TDD), if not S-Subframe
    if (next_slot == 12) {
     
      if (abstraction_flag==0) {
       
	if (phy_vars_eNB->lte_frame_parms.frame_type == 1) {
	  generate_pss(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
		       AMP,
		       &phy_vars_eNB->lte_frame_parms,
		       2,
		       next_slot);

	}
      }
    }
  }

  sect_id=0;

  if ((next_slot % 2)==0) {
#ifdef DEBUG_PHY_PROC
    msg("[PHY][eNB %d] UE %d: Mode %s\n",phy_vars_eNB->Mod_id,0,mode_string[phy_vars_eNB->eNB_UE_stats[0].mode]);
#endif

#ifdef OPENAIR2
      // if there are two users and we want to do cooperation
    if ((phy_vars_eNB->eNB_UE_stats[0].mode == PUSCH) && (phy_vars_eNB->eNB_UE_stats[1].mode == PUSCH))
      mac_xface->eNB_dlsch_ulsch_scheduler(phy_vars_eNB->Mod_id,phy_vars_eNB->cooperation_flag,next_slot>>1);
    else
      mac_xface->eNB_dlsch_ulsch_scheduler(phy_vars_eNB->Mod_id,0,next_slot>>1);

    // Parse DCI received from MAC
    DCI_pdu = mac_xface->get_dci_sdu(phy_vars_eNB->Mod_id,next_slot>>1);
#else
    DCI_pdu = &DCI_pdu_tmp;
    fill_dci(DCI_pdu,next_slot>>1,phy_vars_eNB->cooperation_flag);
#endif


#ifdef EMOS
    emos_dump_eNB.dci_cnt[next_slot>>1] = DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci; //nb_dci_common+nb_dci_ue_spec;
#endif
    // clear previous allocation information for all UEs
    for (i=0;i<NUMBER_OF_UE_MAX;i++) {
      phy_vars_eNB->dlsch_eNB[i][0]->subframe_tx[next_slot>>1] = 0;
    }


    for (i=0;i<DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci ; i++) {
#ifdef DEBUG_PHY_PROC
      debug_msg("[PHY][eNB] Subframe %d : Doing DCI index %d/%d\n",next_slot>>1,i,DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci);
      if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 50))
	dump_dci(&phy_vars_eNB->lte_frame_parms,&DCI_pdu->dci_alloc[i]);
#endif

      if (DCI_pdu->dci_alloc[i].rnti == SI_RNTI) {
	debug_msg("[PHY][eNB %d] SI generate_eNB_dlsch_params_from_dci\n", phy_vars_eNB->Mod_id);
	generate_eNB_dlsch_params_from_dci(next_slot>>1,
					   &DCI_pdu->dci_alloc[i].dci_pdu[0],
					   DCI_pdu->dci_alloc[i].rnti,
					   DCI_pdu->dci_alloc[i].format,
					   &phy_vars_eNB->dlsch_eNB_SI,
					   &phy_vars_eNB->lte_frame_parms,
					   SI_RNTI,
					   RA_RNTI,
					   P_RNTI,
					   phy_vars_eNB->eNB_UE_stats[0].DL_pmi_single);
      }
      else if (DCI_pdu->dci_alloc[i].rnti == RA_RNTI) {
#ifdef DEBUG_PHY_PROC
	msg("[PHY] enb %d RA generate_eNB_dlsch_params_from_dci\n", phy_vars_eNB->Mod_id);
#endif
	generate_eNB_dlsch_params_from_dci(next_slot>>1,
					   &DCI_pdu->dci_alloc[i].dci_pdu[0],
					   DCI_pdu->dci_alloc[i].rnti,
					   DCI_pdu->dci_alloc[i].format,
					   &phy_vars_eNB->dlsch_eNB_ra,
					   &phy_vars_eNB->lte_frame_parms,
					   SI_RNTI,
					   RA_RNTI,
					   P_RNTI,
					   phy_vars_eNB->eNB_UE_stats[0].DL_pmi_single);
      }
      else if (DCI_pdu->dci_alloc[i].format == format0) {  // this is a ULSCH allocation

	harq_pid = subframe2harq_pid(&phy_vars_eNB->lte_frame_parms,((next_slot>>1)+4)%10);
	if (harq_pid==255) {
	  msg("[PHY][eNB] Frame %d: Bad harq_pid for ULSCH allocation\n",mac_xface->frame);
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
	  msg("[PHY][eNB] Frame %d: Unknown UE_id for rnti %x\n",mac_xface->frame,(s16)DCI_pdu->dci_alloc[i].rnti);
#ifdef USER_MODE
	  exit(-1);
#else
	  exit_openair=1;
#endif
	}
#ifdef DEBUG_PHY_PROC
	msg("[PHY][eNB%d] Frame %d, slot %d (%d): Generated ULSCH %d (rnti %x, dci %x) DCI, format 0 (DCI pos %d/%d), aggregation %d\n",
	    phy_vars_eNB->Mod_id,
	    mac_xface->frame,
	    next_slot,next_slot>>1,UE_id,DCI_pdu->dci_alloc[i].rnti,
	    *(unsigned int *)&DCI_pdu->dci_alloc[i].dci_pdu[0],
	    i,DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci,
	    DCI_pdu->dci_alloc[i].L);
#endif
	
	generate_eNB_ulsch_params_from_dci(&DCI_pdu->dci_alloc[i].dci_pdu[0],
					   DCI_pdu->dci_alloc[i].rnti,
					   (next_slot>>1),
					   phy_vars_eNB->transmission_mode[UE_id],
					   format0,
					   phy_vars_eNB->ulsch_eNB[UE_id],
					   &phy_vars_eNB->lte_frame_parms,
					   SI_RNTI,
					   RA_RNTI,
					   P_RNTI,
					   0);  // do_srs
	
#ifdef DEBUG_PHY_PROC
	msg("[PHY][eNB%d] frame %d, subframe %d Setting scheduling flag for ULSCH %d harq_pid %d\n",
	    phy_vars_eNB->Mod_id,
	    mac_xface->frame,next_slot>>1,UE_id,harq_pid);
#endif
	phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;
	
      }
      
      else { // this is a DLSCH allocation

#ifdef OPENAIR2
#ifdef DEBUG_PHY_PROC
	debug_msg("[PHY][eNB] Searching for RNTI %x\n",DCI_pdu->dci_alloc[i].rnti);
#endif
	UE_id = find_ue((s16)DCI_pdu->dci_alloc[i].rnti,phy_vars_eNB);
#else
	UE_id = i;
#endif
	if (UE_id>=0) {
	  generate_eNB_dlsch_params_from_dci(next_slot>>1,
					     &DCI_pdu->dci_alloc[i].dci_pdu[0],
					     DCI_pdu->dci_alloc[i].rnti,
					     DCI_pdu->dci_alloc[i].format,
					     phy_vars_eNB->dlsch_eNB[(u8)UE_id],
					     &phy_vars_eNB->lte_frame_parms,
					     SI_RNTI,
					     RA_RNTI,
					     P_RNTI,
					     phy_vars_eNB->eNB_UE_stats[(u8)UE_id].DL_pmi_single);
#ifdef DEBUG_PHY_PROC      
	  msg("[PHY][eNB%d] Frame %d, slot %d: Generated DLSCH DCI (rnti %x => %x,%x), format %d, aggregation %d\n",
	      phy_vars_eNB->Mod_id,
	      mac_xface->frame, next_slot,
	      DCI_pdu->dci_alloc[i].rnti,
	      *(unsigned int*)DCI_pdu->dci_alloc[i].dci_pdu,
	      *(unsigned int*)(1+DCI_pdu->dci_alloc[i].dci_pdu),
	      DCI_pdu->dci_alloc[i].format,
	      DCI_pdu->dci_alloc[i].L);
#endif
	}
	else {
	  msg("[PHY][eNB] Frame %d : No UE_id with corresponding rnti %x, dropping DLSCH\n",mac_xface->frame,(s16)DCI_pdu->dci_alloc[i].rnti);
	}
      }

    

    } // loop over DCI

    // if we have PHICH to generate
    if (is_phich_subframe(&phy_vars_eNB->lte_frame_parms,next_slot>>1)) {
      //      debug_msg("[PHY_PROCEDURES_eNB] Frame %d, slot %d: Calling generate_phich_top\n",mac_xface->frame, next_slot);
      if (abstraction_flag==0) {
	//      generate_phich_top(&phy_vars_eNB->lte_frame_parms,next_slot>>1,phy_vars_eNB->ulsch_eNB[0],phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id]);
      }
      else {
	generate_phich_emul(phy_vars_eNB,next_slot>>1,phy_vars_eNB->ulsch_eNB[0]);
      }
    }

    // if we have DCI to generate do it now
    if ((DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci)>0) {

#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB%d] Frame %d, slot %d: Calling generate_dci_top\n",
	  phy_vars_eNB->Mod_id,mac_xface->frame, next_slot);
#endif

      if (abstraction_flag == 0) {
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
      else {
	num_pdcch_symbols = generate_dci_top_emul(phy_vars_eNB,DCI_pdu->Num_ue_spec_dci,DCI_pdu->Num_common_dci,DCI_pdu->dci_alloc,next_slot>>1);
      }
#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB%d] Frame %d, slot %d: num_pdcch_symbols=%d\n",phy_vars_eNB->Mod_id,mac_xface->frame, next_slot,num_pdcch_symbols);
#endif

    }
    else {  // for emulation!!
      phy_vars_eNB->num_ue_spec_dci[(next_slot>>1)&1]=0;
      phy_vars_eNB->num_common_dci[(next_slot>>1)&1]=0;
    }
    // For even next slots generate dlsch
    for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
      if ((phy_vars_eNB->dlsch_eNB[(u8)UE_id][0])&&
	  (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti>0)&&
	  (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->active == 1)) {
	harq_pid = phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->current_harq_pid;
	input_buffer_length = phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->TBS/8;
      

#ifdef DEBUG_PHY_PROC
	debug_msg("[PHY][eNB%d] Frame %d, slot %d: Calling generate_dlsch for rnti %x (harq_pid %d) with input size = %d, G %d\n",
		  phy_vars_eNB->Mod_id,
		  mac_xface->frame, next_slot, phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti, harq_pid,input_buffer_length,
		  get_G(&phy_vars_eNB->lte_frame_parms,
			phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->nb_rb,
			phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rb_alloc,
			get_Qm(phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->mcs),
			num_pdcch_symbols,next_slot>>1));
#endif

	phy_vars_eNB->eNB_UE_stats[(u8)UE_id].dlsch_sliding_cnt++;
	if (phy_vars_eNB->dlsch_eNB[UE_id][0]->harq_processes[harq_pid]->Ndi == 1) {

	  phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0]++;
	  
#ifdef OPENAIR2
	  DLSCH_pdu = mac_xface->get_dlsch_sdu(phy_vars_eNB->Mod_id,
					       phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti,
					       0);
#else
	  DLSCH_pdu = DLSCH_pdu_tmp;
	  for (i=0;i<input_buffer_length;i++)
	    DLSCH_pdu[i] = (unsigned char)(taus()&0xff);
#endif      

#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_DLSCH
	  msg("[PHY] eNB DLSCH SDU: \n");
	  for (i=0;i<phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->harq_processes[harq_pid]->TBS>>3;i++)
	    msg("%x.",(u8)DLSCH_pdu[i]);
	  msg("\n");
#endif
#endif
	}
	else {
	  phy_vars_eNB->eNB_UE_stats[UE_id].dlsch_trials[0]++;	
#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_DLSCH  
	  msg("[PHY][eNB] This DLSCH is a retransmission\n");
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
	  for (sect_id=0;sect_id<number_of_cards;sect_id++)
	    
	    re_allocated = dlsch_modulation(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
					    ((phy_vars_eNB->transmission_mode[(u8)UE_id] == 5) &&
					     (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->dl_power_off == 0)) ? AMP/2 : AMP,
					    next_slot/2,
					    &phy_vars_eNB->lte_frame_parms,
					    num_pdcch_symbols,
					    phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]);
	  /*
	    if (mimo_mode == DUALSTREAM) {
	    dlsch_encoding(input_buffer,
	    phy_vars_eNB->lte_frame_parms,
	    dlsch_eNB[1]);
	    
	    re_allocated += dlsch_modulation(phy_vars_eNB->lte_eNB_common_vars.txdataF[sect_id],
	    1024,
	    next_slot>>1,
	    phy_vars_eNB->lte_frame_parms,
	    dlsch_eNB[1]);
	    }
	  */
	}
	else {
	  dlsch_encoding_emul(phy_vars_eNB,
			      DLSCH_pdu,
			      phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]);
	}
	phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->active = 0;
	
#ifdef DEBUG_PHY_PROC   
	msg("[PHY][eNB%d] Frame %d, slot %d, DLSCH re_allocated = %d mod id %d\n",
	    phy_vars_eNB->Mod_id,mac_xface->frame, next_slot, re_allocated, phy_vars_eNB->Mod_id);
#endif
	//mac_xface->macphy_exit("first dlsch transmitted\n");
      }

      else if ((phy_vars_eNB->dlsch_eNB[(u8)UE_id][0])&&
	       (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->rnti>0)&&
	       (phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->active == 0)) {
      
	// clear subframe TX flag since UE is not scheduled for PDSCH in this subframe (so that we don't look for PUCCH later)
	phy_vars_eNB->dlsch_eNB[(u8)UE_id][0]->subframe_tx[next_slot>>1]=0;
#ifdef DEBUG_PHY_PROC
	msg("[PHY][eNB%d] DCI: Clearing subframe_tx for subframe %d, UE %d\n",phy_vars_eNB->Mod_id,next_slot>>1,UE_id);
#endif
      }
    } 
  
    if (phy_vars_eNB->dlsch_eNB_SI->active == 1) {
      input_buffer_length = phy_vars_eNB->dlsch_eNB_SI->harq_processes[0]->TBS/8;


#ifdef OPENAIR2
      DLSCH_pdu = mac_xface->get_dlsch_sdu(phy_vars_eNB->Mod_id,
					   SI_RNTI,
					   0);
#else
      DLSCH_pdu = DLSCH_pdu_tmp;
      for (i=0;i<input_buffer_length;i++)
	DLSCH_pdu[i] = (unsigned char)(taus()&0xff);
#endif      


      
#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_DLSCH
      msg("[PHY][eNB%d] Frame %d, slot %d: Calling generate_dlsch (SI) with input size = %d, num_pdcch_symbols %d\n",
	  phy_vars_eNB->Mod_id,mac_xface->frame, next_slot, input_buffer_length,num_pdcch_symbols);
      for (i=0;i<input_buffer_length;i++)
	msg("dlsch_input_buffer[%d]=%x\n",i,DLSCH_pdu[i]);
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
      else {
	dlsch_encoding_emul(phy_vars_eNB,
			    DLSCH_pdu,
			    phy_vars_eNB->dlsch_eNB_SI);
      }
      phy_vars_eNB->dlsch_eNB_SI->active = 0;
      
#ifdef DEBUG_PHY_PROC    
      msg("[PHY][eNB%d] Frame %d, slot %d, DLSCH (SI) re_allocated = %d\n",
	  phy_vars_eNB->Mod_id,mac_xface->frame, next_slot, re_allocated);
#endif
      
    }
  
    if (phy_vars_eNB->dlsch_eNB_ra->active == 1) {
#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB%d] Frame %d, slot %d, RA active, filling RAR:\n",
	  phy_vars_eNB->Mod_id,mac_xface->frame, next_slot);
#endif

      input_buffer_length = phy_vars_eNB->dlsch_eNB_ra->harq_processes[0]->TBS/8;

#ifdef OPENAIR2
      crnti = mac_xface->fill_rar(phy_vars_eNB->Mod_id,
				  dlsch_input_buffer,
				  phy_vars_eNB->lte_frame_parms.N_RB_UL,
				  input_buffer_length);
      for (i=0;i<input_buffer_length;i++)
	msg("%x.",dlsch_input_buffer[i]);
      msg("\n");
      UE_id = add_ue(crnti,phy_vars_eNB);
      if (UE_id==-1) {
	mac_xface->macphy_exit("[PHY][eNB] Max user count reached.\n");
      }

      phy_vars_eNB->eNB_UE_stats[UE_id].mode = RA_RESPONSE;
      
      generate_eNB_ulsch_params_from_rar(dlsch_input_buffer,
					 (next_slot>>1),
					 phy_vars_eNB->ulsch_eNB[UE_id],
					 &phy_vars_eNB->lte_frame_parms);

      phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_active = 1;

      get_RRCConnReq_alloc(&phy_vars_eNB->lte_frame_parms,
			   next_slot>>1,
			   mac_xface->frame,
			   &phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_frame,
			   &phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_subframe);
#else
      for (i=0;i<input_buffer_length;i++)
      	dlsch_input_buffer[i]= (unsigned char)(taus()&0xff);
#endif

#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB%d] Frame %d, next slot %d: Calling generate_dlsch (RA) with input size = %d,RRCConnRequest frame %d, RRCConnRequest subframe %d\n",
	  phy_vars_eNB->Mod_id,
	  mac_xface->frame, next_slot,input_buffer_length, 
	  phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_frame,
	  phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_subframe);
#endif

      if (abstraction_flag == 0) {

	dlsch_encoding(dlsch_input_buffer,
		       &phy_vars_eNB->lte_frame_parms,
		       num_pdcch_symbols,
		       phy_vars_eNB->dlsch_eNB_ra,
		       next_slot>>1);

	phy_vars_eNB->dlsch_eNB_ra->rnti = RA_RNTI;
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
      else {
	dlsch_encoding_emul(phy_vars_eNB,
			    dlsch_input_buffer,
			    phy_vars_eNB->dlsch_eNB_ra);
      }
      phy_vars_eNB->dlsch_eNB_ra->active = 0;
	
#ifdef DEBUG_PHY_PROC    
      msg("[PHY][eNB%d] Frame %d, slot %d, DLSCH (RA) re_allocated = %d\n",phy_vars_eNB->Mod_id,
	  mac_xface->frame, next_slot, re_allocated);
#endif
    }
  }



#ifdef EMOS
  phy_procedures_emos_eNB_TX(next_slot);
#endif
}
  
void process_RRCConnRequest(PHY_VARS_eNB *phy_vars_eNB,u8 last_slot,u8 UE_id, u8 harq_pid) {
  // this prepares the demodulation of the first PUSCH of a new user, containing RRC connection request

  msg("[PHY][eNB%d] frame %d : subframe %d (last_slot %d): process_RRCConnRequest UE_id %d (active %d, subframe %d, frame %d)\n",
      phy_vars_eNB->Mod_id,
      mac_xface->frame,last_slot>>1,last_slot,UE_id,phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_active,
      phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_subframe,
      phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_frame);
  phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_flag = 0;

  if ((phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_active == 1) && 
      ((last_slot%2)==1) && 
      (phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_subframe == (last_slot>>1)) &&
      (phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_frame == (mac_xface->frame)))   {

    //    harq_pid = 0;

    phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_active = 0;
    phy_vars_eNB->ulsch_eNB[UE_id]->RRCConnRequest_flag = 1;
    phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->subframe_scheduling_flag=1;
    msg("[PHY][eNB%d] frame %d, slot %d, subframe %d: Doing RRCConnRequest for UE %d\n",
	phy_vars_eNB->Mod_id,
	mac_xface->frame,last_slot,last_slot>>1,UE_id);
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
  LTE_eNB_DLSCH_t *dlsch             =  phy_vars_eNB->dlsch_eNB[UE_id][0];
  LTE_eNB_UE_stats *ue_stats         =  &phy_vars_eNB->eNB_UE_stats[UE_id];
  LTE_DL_eNB_HARQ_t *dlsch_harq_proc;
  u8 subframe_m4,M,m;

  if (phy_vars_eNB->lte_frame_parms.frame_type == 0){ //FDD
    subframe_m4 = (subframe<4) ? subframe+6 : subframe-4;

    dl_harq_pid[0] = dlsch->harq_ids[subframe_m4];
    M=1;
    if (pusch_flag == 1)
      dlsch_ACK[0] = phy_vars_eNB->ulsch_eNB[(u8)UE_id]->o_ACK[0];
    else
      dlsch_ACK[0] = pucch_payload[0];
    debug_msg("[MAC][eNB %d] Frame %d: Received ACK/NAK %d for subframe %d\n",phy_vars_eNB->Mod_id,
	mac_xface->frame,dlsch_ACK[0],subframe_m4);
  }
  else {  // TDD Handle M=1,2 cases only
    
    M=ul_ACK_subframe2_M(&phy_vars_eNB->lte_frame_parms,
			 subframe);
    // Now derive ACK information for TDD
    if (pusch_flag == 1) { // Do PUSCH ACK/NAK first
      // detect missing DAI
      //FK: this code is just a guess
       dlsch_ACK[0] = phy_vars_eNB->ulsch_eNB[(u8)UE_id]->o_ACK[0];
       dlsch_ACK[1] = phy_vars_eNB->ulsch_eNB[(u8)UE_id]->o_ACK[1];
    }

    else {
      if (pucch_sel == 2) {  // bundling
	dlsch_ACK[0] = pucch_payload[0];
	dlsch_ACK[1] = pucch_payload[0];
      }
      else {
	dlsch_ACK[0] = pucch_payload[0];
	dlsch_ACK[1] = pucch_payload[1];
      }
    }
  }
 
  for (m=0;m<M;m++) {

    dl_subframe = ul_ACK_subframe2_dl_subframe(&phy_vars_eNB->lte_frame_parms,
					       subframe,
					       m);

    if (dlsch->subframe_tx[dl_subframe]==1) {
      dl_harq_pid[m]     = dlsch->harq_ids[dl_subframe];
      
      if (dl_harq_pid[m]<dlsch->Mdlharq) {
	dlsch_harq_proc = dlsch->harq_processes[dl_harq_pid[m]];
#ifdef DEBUG_PHY_PROC	
	debug_msg("[PHY] eNB %d Process %d status %d, round %d\n",phy_vars_eNB->Mod_id,dl_harq_pid[m],dlsch_harq_proc->status,dlsch_harq_proc->round);
#endif
	if ((dl_harq_pid[m]<dlsch->Mdlharq) &&
	    (dlsch_harq_proc->status == ACTIVE)) {
	  // dl_harq_pid of DLSCH is still active
	  
	  //	  msg("[PHY] eNB %d Process %d is active (%d)\n",phy_vars_eNB->Mod_id,dl_harq_pid[m],dlsch_ACK[m]);
	  if ( dlsch_ACK[m]== 0) {
	    // Received NAK 
	    
	    //	    if (dlsch_harq_proc->round == 0)
	    ue_stats->dlsch_NAK[dlsch_harq_proc->round]++;
	    
	    // then Increment DLSCH round index 
	    dlsch_harq_proc->round++;
	    
	    if (dlsch_harq_proc->round == dlsch->Mdlharq) {
	      // This was the last round for DLSCH so reset round and increment l2_error counter
	      dlsch_harq_proc->round = 0;
	      ue_stats->dlsch_l2_errors++;
	      dlsch_harq_proc->status = SCH_IDLE;
	    }
	  }
	  else {
#ifdef DEBUG_PHY_PROC	
	    debug_msg("[PHY][eNB] ACK Received in round %d for harq_pid %d, resetting process\n",dlsch_harq_proc->round,dl_harq_pid[m]);
#endif
	    // Received ACK so set round to 0 and set dlsch_harq_pid IDLE
	    dlsch_harq_proc->round  = 0;
	    dlsch_harq_proc->status = SCH_IDLE; 
	  }
	  
	  // Do fine-grain rate-adaptation for DLSCH 
	  if (ue_stats->dlsch_NAK[0] > dlsch->error_threshold) {
	    if (ue_stats->dlsch_mcs_offset == 1)
	      ue_stats->dlsch_mcs_offset=0;
	    else
	      ue_stats->dlsch_mcs_offset=-1;
	  }
#ifdef DEBUG_PHY_PROC	  
	  debug_msg("[PHY][process_HARQ_feedback] Frame %d Setting round to %d for pid %d (subframe %d)\n",mac_xface->frame,
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

  if (frame_parms->frame_type ==0 ) { // FDD
    if (phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[(subframe-4)%10]>0) {
      *n1_pucch0 = frame_parms->pucch_config_common.n1PUCCH_AN + phy_vars_eNB->dlsch_eNB[UE_id][0]->nCCE[(subframe-4)%10];
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
	msg("[PHY][eNB%d] frame %d: phy_procedures_lte.c: get_n1pucch, illegal subframe %d for tdd_config %d\n",
	    phy_vars_eNB->Mod_id,
	    mac_xface->frame,
	    subframe,frame_parms->tdd_config);
	return;
      }
      break;
    case 3:  // DL:S:UL:UL:UL:DL:DL:DL:DL:DL
      if (subframe == 2) {  // ACK subframes 5,6 and 1 (S in frame-2), forget about n-11 for the moment (S-subframe)
	if (phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[6]>0) {
	  nCCE1 = phy_vars_eNB->dlsch_eNB[UE_id][0]->nCCE[6];
	  *n1_pucch1 = get_Np(frame_parms->N_RB_DL,nCCE1,1) + nCCE1 + frame_parms->pucch_config_common.n1PUCCH_AN; 
	}
	else
	  *n1_pucch1 = -1;

	if (phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[5]>0) {
	  nCCE0 = phy_vars_eNB->dlsch_eNB[UE_id][0]->nCCE[5];
	  *n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0+ frame_parms->pucch_config_common.n1PUCCH_AN; 
	}
	else
	  *n1_pucch0 = -1;
      }
      else if (subframe == 3) {   // ACK subframes 7 and 8
	//	printf("********get_n1_pucch_eNB : subframe 3, subframe_tx[7] %d, subframe_tx[8] %d\n",
	//	       phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[7],phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[8]);

	if (phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[8]>0) {
	  nCCE1 = phy_vars_eNB->dlsch_eNB[UE_id][0]->nCCE[8];
	  *n1_pucch1 = get_Np(frame_parms->N_RB_DL,nCCE1,1) + nCCE1 + frame_parms->pucch_config_common.n1PUCCH_AN;
	  //msg("nCCE1 %d, n1_pucch1 %d\n",nCCE1,n1_pucch1);
	}
	else
	  *n1_pucch1 = -1;

	if (phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[7]>0) {
	  nCCE0 = phy_vars_eNB->dlsch_eNB[UE_id][0]->nCCE[7];
	  *n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0 +frame_parms->pucch_config_common.n1PUCCH_AN; 
	  //msg("nCCE0 %d, n1_pucch0 %d\n",nCCE0,n1_pucch0);
	}
	else
	  *n1_pucch0 = -1;
      }
      else if (subframe == 4) {  // ACK subframes 9 and 0
	if (phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[0]>0) {
	  nCCE1 = phy_vars_eNB->dlsch_eNB[UE_id][0]->nCCE[0];
	  *n1_pucch1 = get_Np(frame_parms->N_RB_DL,nCCE1,1) + nCCE1 + frame_parms->pucch_config_common.n1PUCCH_AN; 
	}
	else
	  *n1_pucch1 = -1;

	if (phy_vars_eNB->dlsch_eNB[UE_id][0]->subframe_tx[9]>0) {
	  nCCE0 = phy_vars_eNB->dlsch_eNB[UE_id][0]->nCCE[9];
	  *n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0 +frame_parms->pucch_config_common.n1PUCCH_AN; 
	}
	else
	  *n1_pucch0 = -1;
      }
      else {
	msg("[PHY][eNB%d] Frame %d: phy_procedures_lte.c: get_n1pucch, illegal subframe %d for tdd_config %d\n",
	    phy_vars_eNB->Mod_id,mac_xface->frame,subframe,frame_parms->tdd_config);
	return;
      }
      break;
    }  // switch tdd_config     
    // Don't handle the case M>2
    *n1_pucch2 = -1;
    *n1_pucch3 = -1;
  }
}



void phy_procedures_eNB_RX(unsigned char last_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {
  //RX processing
  unsigned int l, ret,i,j;
  unsigned int sect_id=0,UE_id=0;
  int *ulsch_power;
  unsigned char harq_pid, round;
  int sync_pos;
  u8 SR_payload,*pucch_payload,pucch_payload0[2],pucch_payload1[2];
  s16 n1_pucch0,n1_pucch1,n1_pucch2,n1_pucch3;
  u8 do_SR=0,pucch_sel;
  s16 metric0,metric1;
  ANFBmode_t bundling_flag;
  PUCCH_FMT_t format;
  u8 nPRS;
  u8 two_ues_connected = 0;
  UCI_format_t feedback_fmt;

  //  msg("Running phy_procedures_eNB_RX(%d)\n",last_slot);
  if (abstraction_flag == 0) {
    for (l=0;l<phy_vars_eNB->lte_frame_parms.symbols_per_tti/2;l++) {
      
      for (sect_id=0;sect_id<number_of_cards;sect_id++) {
	slot_fep_ul(&phy_vars_eNB->lte_frame_parms,
		    &phy_vars_eNB->lte_eNB_common_vars,
		    l,
		    last_slot,
		    sect_id,
#ifdef USER_MODE
		    0
#else
		    1
#endif
		    );
      }
    }
    
    sect_id = 0;
    /*
    for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
      
      if ((phy_vars_eNB->eNB_UE_stats[UE_id].mode>PRACH) && (last_slot%2==1)) {
#ifdef DEBUG_PHY_PROC	
	debug_msg("[PHY][eNB%d] frame %d, slot %d: Doing SRS estimation and measurements for UE_id %d (UE_mode %d)\n",
		  phy_vars_eNB->Mod_id,
		  mac_xface->frame, last_slot, 
		  UE_id,phy_vars_eNB->eNB_UE_stats[UE_id].mode);
#endif
	for (sect_id=0;sect_id<number_of_cards;sect_id++) {
	  
	  lte_srs_channel_estimation(&phy_vars_eNB->lte_frame_parms,
				     &phy_vars_eNB->lte_eNB_common_vars,
				     &phy_vars_eNB->lte_eNB_srs_vars[UE_id],
				     &phy_vars_eNB->soundingrs_ul_config_dedicated[UE_id],
				     last_slot>>1,
				     sect_id);
	  lte_eNB_srs_measurements(phy_vars_eNB,
				   sect_id,
				   UE_id,
				   1);
#ifdef DEBUG_PHY_PROC
	  debug_msg("[PHY][eNB%d] frame %d, slot %d: UE_id %d, sect_id %d: RX RSSI %d (from SRS)\n",
		    phy_vars_eNB->Mod_id,
		    mac_xface->frame, last_slot, 
		    UE_id,sect_id,
		    phy_vars_eNB->PHY_measurements_eNB[sect_id].rx_rssi_dBm[UE_id]);
#endif
	}
	
	sect_id=0;
#ifdef USER_MODE
	//write_output("srs_est0.m","srsest0",phy_vars_eNB->lte_eNB_common_vars.srs_ch_estimates[0][0],512,1,1);
	//write_output("srs_est1.m","srsest1",phy_vars_eNB->lte_eNB_common_vars.srs_ch_estimates[0][1],512,1,1);
#endif
	
	//msg("timing advance in\n");
	sync_pos = lte_est_timing_advance(&phy_vars_eNB->lte_frame_parms,
					  &phy_vars_eNB->lte_eNB_srs_vars[UE_id],
					  &sect_id,
					  phy_vars_eNB->first_run_timing_advance[UE_id],
					  number_of_cards,
					  24576);
	
	//msg("timing advance out\n");
	
	//phy_vars_eNB->eNB_UE_stats[UE_id].UE_timing_offset = sync_pos - phy_vars_eNB->lte_frame_parms.nb_prefix_samples/8;
	phy_vars_eNB->eNB_UE_stats[UE_id].UE_timing_offset = 0;
	phy_vars_eNB->eNB_UE_stats[UE_id].sector = sect_id;
#ifdef DEBUG_PHY_PROC	
	debug_msg("[PHY][eNB%d] frame %d, slot %d: user %d in sector %d: timing_advance = %d\n",
		  phy_vars_eNB->Mod_id,
		  mac_xface->frame, last_slot, 
		  UE_id, sect_id,
		  phy_vars_eNB->eNB_UE_stats[UE_id].UE_timing_offset);
#endif
      }
    }
   */   	
  }

  // Check for active processes in current subframe
  harq_pid = subframe2harq_pid(&phy_vars_eNB->lte_frame_parms,last_slot>>1);

#ifdef OPENAIR2
  if ((phy_vars_eNB->eNB_UE_stats[0].mode == PUSCH) && (phy_vars_eNB->eNB_UE_stats[1].mode == PUSCH))
    two_ues_connected = 1;
#else
    two_ues_connected = 1;
#endif


  for (i=0;i<NUMBER_OF_UE_MAX;i++) { 

    if ((i == 1) && (phy_vars_eNB->cooperation_flag > 0) && (two_ues_connected == 1))
      break;


#ifdef OPENAIR2
    if (phy_vars_eNB->eNB_UE_stats[i].mode == RA_RESPONSE)
      process_RRCConnRequest(phy_vars_eNB,last_slot,i,harq_pid);
#endif

    /*
    debug_msg("eNB ULSCH checking UE %d : rnti %x, mode %s, subframe_scheduling %d\n",i,
	phy_vars_eNB->ulsch_eNB[i]->rnti,
	mode_string[phy_vars_eNB->eNB_UE_stats[i].mode],
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->subframe_scheduling_flag);
    */

    if ((phy_vars_eNB->ulsch_eNB[i]) &&
	(phy_vars_eNB->ulsch_eNB[i]->rnti>0) &&
	(phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->subframe_scheduling_flag==1) && 
	((last_slot%2)==1)) {

      round = phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round;

#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB%d] frame %d, slot %d, subframe %d: Scheduling ULSCH %d Reception for rnti %x harq_pid %d, rxdataF_ext %p\n",
	  phy_vars_eNB->Mod_id,
	  mac_xface->frame,last_slot,last_slot>>1,i,phy_vars_eNB->ulsch_eNB[i]->rnti,harq_pid,
	  phy_vars_eNB->lte_eNB_ulsch_vars[0]->rxdataF_ext);
#endif
  
#ifdef DEBUG_PHY_PROC
      if (phy_vars_eNB->ulsch_eNB[i]->RRCConnRequest_flag == 1)
	msg("[PHY][eNB%d] frame %d, slot %d, subframe %d: Scheduling ULSCH Reception for RRCConnRequest in Sector %d\n",
	    phy_vars_eNB->Mod_id,
	    mac_xface->frame,last_slot,last_slot>>1,phy_vars_eNB->eNB_UE_stats[i].sector);
      else
	msg("[PHY][eNB%d] frame %d, slot %d, subframe %d: Scheduling ULSCH Reception for UE %d Mode %s sect_id %d\n",
	    phy_vars_eNB->Mod_id,
	    mac_xface->frame,last_slot,last_slot>>1,i,mode_string[phy_vars_eNB->eNB_UE_stats[i].mode],phy_vars_eNB->eNB_UE_stats[i].sector);
#endif

      nPRS = 0;

      phy_vars_eNB->ulsch_eNB[i]->cyclicShift = (phy_vars_eNB->ulsch_eNB[i]->n_DMRS2 + phy_vars_eNB->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift + nPRS)%12;

#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB %d] Subframe %d demodulating PUSCH (harq_pid %d): first_rb %d, nb_rb %d, cyclic_shift %d (n_DMRS2 %d, cyclicShift %d, nprs %d) \n",
	  phy_vars_eNB->Mod_id,last_slot>>1,harq_pid,
	  phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->first_rb,
	  phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->nb_rb,
	  phy_vars_eNB->ulsch_eNB[i]->cyclicShift,
	  phy_vars_eNB->ulsch_eNB[i]->n_DMRS2,
	  phy_vars_eNB->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift,
	  nPRS);
#endif
      if (abstraction_flag==0) {
	rx_ulsch(&phy_vars_eNB->lte_eNB_common_vars,
		 phy_vars_eNB->lte_eNB_ulsch_vars[i],  
		 &phy_vars_eNB->lte_frame_parms,
		 last_slot>>1,
		 phy_vars_eNB->eNB_UE_stats[i].sector,  // this is the effective sector id
		 phy_vars_eNB->ulsch_eNB[i],
		 (two_ues_connected==1 ? phy_vars_eNB->cooperation_flag : 0));
      }
      else {
	rx_ulsch_emul(phy_vars_eNB,
		      last_slot>>1,
		      phy_vars_eNB->eNB_UE_stats[i].sector,  // this is the effective sector id
		      i);
      }


      for (j=0;j<phy_vars_eNB->lte_frame_parms.nb_antennas_rx;j++)
	phy_vars_eNB->eNB_UE_stats[i].UL_rssi[j] = dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power[j]) - phy_vars_eNB->rx_total_gain_eNB_dB;

#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB%d] frame %d, slot %d, subframe %d: ULSCH RX power (%d,%d) dB\n",
	  phy_vars_eNB->Mod_id,
	  mac_xface->frame,last_slot,last_slot>>1,dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power[0]),dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power[1]));
#endif

      if (abstraction_flag == 0) {
	ret = ulsch_decoding(phy_vars_eNB->lte_eNB_ulsch_vars[i]->llr,
			     &phy_vars_eNB->lte_frame_parms,
			     phy_vars_eNB->ulsch_eNB[i],
			     last_slot>>1,
			     0);  // control_only_flag
      }
      else {
	ret = ulsch_decoding_emul(phy_vars_eNB,
				  last_slot>>1,
				  i);
      }
#ifdef DEBUG_PHY_PROC
      msg("[PHY][eNB%d] frame %d, slot %d, subframe %d: ULSCH %d RX power (%d,%d) dB ACK (%d,%d), decoding ret %d\n",
	  phy_vars_eNB->Mod_id,
	  mac_xface->frame,last_slot,last_slot>>1,i,dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power[0]),dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power[1]),phy_vars_eNB->ulsch_eNB[i]->o_ACK[0],phy_vars_eNB->ulsch_eNB[i]->o_ACK[1],ret);
#endif //DEBUG_PHY_PROC
      if ((two_ues_connected==1) && (phy_vars_eNB->cooperation_flag==2)) {
	for (j=0;j<phy_vars_eNB->lte_frame_parms.nb_antennas_rx;j++) {
	  phy_vars_eNB->eNB_UE_stats[i].UL_rssi[j] = dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power_0[j]) 
	    - phy_vars_eNB->rx_total_gain_eNB_dB;
	  phy_vars_eNB->eNB_UE_stats[i+1].UL_rssi[j] = dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power_1[j]) 
	    - phy_vars_eNB->rx_total_gain_eNB_dB;
	}
#ifdef DEBUG_PHY_PROC
	debug_msg("[PHY_PROCEDURES_eNB] frame %d, slot %d, subframe %d: ULSCH %d RX power UE0 (%d,%d) dB RX power UE1 (%d,%d)\n",
		  mac_xface->frame,last_slot,last_slot>>1,i,
		  dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power_0[0]),
		  dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power_0[1]),
		  dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power_1[0]),
		  dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power_1[1]));
#endif
      }
      else {
	for (j=0;j<phy_vars_eNB->lte_frame_parms.nb_antennas_rx;j++)
	  phy_vars_eNB->eNB_UE_stats[i].UL_rssi[j] = dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power[j]) 
	    - phy_vars_eNB->rx_total_gain_eNB_dB;
#ifdef DEBUG_PHY_PROC
	debug_msg("[PHY_PROCEDURES_eNB] frame %d, slot %d, subframe %d: ULSCH %d RX power (%d,%d) dB\n",
		  mac_xface->frame,last_slot,last_slot>>1,i,
		  dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power[0]),
		  dB_fixed(phy_vars_eNB->lte_eNB_ulsch_vars[i]->ulsch_power[1]));
#endif
      }
      
    
      phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round]++;
 
      phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->subframe_scheduling_flag=0;

      
      if (phy_vars_eNB->ulsch_eNB[i]->cqi_crc_status == 1) {
#ifdef DEBUG_PHY_PROC
	if (((mac_xface->frame%10) == 0) || (mac_xface->frame < 50)) 
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

	phy_vars_eNB->ulsch_eNB[i]->o_ACK[0] = 0;
	phy_vars_eNB->ulsch_eNB[i]->o_ACK[1] = 0;


	if (phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round==
	    phy_vars_eNB->ulsch_eNB[i]->Mdlharq) {
	  phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round=0;
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_errors[harq_pid]++;
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_consecutive_errors[harq_pid]++;
	}

	if (phy_vars_eNB->ulsch_eNB[i]->RRCConnRequest_flag == 1) {
	  msg("[PHY_PROCEDURES_eNB] frame %d, slot %d, subframe %d, UE %d: Error receiving ULSCH (RRCConnectionRequest).\n",mac_xface->frame,last_slot,last_slot>>1, i);
	  //	eNB_generate_RRCConnReq_ack = 0;
	  phy_vars_eNB->eNB_UE_stats[i].mode = PRACH;
	  remove_ue(phy_vars_eNB->eNB_UE_stats[i].crnti,phy_vars_eNB,abstraction_flag);
#ifdef OPENAIR2
	  mac_xface->cancel_ra_proc(0,0);
#endif
	}

	// If we've dropped the UE, go back to PRACH mode for this UE
	if (phy_vars_eNB->eNB_UE_stats[i].ulsch_consecutive_errors[harq_pid] == 20) {
	  msg("[PHY_PROCEDURES_eNB] frame %d, slot %d, subframe %d, UE %d: ULSCH consecutive error count reached, removing UE\n",
	      mac_xface->frame,last_slot,last_slot>>1, i);
	  phy_vars_eNB->eNB_UE_stats[i].mode = PRACH;
	  remove_ue(phy_vars_eNB->eNB_UE_stats[i].crnti,phy_vars_eNB,abstraction_flag);
#ifdef OPENAIR2
	  mac_xface->cancel_ra_proc(0,0);
#endif
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_consecutive_errors[harq_pid]=0;
	}
      }  // ulsch in error
      else {
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->phich_active = 1;
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->phich_ACK = 1;
	phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->round = 0;
	phy_vars_eNB->eNB_UE_stats[i].ulsch_consecutive_errors[harq_pid] = 0;

	if (phy_vars_eNB->ulsch_eNB[i]->RRCConnRequest_flag == 1) {
#ifdef OPENAIR2
#ifdef DEBUG_PHY_PROC
	  msg("[PHY][eNB %d] Terminating ra_proc for harq %d, UE %d\n",phy_vars_eNB->Mod_id,harq_pid,i);
#endif
	  mac_xface->terminate_ra_proc(phy_vars_eNB->Mod_id,phy_vars_eNB->ulsch_eNB[i]->rnti,phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->b);
#endif

	  phy_vars_eNB->eNB_UE_stats[i].mode = PUSCH;
	  phy_vars_eNB->ulsch_eNB[i]->RRCConnRequest_flag = 0;

#ifdef DEBUG_PHY_PROC
	  msg("[PHY][eNB %d] Frame %d : RX Subframe %d Setting UE %d mode to PUSCH\n",phy_vars_eNB->Mod_id,mac_xface->frame,last_slot>>1,i);
#endif //DEBUG_PHY_PROC

	  for (j=0;j<phy_vars_eNB->dlsch_eNB[i][0]->Mdlharq;j++) {
	    phy_vars_eNB->eNB_UE_stats[i].dlsch_NAK[j]=0;
	    phy_vars_eNB->eNB_UE_stats[i].dlsch_sliding_cnt=0;
	  }

	  //mac_xface->macphy_exit("Mode PUSCH. Exiting.\n");
	}
	else {
#ifdef DEBUG_PHY_PROC
	  msg("[PHY][eNB] Frame %d, Subframe %d : ULSCH SDU (RX) %d bytes:",mac_xface->frame,last_slot>>1,phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->TBS>>3);
	  for (j=0;j<phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->TBS>>3;j++)
	    msg("%x.",phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->b[j]);
	  msg("\n");
#endif

#ifdef OPENAIR2
	  mac_xface->rx_sdu(phy_vars_eNB->Mod_id,phy_vars_eNB->ulsch_eNB[i]->rnti,phy_vars_eNB->ulsch_eNB[i]->harq_processes[harq_pid]->b);
#endif
	}
      }  // ulsch not in error
      
      // process HARQ feedback
#ifdef DEBUG_PHY_PROC
      debug_msg("[PHY_PROCEDURES_eNB] eNB %d Processing HARQ feedback for UE %d\n",phy_vars_eNB->Mod_id,i);
#endif
      process_HARQ_feedback(i,
			    last_slot>>1,
			    phy_vars_eNB,
			    1, // pusch_flag
			    0,
			    0,
			    0);
      

#ifdef DEBUG_PHY_PROC
      debug_msg("[PHY_PROCEDURES_eNB] frame %d, slot %d, subframe %d, sect %d: received ULSCH harq_pid %d for UE %d, ret = %d, CQI CRC Status %d, ACK %d,%d, ulsch_errors %d/%d\n",mac_xface->frame, last_slot, last_slot>>1, phy_vars_eNB->eNB_UE_stats[i].sector, harq_pid, i, ret, phy_vars_eNB->ulsch_eNB[i]->cqi_crc_status, phy_vars_eNB->ulsch_eNB[i]->o_ACK[0],	phy_vars_eNB->ulsch_eNB[i]->o_ACK[1], phy_vars_eNB->eNB_UE_stats[i].ulsch_errors[harq_pid],phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][0]);
#endif


      if (mac_xface->frame % 100 == 0) {
	if ((phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][round] - 
	     phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts_last[harq_pid][round]) != 0)
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_round_fer[harq_pid][round] = 
	    (100*(phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors[harq_pid][round] - 
		  phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors_last[harq_pid][round]))/
	    (phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][round] - 
	     phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts_last[harq_pid][round]);

	phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts_last[harq_pid][round] = 
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_decoding_attempts[harq_pid][round];
	phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors_last[harq_pid][round] = 
	  phy_vars_eNB->eNB_UE_stats[i].ulsch_round_errors[harq_pid][round];
      }
    }

#ifdef PUCCH
    else if ((phy_vars_eNB->dlsch_eNB[i][0]) &&
	     (phy_vars_eNB->dlsch_eNB[i][0]->rnti>0) &&
	     ((last_slot%2)==1)){ // check for PUCCH

      // check SR availability
      // do_SR = is_SR_subframe(phy_vars_eNB,i,last_slot>>1);
      do_SR = 0;
    
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
	
	  //	msg("[PHY][eNB %d] Frame %d: Checking SR\n",phy_vars_eNB->Mod_id,mac_xface->frame);
	  if (abstraction_flag == 0)
	    metric0 = rx_pucch(&phy_vars_eNB->lte_eNB_common_vars,
			       &phy_vars_eNB->lte_frame_parms,
			       phy_vars_eNB->ncs_cell,
			       pucch_format1,
			       &phy_vars_eNB->pucch_config_dedicated[i],
			       phy_vars_eNB->scheduling_request_config[i].sr_PUCCH_ResourceIndex,
			       0, // n2_pucch
			       1, // shortened format
			       &SR_payload,
			       last_slot>>1);
	  else
	    metric0 = rx_pucch_emul(phy_vars_eNB,
				    i,
				    pucch_format1,
				    &SR_payload,
				    last_slot>>1);

	  if (SR_payload == 1) {
	    //	  msg("[PHY][UE %d] Frame %d: Got SR, transmitting to MAC\n",phy_vars_eNB->Mod_id,mac_xface->frame);
	    mac_xface->SR_indication(phy_vars_eNB->Mod_id,phy_vars_eNB->dlsch_eNB[i][0]->rnti);
	  }
	}
      
	if ((n1_pucch0==-1) && (n1_pucch1==-1)) { // just check for SR
	  return;
	}
	else if (phy_vars_eNB->lte_frame_parms.frame_type==0) { // FDD
	  // if SR was detected, use the n1_pucch from SR, else use n1_pucch0
	  n1_pucch0 = (SR_payload==1) ? phy_vars_eNB->scheduling_request_config[i].sr_PUCCH_ResourceIndex:n1_pucch0;
	  if (abstraction_flag == 0)
	    metric0 = rx_pucch(&phy_vars_eNB->lte_eNB_common_vars,
			       &phy_vars_eNB->lte_frame_parms,
			       phy_vars_eNB->ncs_cell,
			       pucch_format1a,
			       &phy_vars_eNB->pucch_config_dedicated[i],
			       (u16)n1_pucch0,
			       0, //n2_pucch
			       1, // shortened format
			       pucch_payload0,
			       last_slot>>1);
	  else
	    metric0 = rx_pucch_emul(phy_vars_eNB,i,
				    pucch_format1a,
				    pucch_payload0,
				    last_slot>>1);
	
	}
	else {  //TDD
	
	  bundling_flag = phy_vars_eNB->pucch_config_dedicated[i].tdd_AckNackFeedbackMode;
	
	  // fix later for 2 TB case and format1b
	  if (bundling_flag==bundling) {
	    format = pucch_format1a;
	    //	  msg("PUCCH 1a\n");
	  }
	  else {
	    format = pucch_format1b;
	    //	  msg("PUCCH 1b\n");
	  }
	
	  // if SR was detected, use the n1_pucch from SR, else use n1_pucch0
	  if (SR_payload==1) {
	    if (abstraction_flag == 0) 
	      metric0 = rx_pucch(&phy_vars_eNB->lte_eNB_common_vars,
				 &phy_vars_eNB->lte_frame_parms,
				 phy_vars_eNB->ncs_cell,
				 format,
				 &phy_vars_eNB->pucch_config_dedicated[i],
				 phy_vars_eNB->scheduling_request_config[i].sr_PUCCH_ResourceIndex,
				 0, //n2_pucch
				 1, // shortened format
				 pucch_payload0,
				 last_slot>>1);
	    else
	      metric0 = rx_pucch_emul(phy_vars_eNB,i,
				      format,
				      pucch_payload0,
				      last_slot>>1);

	  } 
	  else {  //using n1_pucch0/n1_pucch1 resources
#ifdef DEBUG_PHY_PROC	  
	    msg("[PHY][eNB %d] Frame %d: Checking ACK/NAK (%d,%d,%d,%d)\n",phy_vars_eNB->Mod_id,mac_xface->frame,
		n1_pucch0,n1_pucch1,n1_pucch2,n1_pucch3);
#endif
	    metric0=0;
	    metric1=0;

	    // Check n1_pucch0 metric
	    if (n1_pucch0 != -1) {
	      if (abstraction_flag == 0) 
		metric0 = rx_pucch(&phy_vars_eNB->lte_eNB_common_vars,
				   &phy_vars_eNB->lte_frame_parms,
				   phy_vars_eNB->ncs_cell,
				   format,
				   &phy_vars_eNB->pucch_config_dedicated[i],
				   (u16)n1_pucch0,
				   0, // n2_pucch
				   1, // shortened format
				   pucch_payload0,
				   last_slot>>1);
	      else
		metric0 = rx_pucch_emul(phy_vars_eNB,i,
					format,
					pucch_payload0,
					last_slot>>1);
	    }

	    // Check n1_pucch1 metric
	    if (n1_pucch1 != -1) {
	      if (abstraction_flag == 0)
		metric1 = rx_pucch(&phy_vars_eNB->lte_eNB_common_vars,
				   &phy_vars_eNB->lte_frame_parms,
				   phy_vars_eNB->ncs_cell,
				   format,
				   &phy_vars_eNB->pucch_config_dedicated[i],
				   (u16)n1_pucch1,
				   0, //n2_pucch
				   1, // shortened format
				   pucch_payload1,
				   last_slot>>1);
	      else
		metric1 = rx_pucch_emul(phy_vars_eNB,i,
					format,
					pucch_payload1,
					last_slot>>1);
	    }
	  	  
	    if (bundling_flag == multiplexing) {
	      pucch_payload = (metric1>metric0) ? pucch_payload1 : pucch_payload0;
	      pucch_sel     = (metric1>metric0) ? 1 : 0;
	    }
	    else {
	    
	      if (n1_pucch1 != -1)
		pucch_payload = pucch_payload1;
	      else if (n1_pucch0 != -1)
		pucch_payload = pucch_payload0;
	    
	      pucch_sel = 2;  // indicate that this is a bundled ACK/NAK  
	    }
	  
	  
	    process_HARQ_feedback(i,last_slot>>1,phy_vars_eNB,
				  0,// pusch_flag
				  pucch_payload,
				  pucch_sel,
				  SR_payload);
	  }
	}
      } // PUCCH processing
    }
#endif
  } // loop i=0 ... NUMBER_OF_UE_MAX-1
#ifdef EMOS
  phy_procedures_emos_eNB_RX(last_slot);
#endif
    
}

void phy_procedures_eNB_lte(unsigned char last_slot, unsigned char next_slot,PHY_VARS_eNB *phy_vars_eNB,u8 abstraction_flag) {

  /*
  if (mac_xface->frame >= 1000)
    mac_xface->macphy_exit("Exiting after 1000 Frames\n");
  */

  if (((phy_vars_eNB->lte_frame_parms.frame_type == 1)&&(subframe_select(&phy_vars_eNB->lte_frame_parms,next_slot>>1)==SF_DL))||
      (phy_vars_eNB->lte_frame_parms.frame_type == 0)){
#ifdef DEBUG_PHY_PROC
    msg("[PHY][eNB%d] Frame %d: Calling phy_procedures_eNB_TX(%d)\n",
	phy_vars_eNB->Mod_id,mac_xface->frame, next_slot);
#endif
    phy_procedures_eNB_TX(next_slot,phy_vars_eNB,abstraction_flag);
  }
  if (((phy_vars_eNB->lte_frame_parms.frame_type == 1 )&&(subframe_select(&phy_vars_eNB->lte_frame_parms,last_slot>>1)==SF_UL))||
      (phy_vars_eNB->lte_frame_parms.frame_type == 0)){
#ifdef DEBUG_PHY_PROC
    msg("[PHY][eNB%d] Frame %d: Calling phy_procedures_eNB_RX(%d)\n",phy_vars_eNB->Mod_id,mac_xface->frame, last_slot);
#endif
    phy_procedures_eNB_RX(last_slot,phy_vars_eNB,abstraction_flag);
  }
  if ((subframe_select(&phy_vars_eNB->lte_frame_parms,next_slot>>1)==SF_S)) {
#ifdef DEBUG_PHY_PROC
    msg("[PHY][eNB%d] Frame %d: Calling phy_procedures_eNB_S_TX(%d)\n",
	phy_vars_eNB->Mod_id,mac_xface->frame, next_slot);
#endif
    phy_procedures_eNB_S_TX(next_slot,phy_vars_eNB,abstraction_flag);
  }
  if ((phy_vars_eNB->lte_frame_parms.frame_type==1) && (subframe_select(&phy_vars_eNB->lte_frame_parms,last_slot>>1))==SF_S) {
#ifdef DEBUG_PHY_PROC
    msg("[PHY][eNB%d] Frame %d: Calling phy_procedures_eNB_S_RX(%d)\n",
	phy_vars_eNB->Mod_id,mac_xface->frame, last_slot);
#endif
    phy_procedures_eNB_S_RX(last_slot,phy_vars_eNB,abstraction_flag);
  }
}

