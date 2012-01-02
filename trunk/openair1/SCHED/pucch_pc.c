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

s8 pucch_power_cntl(PHY_VARS_UE *phy_vars_ue,u8 subframe,u8 eNB_id,PUCCH_FMT_t pucch_fmt) {

  s8 Po_PUCCH;
  
  // P_pucch =  P_opucch+ PL + h(nCQI,nHARQ) + delta_pucchF(pucch_fmt) + g(i))
  // 

  Po_PUCCH = get_PL(phy_vars_ue->Mod_id,eNB_id)+
             phy_vars_ue->lte_frame_parms.ul_power_control_config_common.p0_NominalPUCCH+
             phy_vars_ue->g_pucch[eNB_id]; 
  switch (pucch_fmt) {
  case pucch_format1:
  case pucch_format2a:
  case pucch_format2b:
    Po_PUCCH += (-2+(phy_vars_ue->lte_frame_parms.ul_power_control_config_common.deltaF_PUCCH_Format1<<1));
    break;
  case pucch_format1a:
  case pucch_format1b:
    Po_PUCCH += (1+(phy_vars_ue->lte_frame_parms.ul_power_control_config_common.deltaF_PUCCH_Format1b<<1));
    break;
  case pucch_format2:
    switch (phy_vars_ue->lte_frame_parms.ul_power_control_config_common.deltaF_PUCCH_Format2a) {
    case 0:
      Po_PUCCH -= 2;
      break;
    case 2:
      Po_PUCCH += 1;
      break;
    case 3:
      Po_PUCCH += 2;
      break;
    case 1:
    default:
      break;
    }
    break;
  }
  if (pucch_fmt!=pucch_format1) {
    msg("[PHY][UE  %d][PDSCH %x] frame %d, subframe %d: Po_PUCCH %d dBm : Po_NOMINAL_PUCCH %d, g_pucch %d\n",
	phy_vars_ue->Mod_id,
	phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,mac_xface->frame,subframe,
	Po_PUCCH,
	phy_vars_ue->lte_frame_parms.ul_power_control_config_common.p0_NominalPUCCH,
	phy_vars_ue->g_pucch[eNB_id]);
  }
  else {
    msg("[PHY][UE  %d][SR %x] frame %d, subframe %d: Po_PUCCH %d dBm : Po_NOMINAL_PUCCH %d, g_pucch %d\n",
	phy_vars_ue->Mod_id,
	phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,mac_xface->frame,subframe,
	Po_PUCCH,
	phy_vars_ue->lte_frame_parms.ul_power_control_config_common.p0_NominalPUCCH,
	phy_vars_ue->g_pucch[eNB_id]);
  }
  return(Po_PUCCH);
}
