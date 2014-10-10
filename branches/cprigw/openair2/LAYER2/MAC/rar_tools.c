/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

*******************************************************************************/

/*! \file rar_tools.c
 * \brief random access tools
 * \author Raymond Knopp and navid nikaein
 * \date 2011 - 2014
 * \version 1.0
 * @ingroup _mac

 */

#include "defs.h"
#include "proto.h"
#include "extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SIMULATION/TOOLS/defs.h"
#include "UTIL/LOG/log.h"
#include "OCG.h"
#include "OCG_extern.h"
#include "UTIL/OPT/opt.h"

#define DEBUG_RAR

extern unsigned int  localRIV2alloc_LUT25[512];
extern unsigned int  distRIV2alloc_LUT25[512];
extern unsigned short RIV2nb_rb_LUT25[512];
extern unsigned short RIV2first_rb_LUT25[512];

extern inline unsigned int taus(void);

unsigned short fill_rar(module_id_t module_idP,
			int CC_id,
                        frame_t frameP,
                        uint8_t *dlsch_buffer,
                        uint16_t N_RB_UL,
                        uint8_t input_buffer_length) {

  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)dlsch_buffer;
  //  RAR_PDU *rar = (RAR_PDU *)(dlsch_buffer+1);
  uint8_t *rar = (uint8_t *)(dlsch_buffer+1);
  int i,ra_idx = -1;
  uint16_t rballoc;
  uint8_t mcs,TPC,ULdelay,cqireq;

  for (i=0;i<NB_RA_PROC_MAX;i++) {
      if (eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[i].generate_rar == 1) {
          ra_idx=i;
          eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[i].generate_rar = 0;
          break;
      }
  }

  // subheader fixed 
  rarh->E                     = 0; // First and last RAR
  rarh->T                     = 1; // 0 for E/T/R/R/BI subheader, 1 for E/T/RAPID subheader
  rarh->RAPID                 = eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].preamble_index; // Respond to Preamble 0 only for the moment
  /*
  rar->R                      = 0;
  rar->Timing_Advance_Command = eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].timing_offset/4;
  rar->hopping_flag           = 0;
  rar->rb_alloc               = mac_xface->computeRIV(N_RB_UL,12,2);  // 2 RB
  rar->mcs                    = 2;                                   // mcs 2
  rar->TPC                    = 4;   // 2 dB power adjustment
  rar->UL_delay               = 0;
  rar->cqi_req                = 1;
  rar->t_crnti                = eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].rnti;
   */
  rar[4] = (uint8_t)(eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].rnti>>8);
  rar[5] = (uint8_t)(eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].rnti&0xff);
  eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].timing_offset = 0;
  //eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].timing_offset /= 16;
  rar[0] = (uint8_t)(eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].timing_offset>>(2+4)); // 7 MSBs of timing advance + divide by 4
  rar[1] = (uint8_t)(eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].timing_offset<<(4-2))&0xf0; // 4 LSBs of timing advance + divide by 4
  rballoc = mac_xface->computeRIV(N_RB_UL,1,1); // first PRB only for UL Grant
  rar[1] |= (rballoc>>7)&7; // Hopping = 0 (bit 3), 3 MSBs of rballoc
  rar[2] = ((uint8_t)(rballoc&0xff))<<1; // 7 LSBs of rballoc
  mcs = 10;
  TPC = 3;
  ULdelay = 0;
  cqireq = 0;
  rar[2] |= ((mcs&0x8)>>3);  // mcs 10
  rar[3] = (((mcs&0x7)<<5)) | ((TPC&7)<<2) | ((ULdelay&1)<<1) | (cqireq&1); 

  LOG_I(MAC,"[eNB %d][RAPROC] Frame %d Generating RAR (%02x|%02x.%02x.%02x.%02x.%02x.%02x) for ra_idx %d, CRNTI %x,preamble %d/%d,TIMING OFFSET %d\n",module_idP,frameP,
      *(uint8_t*)rarh,rar[0],rar[1],rar[2],rar[3],rar[4],rar[5],
      ra_idx,
      eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].rnti,
      rarh->RAPID,eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[0].preamble_index,
      eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].timing_offset);

#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.opt_enabled){
      trace_pdu(1, dlsch_buffer, input_buffer_length, module_idP, 2, 1,
          eNB_mac_inst[module_idP].subframe, 0, 0);
      LOG_I(OPT,"[eNB %d][RAPROC] RAR Frame %d trace pdu for rnti %x and  rapid %d size %d\n",
          module_idP, frameP, eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].rnti,
          rarh->RAPID, input_buffer_length);
  } 
#endif 
  return(eNB_mac_inst[module_idP].common_channels[CC_id].RA_template[ra_idx].rnti);
}

uint16_t ue_process_rar(module_id_t module_idP, int CC_id,frame_t frameP, uint8_t *dlsch_buffer,rnti_t *t_crnti,uint8_t preamble_index) {

  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)dlsch_buffer;
  //  RAR_PDU *rar = (RAR_PDU *)(dlsch_buffer+1);
  uint8_t *rar = (uint8_t *)(dlsch_buffer+1);

  if (CC_id>0) {
    LOG_W(MAC,"Should not have received RAR on secondary CCs! \n");
    return(0xffff);
  }

  LOG_I(MAC,"[eNB %d][RAPROC] Frame %d Received RAR (%02x|%02x.%02x.%02x.%02x.%02x.%02x) for preamble %d/%d\n",module_idP,frameP,
      *(uint8_t*)rarh,rar[0],rar[1],rar[2],rar[3],rar[4],rar[5],
      rarh->RAPID,preamble_index);
#ifdef DEBUG_RAR
  LOG_D(MAC,"[UE %d][RAPROC] rarh->E %d\n",module_idP,rarh->E);
  LOG_D(MAC,"[UE %d][RAPROC] rarh->T %d\n",module_idP,rarh->T);
  LOG_D(MAC,"[UE %d][RAPROC] rarh->RAPID %d\n",module_idP,rarh->RAPID);

  //  LOG_I(MAC,"[UE %d][RAPROC] rar->R %d\n",module_idP,rar->R);
  LOG_I(MAC,"[UE %d][RAPROC] rar->Timing_Advance_Command %d\n",module_idP,(((uint16_t)(rar[0]&0x7f))<<4) + (rar[1]>>4));
  //  LOG_I(MAC,"[UE %d][RAPROC] rar->hopping_flag %d\n",module_idP,rar->hopping_flag);
  //  LOG_I(MAC,"[UE %d][RAPROC] rar->rb_alloc %d\n",module_idP,rar->rb_alloc);
  //  LOG_I(MAC,"[UE %d][RAPROC] rar->mcs %d\n",module_idP,rar->mcs);
  //  LOG_I(MAC,"[UE %d][RAPROC] rar->TPC %d\n",module_idP,rar->TPC);
  //  LOG_I(MAC,"[UE %d][RAPROC] rar->UL_delay %d\n",module_idP,rar->UL_delay);
  //  LOG_I(MAC,"[UE %d][RAPROC] rar->cqi_req %d\n",module_idP,rar->cqi_req);
  LOG_I(MAC,"[UE %d][RAPROC] rar->t_crnti %x\n",module_idP,(uint16_t)rar[5]+(rar[4]<<8));
#endif


  if (preamble_index == rarh->RAPID) {
      *t_crnti = (uint16_t)rar[5]+(rar[4]<<8);//rar->t_crnti;
      UE_mac_inst[module_idP].crnti = *t_crnti;//rar->t_crnti;
      //return(rar->Timing_Advance_Command);
      return((((uint16_t)(rar[0]&0x7f))<<4) + (rar[1]>>4));
  }
  else {
      UE_mac_inst[module_idP].crnti=0;
      return(0xffff);
  }
}
