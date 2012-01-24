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

/*! \file PHY/LTE_TRANSPORT/rar_tools.c
* \brief Routine for filling the PUSCH/ULSCH data structures based on a random-access response (RAR) SDU from MAC.  Note this is both for UE and eNB. V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/extern.h"
#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#endif

extern unsigned int  localRIV2alloc_LUT25[512];
extern unsigned int  distRIV2alloc_LUT25[512];
extern unsigned short RIV2nb_rb_LUT25[512];
extern unsigned short RIV2first_rb_LUT25[512];
extern unsigned short RIV_max;

#define DEBUG_RAR

#ifdef OPENAIR2
int generate_eNB_ulsch_params_from_rar(unsigned char *rar_pdu,
				       u32 frame,
				       unsigned char subframe,
				       LTE_eNB_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms) {

  

  //  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)rar_pdu;
  RAR_PDU *rar = (RAR_PDU *)(rar_pdu+1);
  u8 harq_pid = get_Msg3_harq_pid(frame_parms,frame,subframe);
  
  LOG_D(PHY,"[eNB][RAPROC] generate_eNB_ulsch_params_from_rar: subframe %d (harq_pid %d)\n",subframe,harq_pid);
  
  ulsch->harq_processes[harq_pid]->TPC                = rar->TPC;

  if (rar->rb_alloc>RIV_max) {
    LOG_E(PHY,"[eNB]dci_tools.c: ERROR: rb_alloc > RIV_max\n");
    return(-1);
  }
  ulsch->harq_processes[harq_pid]->rar_alloc          = 1;
  ulsch->harq_processes[harq_pid]->first_rb           = RIV2first_rb_LUT25[rar->rb_alloc];
  ulsch->harq_processes[harq_pid]->nb_rb              = RIV2nb_rb_LUT25[rar->rb_alloc];
  ulsch->harq_processes[harq_pid]->Ndi                = 1;
  ulsch->Or2                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
  ulsch->Or1                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
  ulsch->O_RI                                  = 1;
  ulsch->O_ACK                                 = 2;
  ulsch->beta_offset_cqi_times8                = 18;
  ulsch->beta_offset_ri_times8                 = 10;
  ulsch->beta_offset_harqack_times8            = 16;

  
  ulsch->Nsymb_pusch                           = 12-(frame_parms->Ncp<<1);
  ulsch->rnti = rar->t_crnti;  
  if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
    ulsch->harq_processes[harq_pid]->status = ACTIVE;
    ulsch->harq_processes[harq_pid]->rvidx = 0;
    ulsch->harq_processes[harq_pid]->mcs         = rar->mcs;
    ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb-1];
    ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
    ulsch->harq_processes[harq_pid]->Nsymb_initial = 9;
    ulsch->harq_processes[harq_pid]->round = 0;
  }
  else {
    ulsch->harq_processes[harq_pid]->rvidx = 0;
    ulsch->harq_processes[harq_pid]->round++;
  }
#ifdef DEBUG_RAR
  msg("ulsch ra (eNB): harq_pid %d\n",harq_pid);
  msg("ulsch ra (eNB): NBRB     %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
  msg("ulsch ra (eNB): rballoc  %x\n",ulsch->harq_processes[harq_pid]->first_rb);
  msg("ulsch ra (eNB): harq_pid %d\n",harq_pid);
  msg("ulsch ra (eNB): Ndi      %d\n",ulsch->harq_processes[harq_pid]->Ndi);  
  msg("ulsch ra (eNB): TBS      %d\n",ulsch->harq_processes[harq_pid]->TBS);
  msg("ulsch ra (eNB): mcs      %d\n",ulsch->harq_processes[harq_pid]->mcs);
  msg("ulsch ra (eNB): Or1      %d\n",ulsch->Or1);
#endif
  return(0);
}

s8 delta_PUSCH_msg2[8] = {-6,-4,-2,0,2,4,6,8};

int generate_ue_ulsch_params_from_rar(PHY_VARS_UE *phy_vars_ue,
				      unsigned char eNB_id ){
  
  //  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)rar_pdu;
  u32 frame              = phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id];
  unsigned char *rar_pdu = phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b;
  unsigned char subframe = phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id];
  LTE_UE_ULSCH_t *ulsch  = phy_vars_ue->ulsch_ue[eNB_id];
  PHY_MEASUREMENTS *meas = &phy_vars_ue->PHY_measurements;
  LTE_DL_FRAME_PARMS *frame_parms =  &phy_vars_ue->lte_frame_parms;
  //  int current_dlsch_cqi = phy_vars_ue->current_dlsch_cqi[eNB_id];  

  RAR_PDU *rar = (RAR_PDU *)(rar_pdu+1);
  u8 harq_pid = subframe2harq_pid(frame_parms,phy_vars_ue->frame,subframe);

    
   
#ifdef DEBUG_RAR
  msg("rar_tools.c: Filling ue ulsch params -> ulsch %p : subframe %d\n",ulsch,subframe);
#endif



    ulsch->harq_processes[harq_pid]->TPC                                   = rar->TPC;

    if (rar->rb_alloc>RIV_max) {
      msg("rar_tools.c: ERROR: rb_alloc > RIV_max\n");
      return(-1);
    }

    ulsch->harq_processes[harq_pid]->first_rb                              = RIV2first_rb_LUT25[rar->rb_alloc];
    ulsch->harq_processes[harq_pid]->nb_rb                                 = RIV2nb_rb_LUT25[rar->rb_alloc];
    if (ulsch->harq_processes[harq_pid]->nb_rb ==0)
      return(-1);

    ulsch->power_offset = ue_power_offsets[ulsch->harq_processes[harq_pid]->nb_rb];

    if (ulsch->harq_processes[harq_pid]->nb_rb > 3) {
      msg("rar_tools.c: unlikely rb count for RAR grant : nb_rb > 3\n");
      return(-1);
    }

    ulsch->harq_processes[harq_pid]->Ndi                                   = 1;
    if (ulsch->harq_processes[harq_pid]->Ndi == 1)
      ulsch->harq_processes[harq_pid]->status = ACTIVE;

    ulsch->O_RI                                  = 1;
    if (meas->rank[eNB_id] == 1) {
      ulsch->uci_format                          = wideband_cqi_rank2_2A;
      ulsch->O                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
      ulsch->o_RI[0]                             = 1;
    }
    else {
      ulsch->uci_format                          = wideband_cqi_rank1_2A;
      ulsch->O                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
      ulsch->o_RI[0]                             = 0;
    }

    ulsch->uci_format = HLC_subband_cqi_nopmi;
    fill_CQI(ulsch->o,ulsch->uci_format,meas,eNB_id);
    if (((phy_vars_ue->frame % 100) == 0) || (phy_vars_ue->frame < 10)) 
      print_CQI(ulsch->o,ulsch->uci_format,eNB_id);

    ulsch->O_ACK                                  = 2;

    ulsch->beta_offset_cqi_times8                  = 18;
    ulsch->beta_offset_ri_times8                   = 10;
    ulsch->beta_offset_harqack_times8              = 16;
    
    ulsch->Nsymb_pusch                             = 12-(frame_parms->Ncp<<1);
    ulsch->rnti = rar->t_crnti;  
    if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
      ulsch->harq_processes[harq_pid]->status = ACTIVE;
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->mcs         = rar->mcs;
      ulsch->harq_processes[harq_pid]->TPC         = rar->TPC;
      ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb-1];
      ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
      ulsch->harq_processes[harq_pid]->Nsymb_initial = 9;
      ulsch->harq_processes[harq_pid]->round = 0;
    }
    else {
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->round++;
    }

    // initialize power control based on PRACH power
	ulsch->f_pusch = delta_PUSCH_msg2[ulsch->harq_processes[harq_pid]->TPC] +
	mac_xface->get_deltaP_rampup(phy_vars_ue->Mod_id);
	msg("[PHY][UE %d][PUSCH PC] Initializing f_pusch to %d dB, TPC %d (delta_PUSCH_msg2 %d dB), deltaP_rampup %d dB\n",
	    phy_vars_ue->Mod_id,ulsch->f_pusch,ulsch->harq_processes[harq_pid]->TPC,delta_PUSCH_msg2[ulsch->harq_processes[harq_pid]->TPC],
	    mac_xface->get_deltaP_rampup(phy_vars_ue->Mod_id));
    

#ifdef DEBUG_RAR
    msg("ulsch (ue,ra): NBRB     %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    msg("ulsch (ue,ra): first_rb %x\n",ulsch->harq_processes[harq_pid]->first_rb);
    msg("ulsch (ue,ra): nb_rb    %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    msg("ulsch (ue,ra): Ndi      %d\n",ulsch->harq_processes[harq_pid]->Ndi);  
    msg("ulsch (ue,ra): TBS      %d\n",ulsch->harq_processes[harq_pid]->TBS);
    msg("ulsch (ue,ra): mcs      %d\n",ulsch->harq_processes[harq_pid]->mcs);
#endif
    return(0);
}
#endif

