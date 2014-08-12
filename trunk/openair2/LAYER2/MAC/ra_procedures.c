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

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

*******************************************************************************/

/*! \file ra_procedures.c
 * \brief Routines for UE MAC-layer Random-access procedures (36.321) V8.6 2009-03
 * \author R. Knopp and Navid Nikaein
 * \date 2011
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr navid.nikaein@eurecom.fr
 * \note
 * \warning
 */

#include "extern.h"
#include "defs.h"
#include "proto.h"
#include "UTIL/LOG/vcd_signal_dumper.h"
#include "PHY_INTERFACE/defs.h"
#include "PHY_INTERFACE/extern.h"
#include "COMMON/mac_rrc_primitives.h"
#include "RRC/LITE/extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "UTIL/LOG/log.h"
#include "UTIL/OPT/opt.h"
#include "OCG.h"
#include "OCG_extern.h"
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif

extern inline unsigned int taus(void);

int8_t get_DELTA_PREAMBLE(module_id_t module_idP,int CC_id) {

  if (CC_id>0) {
    LOG_E(MAC,"Transmission on secondary CCs is not supported yet\n");
    mac_xface->macphy_exit("MAC FATAL  CC_id>0");
    return;
  }

  uint8_t prachConfigIndex = UE_mac_inst[module_idP].radioResourceConfigCommon->prach_Config.prach_ConfigInfo.prach_ConfigIndex;
  uint8_t preambleformat;

  if (UE_mac_inst[module_idP].tdd_Config) { // TDD
      if (prachConfigIndex < 20)
        preambleformat = 0;
      else if (prachConfigIndex < 30)
        preambleformat = 1;
      else if (prachConfigIndex < 40)
        preambleformat = 2;
      else if (prachConfigIndex < 48)
        preambleformat = 3;
      else
        preambleformat = 4;
  }
  else { // FDD
      preambleformat = prachConfigIndex>>2;
  }
  switch (preambleformat) {
  case 0:
  case 1:
    return(0);
  case 2:
  case 3:
    return(-3);
  case 4:
    return(8);
  default:
    LOG_E(MAC,"[UE %d] ue_procedures.c: FATAL, Illegal preambleformat %d, prachConfigIndex %d\n",
        module_idP,
        preambleformat,prachConfigIndex);
    mac_xface->macphy_exit("MAC get_DELTA_PREAMBLE Illegal preamble format");
    return(0);
  }

}

/// This routine implements Section 5.1.2 (UE Random Access Resource Selection) from 36.321
void get_prach_resources(module_id_t module_idP,
			 int CC_id,
			 uint8_t eNB_index,
			 uint8_t t_id,
			 uint8_t first_Msg3,
			 RACH_ConfigDedicated_t *rach_ConfigDedicated) {

  uint8_t Msg3_size = UE_mac_inst[module_idP].RA_Msg3_size;
  PRACH_RESOURCES_t *prach_resources = &UE_mac_inst[module_idP].RA_prach_resources;
  RACH_ConfigCommon_t *rach_ConfigCommon = NULL;
  uint8_t noGroupB = 0;
  uint8_t f_id = 0,num_prach=0;

  if (CC_id>0) {
    LOG_E(MAC,"Transmission on secondary CCs is not supported yet\n");
    mac_xface->macphy_exit("MAC FATAL  CC_id>0");
    return;
  }
    
  if (UE_mac_inst[module_idP].radioResourceConfigCommon)
    rach_ConfigCommon = &UE_mac_inst[module_idP].radioResourceConfigCommon->rach_ConfigCommon;
  else {
      LOG_E(MAC,"[UE %d] FATAL  radioResourceConfigCommon is NULL !!!\n",module_idP);
      mac_xface->macphy_exit("MAC FATAL  radioResourceConfigCommon is NULL");
  }

  if (rach_ConfigDedicated) {   // This is for network controlled Mobility, later
      if (rach_ConfigDedicated->ra_PRACH_MaskIndex != 0) {
          prach_resources->ra_PreambleIndex = rach_ConfigDedicated->ra_PreambleIndex;
          prach_resources->ra_RACH_MaskIndex = rach_ConfigDedicated->ra_PRACH_MaskIndex;
          return;
      }
  }

  if (!rach_ConfigCommon->preambleInfo.preamblesGroupAConfig) {
      noGroupB = 1;

  }
  else {
      if (rach_ConfigCommon->preambleInfo.preamblesGroupAConfig->sizeOfRA_PreamblesGroupA ==
          rach_ConfigCommon->preambleInfo.numberOfRA_Preambles)
        noGroupB = 1;
  }

  if (first_Msg3 == 1) {
      if (noGroupB == 1) {
          // use Group A procedure
          UE_mac_inst[module_idP].RA_prach_resources.ra_PreambleIndex  = (taus())&0x3f;
          UE_mac_inst[module_idP].RA_prach_resources.ra_RACH_MaskIndex = 0;
          UE_mac_inst[module_idP].RA_usedGroupA = 1;
      }
      else if ((Msg3_size < rach_ConfigCommon->preambleInfo.preamblesGroupAConfig->messageSizeGroupA) ||
          (mac_xface->get_PL(module_idP,0,eNB_index) > UE_mac_inst[module_idP].RA_maxPL)) {
          // use Group A procedure
          UE_mac_inst[module_idP].RA_prach_resources.ra_PreambleIndex  = (taus())%rach_ConfigCommon->preambleInfo.preamblesGroupAConfig->sizeOfRA_PreamblesGroupA;
          UE_mac_inst[module_idP].RA_prach_resources.ra_RACH_MaskIndex = 0;
          UE_mac_inst[module_idP].RA_usedGroupA = 1;
      }
      else {  // use Group B
          UE_mac_inst[module_idP].RA_prach_resources.ra_PreambleIndex  =
              rach_ConfigCommon->preambleInfo.preamblesGroupAConfig->sizeOfRA_PreamblesGroupA +
              (taus())%(rach_ConfigCommon->preambleInfo.numberOfRA_Preambles -
                  rach_ConfigCommon->preambleInfo.preamblesGroupAConfig->sizeOfRA_PreamblesGroupA);
          UE_mac_inst[module_idP].RA_prach_resources.ra_RACH_MaskIndex = 0;
          UE_mac_inst[module_idP].RA_usedGroupA = 0;
      }
      UE_mac_inst[module_idP].RA_prach_resources.ra_PREAMBLE_RECEIVED_TARGET_POWER = get_Po_NOMINAL_PUSCH(module_idP,CC_id);
  }
  else {  // Msg3 is being retransmitted
      if (UE_mac_inst[module_idP].RA_usedGroupA == 1) {
          if (rach_ConfigCommon->preambleInfo.preamblesGroupAConfig)
            UE_mac_inst[module_idP].RA_prach_resources.ra_PreambleIndex  = (taus())%rach_ConfigCommon->preambleInfo.preamblesGroupAConfig->sizeOfRA_PreamblesGroupA;
          else
            UE_mac_inst[module_idP].RA_prach_resources.ra_PreambleIndex  = (taus())&0x3f;
          UE_mac_inst[module_idP].RA_prach_resources.ra_RACH_MaskIndex = 0;
      }
      else {
          UE_mac_inst[module_idP].RA_prach_resources.ra_PreambleIndex  =
              rach_ConfigCommon->preambleInfo.preamblesGroupAConfig->sizeOfRA_PreamblesGroupA +
              (taus())%(rach_ConfigCommon->preambleInfo.numberOfRA_Preambles -
                  rach_ConfigCommon->preambleInfo.preamblesGroupAConfig->sizeOfRA_PreamblesGroupA);
          UE_mac_inst[module_idP].RA_prach_resources.ra_RACH_MaskIndex = 0;
      }
  }
  // choose random PRACH resource in TDD
  if (UE_mac_inst[module_idP].tdd_Config) {
      num_prach = mac_xface->get_num_prach_tdd(mac_xface->lte_frame_parms);
      if ((num_prach>0) && (num_prach<6))
        UE_mac_inst[module_idP].RA_prach_resources.ra_TDD_map_index = (taus()%num_prach);
      f_id = mac_xface->get_fid_prach_tdd(mac_xface->lte_frame_parms,
          UE_mac_inst[module_idP].RA_prach_resources.ra_TDD_map_index);
  }

  // choose RA-RNTI
  UE_mac_inst[module_idP].RA_prach_resources.ra_RNTI = 1 + t_id + 10*f_id;
}

void Msg1_tx(module_id_t module_idP,int CC_id,frame_t frameP, uint8_t eNB_id) {

  if (CC_id>0) {
    LOG_E(MAC,"Transmission on secondary CCs is not supported yet\n");
    mac_xface->macphy_exit("MAC FATAL  CC_id>0");
    return;
  }

  // start contention resolution timer
  UE_mac_inst[module_idP].RA_attempt_number++;
#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.opt_enabled) {
      trace_pdu(0, NULL, 0, module_idP, 3, UE_mac_inst[module_idP].RA_prach_resources.ra_PreambleIndex,
          UE_mac_inst[module_idP].subframe, 0, UE_mac_inst[module_idP].RA_attempt_number);
      LOG_D(OPT,"[UE %d][RAPROC] TX MSG1 Frame %d trace pdu for rnti %x  with size %d\n",
          module_idP, frameP, 1, UE_mac_inst[module_idP].RA_Msg3_size);
  }
#endif
}


void Msg3_tx(module_id_t module_idP,int CC_id,frame_t frameP, uint8_t eNB_id) {

  if (CC_id>0) {
    LOG_E(MAC,"Transmission on secondary CCs is not supported yet\n");
    mac_xface->macphy_exit("MAC FATAL  CC_id>0");
    return;
  }

  // start contention resolution timer
  LOG_I(MAC,"[UE %d][RAPROC] Frame %d : Msg3_tx: Setting contention resolution timer\n",module_idP,frameP);
  UE_mac_inst[module_idP].RA_contention_resolution_cnt = 0;
  UE_mac_inst[module_idP].RA_contention_resolution_timer_active = 1;

#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.opt_enabled) { // msg3
      trace_pdu(0, &UE_mac_inst[module_idP].CCCH_pdu.payload[0], UE_mac_inst[module_idP].RA_Msg3_size,
          module_idP, 3, UE_mac_inst[module_idP].crnti, UE_mac_inst[module_idP].subframe, 0, 0);
      LOG_D(OPT,"[UE %d][RAPROC] MSG3 Frame %d trace pdu Preamble %d   with size %d\n",
          module_idP, frameP, UE_mac_inst[module_idP].crnti /*UE_mac_inst[module_idP].RA_prach_resources.ra_PreambleIndex*/, UE_mac_inst[module_idP].RA_Msg3_size);
  }
#endif
}


PRACH_RESOURCES_t *ue_get_rach(module_id_t module_idP,int CC_id,frame_t frameP, uint8_t eNB_indexP,sub_frame_t subframeP){


  uint8_t                        Size=0;
  UE_MODE_t                 UE_mode = mac_xface->get_ue_mode(module_idP,0,eNB_indexP);
  uint8_t                        lcid = CCCH;
  uint16_t                       Size16;
  struct RACH_ConfigCommon *rach_ConfigCommon = (struct RACH_ConfigCommon *)NULL;
  int32_t                       frame_diff=0;
  mac_rlc_status_resp_t     rlc_status;
  uint8_t                        dcch_header_len=0;
  uint16_t                       sdu_lengths[8];
  uint8_t                        ulsch_buff[MAX_ULSCH_PAYLOAD_BYTES];

  if (CC_id>0) {
    LOG_E(MAC,"Transmission on secondary CCs is not supported yet\n");
    mac_xface->macphy_exit("MAC FATAL  CC_id>0");
    return;
  }

  if (UE_mode == PRACH) {
      if (UE_mac_inst[module_idP].radioResourceConfigCommon)
        rach_ConfigCommon = &UE_mac_inst[module_idP].radioResourceConfigCommon->rach_ConfigCommon;
      else {
          return(NULL);
      }
      if (Is_rrc_registered == 1) {

          if (UE_mac_inst[module_idP].RA_active == 0) {
	    printf("RA not active\n");
              // check if RRC is ready to initiate the RA procedure
              Size = mac_rrc_data_req(module_idP,
                  frameP,
                  CCCH,1,
                  &UE_mac_inst[module_idP].CCCH_pdu.payload[sizeof(SCH_SUBHEADER_SHORT)+1],0,
                  eNB_indexP,
                  0);
              Size16 = (uint16_t)Size;

              //	LOG_D(MAC,"[UE %d] Frame %d: Requested RRCConnectionRequest, got %d bytes\n",module_idP,frameP,Size);
              LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_DATA_REQ (RRCConnectionRequest eNB %d) --->][MAC_UE][MOD %02d][]\n",
                  frameP, module_idP, eNB_indexP, module_idP);
              LOG_D(MAC,"[UE %d] Frame %d: Requested RRCConnectionRequest, got %d bytes\n",module_idP,frameP,Size);

              if (Size>0) {

                  UE_mac_inst[module_idP].RA_active                        = 1;
                  UE_mac_inst[module_idP].RA_PREAMBLE_TRANSMISSION_COUNTER = 1;
                  UE_mac_inst[module_idP].RA_Msg3_size                     = Size+sizeof(SCH_SUBHEADER_SHORT)+sizeof(SCH_SUBHEADER_SHORT);
                  UE_mac_inst[module_idP].RA_prachMaskIndex                = 0;
                  UE_mac_inst[module_idP].RA_prach_resources.Msg3          = UE_mac_inst[module_idP].CCCH_pdu.payload;
                  UE_mac_inst[module_idP].RA_backoff_cnt                   = 0;  // add the backoff condition here if we have it from a previous RA reponse which failed (i.e. backoff indicator)
                  if (rach_ConfigCommon) {
                      UE_mac_inst[module_idP].RA_window_cnt                    = 2+ rach_ConfigCommon->ra_SupervisionInfo.ra_ResponseWindowSize;
                      if (UE_mac_inst[module_idP].RA_window_cnt == 9)
                        UE_mac_inst[module_idP].RA_window_cnt = 10;  // Note: 9 subframe window doesn't exist, after 8 is 10!
                  }
                  else {
                      LOG_D(MAC,"[UE %d] FATAL Frame %d: rach_ConfigCommon is NULL !!!\n",module_idP,frameP);
                      mac_xface->macphy_exit("MAC rach_ConfigCommon is NULL");
                  }
                  UE_mac_inst[module_idP].RA_tx_frame         = frameP;
                  UE_mac_inst[module_idP].RA_tx_subframe      = subframeP;
                  UE_mac_inst[module_idP].RA_backoff_frame    = frameP;
                  UE_mac_inst[module_idP].RA_backoff_subframe = subframeP;
                  // Fill in preamble and PRACH resource
                  get_prach_resources(module_idP,CC_id,eNB_indexP,subframeP,1,NULL);

                  generate_ulsch_header((uint8_t*)&UE_mac_inst[module_idP].CCCH_pdu.payload[0],  // mac header
                      1,      // num sdus
                      0,            // short pading
                      &Size16,  // sdu length
                      &lcid,    // sdu lcid
                      NULL,  // power headroom
                      NULL,  // crnti
                      NULL,  // truncated bsr
                      NULL, // short bsr
                      NULL, // long_bsr
                      1); //post_padding

                  return(&UE_mac_inst[module_idP].RA_prach_resources);
              }
              else if (UE_mac_inst[module_idP].scheduling_info.BSR_bytes[DCCH] > 0) {
                  // This is for triggering a transmission on DCCH using PRACH (during handover, or sending SR for example)
                  dcch_header_len = 2 + 2;  /// SHORT Subheader + C-RNTI control element
                  rlc_status = mac_rlc_status_ind(0, module_idP,frameP,ENB_FLAG_NO,MBMS_FLAG_NO,
                      DCCH,
                      6);
                  if (UE_mac_inst[module_idP].crnti_before_ho)
                    LOG_D(MAC,"[UE %d] Frame %d : UL-DCCH -> ULSCH, HO RRCConnectionReconfigurationComplete (%x, %x), RRC message has %d bytes to send throug PRACH (mac header len %d)\n",
                        module_idP,frameP, UE_mac_inst[module_idP].crnti,UE_mac_inst[module_idP].crnti_before_ho, rlc_status.bytes_in_buffer,dcch_header_len);
                  else
                    LOG_D(MAC,"[UE %d] Frame %d : UL-DCCH -> ULSCH, RRC message has %d bytes to send through PRACH(mac header len %d)\n",
                        module_idP,frameP, rlc_status.bytes_in_buffer,dcch_header_len);

                  sdu_lengths[0] = mac_rlc_data_req(eNB_indexP, module_idP,frameP,ENB_FLAG_NO, MBMS_FLAG_NO,
                      DCCH,
                      (char *)&ulsch_buff[0]);

                  LOG_D(MAC,"[UE %d] TX Got %d bytes for DCCH\n",module_idP,sdu_lengths[0]);
                  update_bsr(module_idP, frameP, DCCH,UE_mac_inst[module_idP].scheduling_info.LCGID[DCCH]);
                  //header_len +=2;
                  UE_mac_inst[module_idP].RA_active                        = 1;
                  UE_mac_inst[module_idP].RA_PREAMBLE_TRANSMISSION_COUNTER = 1;
                  UE_mac_inst[module_idP].RA_Msg3_size                     = Size+dcch_header_len;
                  UE_mac_inst[module_idP].RA_prachMaskIndex                = 0;
                  UE_mac_inst[module_idP].RA_prach_resources.Msg3          = ulsch_buff;
                  UE_mac_inst[module_idP].RA_backoff_cnt                   = 0;  // add the backoff condition here if we have it from a previous RA reponse which failed (i.e. backoff indicator)
                  if (rach_ConfigCommon) {
                      UE_mac_inst[module_idP].RA_window_cnt                    = 2+ rach_ConfigCommon->ra_SupervisionInfo.ra_ResponseWindowSize;
                      if (UE_mac_inst[module_idP].RA_window_cnt == 9)
                        UE_mac_inst[module_idP].RA_window_cnt = 10;  // Note: 9 subframe window doesn't exist, after 8 is 10!
                  }
                  else {
                      LOG_D(MAC,"[UE %d] FATAL Frame %d: rach_ConfigCommon is NULL !!!\n",module_idP,frameP);
                      mac_xface->macphy_exit("MAC rach_ConfigCommon is NULL");
                  }
                  UE_mac_inst[module_idP].RA_tx_frame         = frameP;
                  UE_mac_inst[module_idP].RA_tx_subframe      = subframeP;
                  UE_mac_inst[module_idP].RA_backoff_frame    = frameP;
                  UE_mac_inst[module_idP].RA_backoff_subframe = subframeP;
                  // Fill in preamble and PRACH resource
                  get_prach_resources(module_idP,CC_id,eNB_indexP,subframeP,1,NULL);
                  generate_ulsch_header((uint8_t*)ulsch_buff,  // mac header
                      1,      // num sdus
                      0,            // short pading
                      &Size16,  // sdu length
                      &lcid,    // sdu lcid
                      NULL,  // power headroom
                      &UE_mac_inst[module_idP].crnti,  // crnti
                      NULL,  // truncated bsr
                      NULL, // short bsr
                      NULL, // long_bsr
                      0); //post_padding

                  return(&UE_mac_inst[module_idP].RA_prach_resources);
              }
          }
          else {  // RACH is active
              LOG_D(MAC,"[MAC][UE %d][RAPROC] frameP %d, subframe %d: RA Active, window cnt %d (RA_tx_frame %d, RA_tx_subframe %d)\n",module_idP,
                  frameP,subframeP,UE_mac_inst[module_idP].RA_window_cnt,
                  UE_mac_inst[module_idP].RA_tx_frame,UE_mac_inst[module_idP].RA_tx_subframe);
              // compute backoff parameters
              if (UE_mac_inst[module_idP].RA_backoff_cnt>0) {
                  frame_diff = (sframe_t)frameP - UE_mac_inst[module_idP].RA_backoff_frame;
                  if (frame_diff < 0)
                    frame_diff = -frame_diff;
                  UE_mac_inst[module_idP].RA_backoff_cnt -= ((10*frame_diff) + (subframeP-UE_mac_inst[module_idP].RA_backoff_subframe));

                  UE_mac_inst[module_idP].RA_backoff_frame    = frameP;
                  UE_mac_inst[module_idP].RA_backoff_subframe = subframeP;
              }
              // compute RA window parameters
              if (UE_mac_inst[module_idP].RA_window_cnt>0) {
                  frame_diff = (frame_t)frameP - UE_mac_inst[module_idP].RA_tx_frame;
                  if (frame_diff < 0)
                    frame_diff = -frame_diff;
                  UE_mac_inst[module_idP].RA_window_cnt -= ((10*frame_diff) + (subframeP-UE_mac_inst[module_idP].RA_tx_subframe));
                  LOG_D(MAC,"[MAC][UE %d][RAPROC] frameP %d, subframe %d: RA Active, adjusted window cnt %d\n",module_idP,
                      frameP,subframeP,UE_mac_inst[module_idP].RA_window_cnt);
              }
              if ((UE_mac_inst[module_idP].RA_window_cnt<=0) &&
                  (UE_mac_inst[module_idP].RA_backoff_cnt<=0)) {

                  UE_mac_inst[module_idP].RA_tx_frame    = frameP;
                  UE_mac_inst[module_idP].RA_tx_subframe = subframeP;
                  UE_mac_inst[module_idP].RA_PREAMBLE_TRANSMISSION_COUNTER++;
                  UE_mac_inst[module_idP].RA_prach_resources.ra_PREAMBLE_RECEIVED_TARGET_POWER += (rach_ConfigCommon->powerRampingParameters.powerRampingStep<<1);  // 2dB increments in ASN.1 definition
                  if (UE_mac_inst[module_idP].RA_PREAMBLE_TRANSMISSION_COUNTER == rach_ConfigCommon->ra_SupervisionInfo.preambleTransMax) {
                      LOG_D(MAC,"[UE %d] Frame %d: Maximum number of RACH attempts (%d)\n",module_idP,frameP,rach_ConfigCommon->ra_SupervisionInfo.preambleTransMax);
                      // send message to RRC
                      UE_mac_inst[module_idP].RA_PREAMBLE_TRANSMISSION_COUNTER=1;
                      UE_mac_inst[module_idP].RA_prach_resources.ra_PREAMBLE_RECEIVED_TARGET_POWER = get_Po_NOMINAL_PUSCH(module_idP,CC_id);
                  }
                  UE_mac_inst[module_idP].RA_window_cnt                    = 2+ rach_ConfigCommon->ra_SupervisionInfo.ra_ResponseWindowSize;
                  UE_mac_inst[module_idP].RA_backoff_cnt                   = 0;

                  // Fill in preamble and PRACH resource
                  get_prach_resources(module_idP,CC_id,eNB_indexP,subframeP,0,NULL);
                  return(&UE_mac_inst[module_idP].RA_prach_resources);
              }
          }
      }
  }
  else if (UE_mode == PUSCH) {
      LOG_D(MAC,"[UE %d] FATAL: Should not have checked for RACH in PUSCH yet ...",module_idP);
      mac_xface->macphy_exit("MAC FATAL: Should not have checked for RACH in PUSCH yet");
  }
  return(NULL);
}
