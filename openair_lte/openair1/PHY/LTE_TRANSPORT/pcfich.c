#include "PHY/defs.h"
#include "MAC_INTERFACE/extern.h"

unsigned short pcfich_reg[4];

#define DEBUG_PCFICH

void generate_pcfich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned short kbar = 6 * (frame_parms->Nid_cell %(2*frame_parms->N_RB_DL));
  pcfich_reg[0] = kbar;
  pcfich_reg[1] = (kbar + (frame_parms->N_RB_DL>>1)*6)%(frame_parms->N_RB_DL*12);
  pcfich_reg[2] = (kbar + (frame_parms->N_RB_DL)*6)%(frame_parms->N_RB_DL*12);
  pcfich_reg[3] = (kbar + ((3*frame_parms->N_RB_DL)>>1)*6)%(frame_parms->N_RB_DL*12);
  
#ifdef DEBUG_PCFICH
  debug_msg("pcfich_reg : %d,%d,%d,%d\n",pcfich_reg[0],pcfich_reg[1],pcfich_reg[2],pcfich_reg[3]);
#endif
}
