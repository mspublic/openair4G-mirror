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

/*! \file pusch_pc.c
 * \brief Implementation of UE PUSCH Power Control procedures from 36.213 LTE specifications (Section 
 * \author R. Knopp
 * \date 2011
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr
 * \note
 * \warning
 */

#include "PHY/defs.h"
#include "PHY/LTE_TRANSPORT/proto.h"
#include "PHY/extern.h"

s16 get_hundred_times_delta_IF(PHY_VARS_UE *phy_vars_ue,u8 eNB_id) {
 
  if (phy_vars_ue->ul_power_control_dedicated[eNB_id].deltaMCS_Enabled == 1) {
    msg("[PHY] pusch_pc.c: FATAL, deltaMCS is not available yet\n");
  }
  else {
    
    return(0);
  }
}

u8 hundred_times_log10_NPRB[100] = {0,30,47,60,69,77,84,90,95,100,104,107,111,114,117,120,123,125,127,130,132,134,136,138,139,141,143,144,146,147,149,150,151,153,154,155,156,157,159,160,161,162,163,164,165,166,167,168,169,169,170,171,172,173,174,174,175,176,177,177,178,179,179,180,181,181,182,183,183,184,185,185,186,186,187,188,188,189,189,190,190,191,191,192,192,193,193,194,194,195,195,196,196,197,197,198,198,199,199,200};

void pusch_power_cntl(PHY_VARS_UE *phy_vars_ue,u8 subframe,u8 eNB_id,u8 j) {

  u8 harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,subframe);    

  u8 mcs   = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->mcs;
  u8 nb_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb;
  s8 PL;

  
  // P_pusch = 10*log10(nb_rb + P_opusch(j)+ alpha(u)*PL + delta_TF(i) + f(i))
  // 
  // P_opusch(0) = P_oPTR + deltaP_Msg3 if PUSCH is transporting Msg3
  // else
  // P_opusch(0) = PO_NOMINAL_PUSCH(j) + P_O_UE_PUSCH(j)
  PL = get_PL(phy_vars_ue->Mod_id,eNB_id);

  if( phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 1) {  // Msg3 PUSCH
    phy_vars_ue->ulsch_ue[eNB_id]->Po_PUSCH = (100*mac_xface->get_Po_NOMINAL_PUSCH(phy_vars_ue->Mod_id) +
					       hundred_times_log10_NPRB[nb_rb]+
					       100*PL+
					       get_hundred_times_delta_IF(phy_vars_ue,eNB_id) +
					       100*phy_vars_ue->ulsch_ue[eNB_id]->f_pusch)/100; 
    msg("[PHY][UE %d][RARPROC] frame %d, subframe %d: Msg3 Po_PUSCH %d dBm (%d,%d,%d,%d,%d)\n",
	phy_vars_ue->Mod_id,mac_xface->frame,subframe,phy_vars_ue->ulsch_ue[eNB_id]->Po_PUSCH,
	100*mac_xface->get_Po_NOMINAL_PUSCH(phy_vars_ue->Mod_id),
	hundred_times_log10_NPRB[nb_rb],
	100*PL,
	get_hundred_times_delta_IF(phy_vars_ue,eNB_id),
	100*phy_vars_ue->ulsch_ue[eNB_id]->f_pusch);
  }
  else if (j==0) {  // SPS PUSCH

  }
  else if (j==1) {  // Normal PUSCH

  }

}
