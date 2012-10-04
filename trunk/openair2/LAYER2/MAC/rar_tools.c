/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2010 Eurecom

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
/*! \file rar_tools.c
* \brief random access tools
* \author Raymond Knopp
* \date 2011
* \version 0.5
* @ingroup _mac

*/

#include "defs.h"
#include "extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SIMULATION/TOOLS/defs.h"
#include "UTIL/LOG/log.h"
#include "OCG.h"
#include "OCG_extern.h"

#define DEBUG_RAR

extern unsigned int  localRIV2alloc_LUT25[512];
extern unsigned int  distRIV2alloc_LUT25[512];
extern unsigned short RIV2nb_rb_LUT25[512];
extern unsigned short RIV2first_rb_LUT25[512];

extern inline unsigned int taus(void);

unsigned short fill_rar(u8 Mod_id,
			u32 frame,
			u8 *dlsch_buffer,
			u16 N_RB_UL,
			u8 input_buffer_length) {

  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)dlsch_buffer;
  RAR_PDU *rar = (RAR_PDU *)(dlsch_buffer+1);
  int i,ra_idx;

  for (i=0;i<NB_RA_PROC_MAX;i++) {
    if (eNB_mac_inst[Mod_id].RA_template[i].generate_rar == 1) {
      ra_idx=i;
      eNB_mac_inst[Mod_id].RA_template[i].generate_rar = 0;
      break;
    }
  }

  // subheader fixed 
  rarh->E                     = 0; // First and last RAR
  rarh->T                     = 0; // Preamble ID RAR
  rarh->RAPID                 = eNB_mac_inst[Mod_id].RA_template[ra_idx].preamble_index; // Respond to Preamble 0 only for the moment
  rar->R                      = 0;
  rar->Timing_Advance_Command = eNB_mac_inst[Mod_id].RA_template[ra_idx].timing_offset/4;
  rar->hopping_flag           = 0;
  rar->rb_alloc               = mac_xface->computeRIV(N_RB_UL,0,2);  // 2 RB
  rar->mcs                    = 2;                                   // mcs 2
  rar->TPC                    = 4;   // 2 dB power adjustment
  rar->UL_delay               = 0;
  rar->cqi_req                = 1;
  rar->t_crnti                = eNB_mac_inst[Mod_id].RA_template[ra_idx].rnti;

  LOG_D(MAC,"[eNB %d][RAPROC] Frame %d Generating RAR for CRNTI %x,preamble %d/%d\n",Mod_id,frame,rar->t_crnti,rarh->RAPID,eNB_mac_inst[Mod_id].RA_template[0].preamble_index);

#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.opt_enabled){
    trace_pdu(1, dlsch_buffer, input_buffer_length, Mod_id, 2, 1,frame,0,0);
    LOG_D(OPT,"[eNB %d][RAPROC] RAR Frame %d trace pdu for rnti %x and  rapid %d size %d\n", 
	  Mod_id, frame, rar->t_crnti, rarh->RAPID, input_buffer_length);
  } 
#endif 
  return(rar->t_crnti);
}

u16 ue_process_rar(u8 Mod_id, u32 frame, u8 *dlsch_buffer,u16 *t_crnti,u8 preamble_index) {

  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)dlsch_buffer;
  RAR_PDU *rar = (RAR_PDU *)(dlsch_buffer+1);
  
  LOG_D(MAC,"[UE %d][RAPROC] Frame %d : process RAR : preamble_index %d, received %d\n",Mod_id,frame,preamble_index,rarh->RAPID);
  
#ifdef DEBUG_RAR
  LOG_D(MAC,"[UE %d][RAPROC] rarh->E %d\n",Mod_id,rarh->E);
  LOG_D(MAC,"[UE %d][RAPROC] rarh->T %d\n",Mod_id,rarh->T);
  LOG_D(MAC,"[UE %d][RAPROC] rarh->RAPID %d\n",Mod_id,rarh->RAPID);

  LOG_D(MAC,"[UE %d][RAPROC] rar->R %d\n",Mod_id,rar->R);
  LOG_D(MAC,"[UE %d][RAPROC] rar->Timing_Advance_Command %d\n",Mod_id,rar->Timing_Advance_Command);
  LOG_D(MAC,"[UE %d][RAPROC] rar->hopping_flag %d\n",Mod_id,rar->hopping_flag);
  LOG_D(MAC,"[UE %d][RAPROC] rar->rb_alloc %d\n",Mod_id,rar->rb_alloc);
  LOG_D(MAC,"[UE %d][RAPROC] rar->mcs %d\n",Mod_id,rar->mcs);
  LOG_D(MAC,"[UE %d][RAPROC] rar->TPC %d\n",Mod_id,rar->TPC);
  LOG_D(MAC,"[UE %d][RAPROC] rar->UL_delay %d\n",Mod_id,rar->UL_delay);
  LOG_D(MAC,"[UE %d][RAPROC] rar->cqi_req %d\n",Mod_id,rar->cqi_req);
  LOG_D(MAC,"[UE %d][RAPROC] rar->t_crnti %x\n",Mod_id,rar->t_crnti);
#endif
  if (preamble_index == rarh->RAPID) {
    *t_crnti = rar->t_crnti;
     UE_mac_inst[Mod_id].crnti = rar->t_crnti;
    return(rar->Timing_Advance_Command);
  }
  else {
    UE_mac_inst[Mod_id].crnti=0;
    return(0xffff);
  }
}
