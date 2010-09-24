#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"


void get_RRCConnReq_alloc(unsigned char tdd_config,
			  unsigned char current_subframe, 
			  unsigned int current_frame,
			  unsigned int *frame,
			  unsigned char *subframe) {

  if (tdd_config == 3) {
    switch (current_subframe) {
      
    case 0:
    case 5:
    case 6:
      *subframe = 2;
      *frame = current_frame+2;
      break;
    case 7:
      *subframe = 3;
      *frame = current_frame+2;
      break;
    case 8:
      *subframe = 4;
      *frame = current_frame+2;
      break;
    case 9:
      *subframe = 2;
      *frame = current_frame+3;
      break;
    }
  }
}

unsigned char ul_ACK_subframe2_dl_subframe(unsigned char tdd_config,unsigned char subframe,unsigned char ACK_index) {

  switch (tdd_config) {
  case 3:
    if (subframe == 2) {  // ACK subframes 5 and 6
      return(5+ACK_index);
    }
    else if (subframe == 3) {   // ACK subframes 7 and 8
      return(6+ACK_index);
      //      return(7+ACK_index);  // To be updated
    }
    else if (subframe == 4) {  // ACK subframes 9 and 0
      return((9+ACK_index)%10);
    }
    else {
      msg("phy_procedures_lte_common.c/subframe2_dl_harq_pid: illegal subframe %d for tdd_config %d\n",
	  subframe,tdd_config);
      return(0);
    }
    break;
    
  }
  return(0);
}

unsigned char get_ack(unsigned char tdd_config,harq_status_t *harq_ack,unsigned char subframe,unsigned char *o_ACK) {

  switch (tdd_config) {
  case 3:
    if (subframe == 2) {  // ACK subframes 5 and 6
      o_ACK[0] = harq_ack[5].ack;  
      o_ACK[1] = harq_ack[6].ack;
    }
    else if (subframe == 3) {   // ACK subframes 7 and 8
      o_ACK[0] = harq_ack[6].ack; //harq_ack[7].ack;
      o_ACK[1] = harq_ack[8].ack;
    }
    else if (subframe == 4) {  // ACK subframes 9 and 0
      o_ACK[0] = harq_ack[9].ack;
      o_ACK[1] = harq_ack[0].ack;
    }
    else {
      msg("phy_procedures_lte.c: get_ack, illegal subframe %d for tdd_config %d\n",
	  subframe,tdd_config);
      return(0);
    }
    break;
    
  }
  return(0);
}

lte_subframe_t subframe_select_tdd(unsigned char tdd_config,unsigned char subframe) {

  switch (tdd_config) {

  case 3:
    if  ((subframe<1) || (subframe>=5)) 
      return(SF_DL);
    else if ((subframe>1) && (subframe < 5))  
      return(SF_UL);
    else if (subframe==1)
      return (SF_S);
    else  {
      msg("[PHY_PROCEDURES_LTE] Unknown subframe number\n");
      return(255);
    }
    break;
  default:
    msg("[PHY_PROCEDURES_LTE] Unsupported TDD mode\n");
    return(255);
    
  }
}

unsigned int is_phich_subframe(unsigned char tdd_config,unsigned char subframe) {

  switch (tdd_config) {
  case 3:
    if ((subframe == 0) || (subframe == 8) || (subframe == 9))
      return(1);
    break;
  case 4:
    if ((subframe == 0) || (subframe == 8) )
      return(1);
    break;
  case 5:
    if (subframe == 0)
      return(1);
    break;
  default:
    return(0);
    break;
  }
  return(0);
}

//#define DEBUG_PHY 1

void phy_procedures_lte(unsigned char last_slot, unsigned char next_slot) {

  //#undef DEBUG_PHY
  if (mac_xface->is_cluster_head == 0) {
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_UE_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_UE_RX(last_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_UE_S_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_UE_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_UE_RX(last_slot);
    }
  }
  else { //eNB

    //    if ((mac_xface->frame % 1000) == 0)
    //      ulsch_errors = 0;
    msg("[PHY][eNB] next_slot %d Current dlsch round %d\n",next_slot,dlsch_eNb[0]->harq_processes[0]->round);

    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_DL) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_eNB_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_UL) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_eNB_RX(last_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_TX(%d)\n",mac_xface->frame, next_slot);
#endif
      phy_procedures_eNB_S_TX(next_slot);
    }
    if (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_S) {
#ifdef DEBUG_PHY
      msg("[PHY_PROCEDURES_LTE] Frame% d: Calling phy_procedures_eNB_S_RX(%d)\n",mac_xface->frame, last_slot);
#endif
      phy_procedures_eNB_S_RX(last_slot);
    }
  }
}




