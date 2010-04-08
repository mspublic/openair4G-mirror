/*________________________macphy_scheduler.c________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/

/*!\brief Initilization and reconfiguration routines for generic MAC interface */

#include "extern.h"
#include "defs.h"
#include "PHY_INTERFACE/defs.h"
#include "PHY_INTERFACE/extern.h"

#ifdef CELLULAR
#include "RRC/CELLULAR/rrc_rg_vars_extern.h"
#endif 

#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif
#define DEBUG_SCHEDULER 1

void macphy_scheduler(unsigned char last_slot) {
  u8 i,j;
 
  if(last_slot == 19)
    Mac_rlc_xface->frame++;



  for (i=0;i<NB_INST;i++) {  
    if(Mac_rlc_xface->Is_cluster_head[i]==2){  //power on procedures: neighbehood discovering and roles' affectation 
      // node is not a CH neither a MR. when a node starts it begins by trying to detect a neihgboring CH.
      //if no CH is found during a period of PHY_CHBCH_SCH_WAIT_MAX TTIs (CONTROL/defs.h) it declare itself as cluster head.
      //otherwise it tries to synchronize and associate with the founded CH
      if (last_slot == 3){
	for(j=0;j<NB_CNX_UE;j++)
	  emul_phy_sync(i,j);// prepare a data request for CHBCH detection
      }
    }

    else{
      if ((last_slot%2) == 1) {
	if (Is_rrc_registered==1) {
#ifdef DEBUG_SCHEDULER
	  msg("/******************************MACPHY_SCHEDULER:Frame %d (subframe %d), CALLING PDCP INST %d***********************/\n",Mac_rlc_xface->frame,last_slot>>1,i);
#endif
	  if(i==0){
	      Rrc_xface->Frame_index=Mac_rlc_xface->frame;
	      Mac_rlc_xface->pdcp_run(i);
#ifndef NO_RRM
	      Rrc_xface->fn_rrc();
#endif
	  }
	  
#ifdef DEBUG_SCHEDULER
	    msg("[OPENAIR2]/******************************MACPHY_SCHEDULER:Frame %d (%d), CALLING RRC INST %d***********************/\n",Mac_rlc_xface->frame,last_slot/2,i);
#endif
	    
	    Rrc_xface->rrc_rx_tx(i);
	}
	// call MAC TX procedures 
	// CH : get data for subframe n+1, schedule n+1 (TX), schedule n+2 (RX)
	// CH : generate chbch,sach : data_ind (n+1)
	if (Mac_rlc_xface->Is_cluster_head[i] == 1) {
	  // If we are in the last slot of the mini-frame now, then generate the CHSCH/CHBCH in the next slot
	  // UE : schedule n+1 (RX) from DL_SACCH      
	  // Launch NodeB CCCH Procedures for next mini-frame
	  // Prepare PHY procedures for next mini-frame (TX/RX)
#ifdef DEBUG_SCHEDULER
	  msg("[OPENAIR2]/******************************MACPHY_SCHEDULER:Frame %d,  CALLING Nodeb_TX INST %d last_slot %d***********************/\n",Mac_rlc_xface->frame,i,last_slot);
#endif
	  nodeb_mac_scheduler_tx(i,((last_slot+2)%20)>>1);      
	}
	else{ //This is an UE
#ifdef DEBUG_SCHEDULER
	  msg("[MAC][UE] Frame %d, last_slot %d: CALLING SCHEDULER\n", Mac_rlc_xface->frame,last_slot+2);
	  msg("/******************************MACPHY_SCHEDULER: CALLING UE TX/RX INST %d***********************/\n",i);
#endif
	  ue_mac_scheduler_tx(i-NB_CH_INST); // ue_mac_scheduler_tx
	  
	}
      }
      
      else if ((last_slot %2)==0) {  // call MAC RX procedures before DL/UL switch
	// CH : get_rach,get_sach : data_req (n+1)
	// UE : get_chbch,get_sach : data_req (n+1)
	// UE : generate sach : data_ind(n+1)
	
	if (Mac_rlc_xface->Is_cluster_head[i] == 1) {
	  // If we in the last slot of the mini-frame now, then generate the CHSCH/CHBCH in the next slot
	  // Launch NodeB CCCH Procedures for next mini-frame
	  // Prepare PHY procedures for next mini-frame (TX/RX)
#ifdef DEBUG_SCHEDULER
	  //msg("[][MAC] Frame %d, last_slot %d: CALLING SCHEDULER\n", Mac_rlc_xface->frame,last_slot);
#endif	
	  nodeb_mac_scheduler_rx(i);      
	  
	}
	else{
	  ue_mac_scheduler_rx(i-NB_CH_INST); // ue_mac_scheduler_rx
	  
	}
      }
      
    }
  }
}
