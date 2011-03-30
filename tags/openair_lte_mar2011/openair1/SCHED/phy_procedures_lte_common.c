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

u8 get_RRCConnReq_harq_pid(unsigned char tdd_config,
			     unsigned char current_subframe) {

  u8 ul_subframe;

  if (tdd_config == 3) {
    switch (current_subframe) {
      
    case 0:
    case 5:
    case 6:
      ul_subframe = 2;
      break;
    case 7:
      ul_subframe = 3;
      break;
    case 8:
      ul_subframe = 4;
      break;
    case 9:
      ul_subframe = 2;
      break;
    }
  }
  return(subframe2harq_pid_tdd(tdd_config,ul_subframe));
}

unsigned char ul_ACK_subframe2_dl_subframe(unsigned char tdd_config,unsigned char subframe,unsigned char ACK_index) {

  switch (tdd_config) {
  case 3:
    if (subframe == 2) {  // ACK subframes 5 and 6
      return(5+ACK_index);
    }
    else if (subframe == 3) {   // ACK subframes 7 and 8
      return(7+ACK_index);  // To be updated
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

  //  printf("get_ack: SF %d\n",subframe);

  switch (tdd_config) {
  case 3:
    if (subframe == 2) {  // ACK subframes 5 and 6
      o_ACK[0] = harq_ack[5].ack;  
      o_ACK[1] = harq_ack[6].ack;
    }
    else if (subframe == 3) {   // ACK subframes 7 and 8
      o_ACK[0] = harq_ack[7].ack;
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


LTE_eNB_UE_stats* get_eNB_UE_stats(u8 Mod_id, u16 rnti) {
  s8 UE_id;
  if ((PHY_vars_eNb_g == NULL) || (PHY_vars_eNb_g[Mod_id] == NULL)) {
    msg("get_eNB_UE_stats: No phy_vars_eNb found (or not allocated) for Mod_id %d\n",Mod_id);
    return NULL;
  }
  UE_id = find_ue(rnti, PHY_vars_eNb_g[Mod_id]);
  if (UE_id == -1) {
    msg("get_eNB_UE_stats: UE with rnti %d not found\n",rnti);
    return NULL;
  }
  return(&PHY_vars_eNb_g[Mod_id]->eNB_UE_stats[UE_id]);
}

