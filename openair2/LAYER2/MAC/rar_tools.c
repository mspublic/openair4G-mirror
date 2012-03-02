#include "defs.h"
#include "extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SIMULATION/TOOLS/defs.h"
#include "UTIL/LOG/log.h"
 
#define DEBUG_RAR

extern unsigned int  localRIV2alloc_LUT25[512];
extern unsigned int  distRIV2alloc_LUT25[512];
extern unsigned short RIV2nb_rb_LUT25[512];
extern unsigned short RIV2first_rb_LUT25[512];

extern inline unsigned int taus(void);

unsigned short fill_rar(u8 Mod_id,
			u8 *dlsch_buffer,
			u16 N_RB_UL,
			u8  input_buffer_length) {

  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)dlsch_buffer;
  
  RAR_PDU *rar = (RAR_PDU *)(dlsch_buffer+1);

  rarh->E                     = 0; // First and last RAR
  rarh->T                     = 0; // Preamble ID RAR
  rarh->RAPID                 = 0; // Respond to Preamble 0 only for the moment
  rar->R                      = 0;
  rar->Timing_Advance_Command = eNB_mac_inst[Mod_id].RA_template[0].timing_offset;
  rar->hopping_flag           = 0;
  rar->rb_alloc               = mac_xface->computeRIV(N_RB_UL,0,2);  // 2 RB
  rar->mcs                    = 2;                                   // mcs 2
  rar->TPC                    = 0;
  rar->UL_delay               = 0;
  rar->cqi_req                = 1;
  rar->t_crnti                = eNB_mac_inst[Mod_id].RA_template[0].rnti;

#ifdef DEBUG_RAR
  LOG_D(MAC,"[MAC][eNB %d] Generating RAR for CRNTI %x\n",Mod_id,rar->t_crnti);
#endif
  return(rar->t_crnti);
}

u16 ue_process_rar(u8 Mod_id,u8 *dlsch_buffer,u16 *t_crnti) {

  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)dlsch_buffer;
  RAR_PDU *rar = (RAR_PDU *)(dlsch_buffer+1);
#ifdef DEBUG_RAR
  LOG_T(MAC,"[UE %d] rarh->E %d\n",Mod_id,rarh->E);
  LOG_D(MAC,"[UE %d] rarh->T %d\n",Mod_id,rarh->T);
  LOG_D(MAC,"[UE %d] rarh->RAPID %d\n",Mod_id,rarh->RAPID);

  LOG_D(MAC,"[UE %d] rar->R %d\n",Mod_id,rar->R);
  LOG_D(MAC,"[UE %d] rar->Timing_Advance_Command %d\n",Mod_id,rar->Timing_Advance_Command);
  LOG_D(MAC,"[UE %d] rar->hopping_flag %d\n",Mod_id,rar->hopping_flag);
  LOG_D(MAC,"[UE %d] rar->rb_alloc %d\n",Mod_id,rar->rb_alloc);
  LOG_D(MAC,"[UE %d] rar->mcs %d\n",Mod_id,rar->mcs);
  LOG_D(MAC,"[UE %d] rar->TPC %d\n",Mod_id,rar->TPC);
  LOG_D(MAC,"[UE %d] rar->UL_delay %d\n",Mod_id,rar->UL_delay);
  LOG_D(MAC,"[UE %d] rar->cqi_req %d\n",Mod_id,rar->cqi_req);
  LOG_D(MAC,"[UE %d] rar->t_crnti %x\n",Mod_id,rar->t_crnti);
#endif
  *t_crnti = rar->t_crnti;
  return(rar->Timing_Advance_Command);
}
