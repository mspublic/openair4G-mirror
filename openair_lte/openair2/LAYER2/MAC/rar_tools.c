#include "LAYER2/MAC/defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SIMULATION/TOOLS/defs.h"


//#define DEBUG_RAR

extern unsigned int  localRIV2alloc_LUT25[512];
extern unsigned int  distRIV2alloc_LUT25[512];
extern unsigned short RIV2nb_rb_LUT25[512];
extern unsigned short RIV2first_rb_LUT25[512];

extern inline unsigned int taus(void);

unsigned short fill_rar(unsigned char *dlsch_buffer,
			unsigned short N_RB_UL,
			unsigned char input_buffer_length,
			unsigned short timing_advance_cmd) {

  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)dlsch_buffer;

  RAR_PDU *rar = (RAR_PDU *)(dlsch_buffer+1);

  rarh->E                     = 0; // First and last RAR
  rarh->T                     = 0; // Preamble ID RAR
  rarh->RAPID                 = 0; // Respond to Preamble 0 only for the moment

  rar->R                      = 0;
  rar->Timing_Advance_Command = timing_advance_cmd;
  rar->hopping_flag           = 0;
  rar->rb_alloc               = computeRIV(N_RB_UL,0,2);
  rar->mcs                    = 2;
  rar->TPC                    = 0;
  rar->UL_delay               = 0;
  rar->cqi_req                = 1;
  rar->t_crnti                = taus();

#ifdef DEBUG_RAR
  debug_msg("[MAC eNB] Generating RAR for CRNTI %x\n",rar->t_crnti);
#endif
  return(rar->t_crnti);
}

unsigned short process_rar(unsigned char *dlsch_buffer,unsigned short *t_crnti) {

  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)dlsch_buffer;
  RAR_PDU *rar = (RAR_PDU *)(dlsch_buffer+1);
#ifdef DEBUG_RAR
  msg("[MAC UE] rarh->E %d\n",rarh->E);
  msg("[MAC UE] rarh->T %d\n",rarh->T);
  msg("[MAC UE] rarh->RAPID %d\n",rarh->RAPID);

  msg("[MAC UE] rar->R %d\n",rar->R);
  msg("[MAC UE] rar->Timing_Advance_Command %d\n",rar->Timing_Advance_Command);
  msg("[MAC UE] rar->hopping_flag %d\n",rar->hopping_flag);
  msg("[MAC UE] rar->rb_alloc %d\n",rar->rb_alloc);
  msg("[MAC UE] rar->mcs %d\n",rar->mcs);
  msg("[MAC UE] rar->TPC %d\n",rar->TPC);
  msg("[MAC UE] rar->UL_delay %d\n",rar->UL_delay);
  msg("[MAC UE] rar->cqi_req %d\n",rar->cqi_req);
  msg("[MAC UE] rar->t_crnti %x\n",rar->t_crnti);
#endif
  *t_crnti = rar->t_crnti;
  return(rar->Timing_Advance_Command);
}
