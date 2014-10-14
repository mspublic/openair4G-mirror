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

/*! \file eNB_scheduler_mch.c
 * \brief procedures related to eNB for the MCH transport channel
 * \author  Navid Nikaein and Raymond Knopp
 * \date 2012 - 2014
 * \email: navid.nikaein@eurecom.fr
 * \version 1.0
 * @ingroup _mac

 */

#include "assertions.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/proto.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"
#include "UTIL/LOG/vcd_signal_dumper.h"
#include "UTIL/OPT/opt.h"
#include "OCG.h"
#include "OCG_extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"

#include "RRC/LITE/extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"

//#include "LAYER2/MAC/pre_processor.c"
#include "pdcp.h"

#if defined(ENABLE_ITTI)
# include "intertask_interface.h"
#endif

#define ENABLE_MAC_PAYLOAD_DEBUG
#define DEBUG_eNB_SCHEDULER 1

 extern inline unsigned int taus(void);

#ifdef Rel10
int8_t get_mbsfn_sf_alloction (module_id_t module_idP, uint8_t CC_id, uint8_t mbsfn_sync_area){
  // currently there is one-to-one mapping between sf allocation pattern and sync area
  if (mbsfn_sync_area > MAX_MBSFN_AREA){
      LOG_W(MAC,"[eNB %d] MBSFN synchronization area %d out of range\n ", module_idP, mbsfn_sync_area);
      return -1;
  }
  else if (eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[mbsfn_sync_area] != NULL)
    return mbsfn_sync_area;
  else {
      LOG_W(MAC,"[eNB %d] MBSFN Subframe Config pattern %d not found \n ", module_idP, mbsfn_sync_area);
      return -1;
  }
}

int schedule_MBMS(module_id_t module_idP, uint8_t CC_id, frame_t frameP, sub_frame_t subframeP) {

  int mcch_flag=0,mtch_flag=0, msi_flag=0;
  int mbsfn_period =0;// 1<<(eNB_mac_inst[module_idP].mbsfn_SubframeConfig[0]->radioframeAllocationPeriod);
  int mcch_period = 0;//32<<(eNB_mac_inst[module_idP].mbsfn_AreaInfo[0]->mcch_Config_r9.mcch_RepetitionPeriod_r9);
  int mch_scheduling_period = 8<<(eNB_mac_inst[module_idP].common_channels[CC_id].pmch_Config[0]->mch_SchedulingPeriod_r9);
  unsigned char mcch_sdu_length;
  unsigned char header_len_mcch=0,header_len_msi=0,header_len_mtch=0, header_len_mtch_temp=0, header_len_mcch_temp=0, header_len_msi_temp=0;
  int ii=0, msi_pos=0;
  int mcch_mcs = -1;
  uint16_t TBS,j,padding=0,post_padding=0;
  mac_rlc_status_resp_t rlc_status;
  int num_mtch;
  int msi_length,i,k;
  unsigned char sdu_lcids[11], num_sdus=0, offset=0;
  uint16_t sdu_lengths[11], sdu_length_total=0;
  unsigned char mch_buffer[MAX_DLSCH_PAYLOAD_BYTES]; // check the max value, this is for dlsch only

  eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.Pdu_size=0;

  for (i=0;
      i< eNB_mac_inst[module_idP].common_channels[CC_id].num_active_mbsfn_area;
      i++ ){
      // assume, that there is always a mapping
    if ((j=get_mbsfn_sf_alloction(module_idP,CC_id,i)) == -1)
        return 0;

      mbsfn_period = 1<<(eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->radioframeAllocationPeriod);
      mcch_period = 32<<(eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_RepetitionPeriod_r9);
      msi_pos=0; ii=0;
      LOG_D(MAC,"[eNB %d] Frame %d subframeP %d : Checking MBSFN Sync Area %d/%d with SF allocation %d/%d for MCCH and MTCH (mbsfn period %d, mcch period %d)\n",
          module_idP,frameP, subframeP,i,eNB_mac_inst[module_idP].common_channels[CC_id].num_active_mbsfn_area,
          j,eNB_mac_inst[module_idP].common_channels[CC_id].num_sf_allocation_pattern,mbsfn_period,mcch_period);


      switch (eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.signallingMCS_r9) {
      case 0:
        mcch_mcs = 2;
        break;
      case 1:
        mcch_mcs = 7;
        break;
      case 2:
        mcch_mcs = 13;
        break;
      case 3:
        mcch_mcs = 19;
        break;
      }

      // 1st: Check the MBSFN subframes from SIB2 info (SF allocation pattern i, max 8 non-overlapping patterns exist)
      if (frameP %  mbsfn_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->radioframeAllocationOffset){ // MBSFN frameP
          if (eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.present == MBSFN_SubframeConfig__subframeAllocation_PR_oneFrame){// one-frameP format

              //  Find the first subframeP in this MCH to transmit MSI
              if (frameP % mch_scheduling_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->radioframeAllocationOffset ) {
                  while (ii == 0) {
                      ii = eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & (0x80>>msi_pos);
                      msi_pos++;
                  }
                  LOG_D(MAC,"[eNB %d] Frame %d subframeP %d : sync area %d sf allocation pattern %d sf alloc %x msi pos is %d \n", module_idP,frameP, subframeP,i,j,eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0], msi_pos);
              }

              // Check if the subframeP is for MSI, MCCH or MTCHs and Set the correspoding flag to 1
              switch (subframeP) {
              case 1:
                if (mac_xface->lte_frame_parms->frame_type == FDD) {
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_FDD_SF1) == MBSFN_FDD_SF1) {
                        if (msi_pos == 1)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_FDD_SF1) == MBSFN_FDD_SF1) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                break;

              case 2:
                if (mac_xface->lte_frame_parms->frame_type == FDD){
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_FDD_SF2) == MBSFN_FDD_SF2) {
                        if (msi_pos == 2)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_FDD_SF2) == MBSFN_FDD_SF2) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                break;

              case 3:
                if (mac_xface->lte_frame_parms->frame_type == TDD){// TDD
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_TDD_SF3) == MBSFN_TDD_SF3) {
                        if (msi_pos == 1)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_TDD_SF3) == MBSFN_TDD_SF3) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                else {// FDD
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_FDD_SF3) == MBSFN_FDD_SF3) {
                        if (msi_pos == 3)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_FDD_SF3) == MBSFN_FDD_SF3) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                break;

              case 4:
                if (mac_xface->lte_frame_parms->frame_type == TDD){
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_TDD_SF4) == MBSFN_TDD_SF4) {
                        if (msi_pos == 2)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_TDD_SF4) == MBSFN_TDD_SF4) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                break;

              case 6:
                if (mac_xface->lte_frame_parms->frame_type == FDD){
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_FDD_SF6) == MBSFN_FDD_SF6) {
                        if (msi_pos == 4)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_FDD_SF6) == MBSFN_FDD_SF6) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                break;

              case 7:
                if (mac_xface->lte_frame_parms->frame_type == TDD){ // TDD
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_TDD_SF7) == MBSFN_TDD_SF7) {
                        if (msi_pos == 3)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_TDD_SF7) == MBSFN_TDD_SF7) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                else {// FDD
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_FDD_SF7) == MBSFN_FDD_SF7) {
                        if (msi_pos == 5)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_FDD_SF7) == MBSFN_FDD_SF7) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                break;

              case 8:
                if (mac_xface->lte_frame_parms->frame_type == TDD){ //TDD
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_TDD_SF8) == MBSFN_TDD_SF8) {
                        if (msi_pos == 4)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_TDD_SF8) == MBSFN_TDD_SF8) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                else{ // FDD
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_FDD_SF8) == MBSFN_FDD_SF8) {
                        if (msi_pos == 6)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_FDD_SF8) == MBSFN_FDD_SF8) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                break;

              case 9:
                if (mac_xface->lte_frame_parms->frame_type == TDD){
                    if ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_SubframeConfig[j]->subframeAllocation.choice.oneFrame.buf[0] & MBSFN_TDD_SF9) == MBSFN_TDD_SF9) {
                        if (msi_pos == 5)
                          msi_flag = 1;
                        if ( (frameP % mcch_period == eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_Offset_r9) &&
                            ((eNB_mac_inst[module_idP].common_channels[CC_id].mbsfn_AreaInfo[i]->mcch_Config_r9.sf_AllocInfo_r9.buf[0] & MBSFN_TDD_SF9) == MBSFN_TDD_SF9) )
                          mcch_flag = 1;
                        mtch_flag = 1;
                    }
                }
                break;
              }// end switch
              // sf allocation is non-overlapping
              if ((msi_flag==1) || (mcch_flag==1) || (mtch_flag==1)){
                  LOG_D(MAC,"[eNB %d] Frame %d Subframe %d: sync area %d SF alloc %d: msi flag %d, mcch flag %d, mtch flag %d\n",
                      module_idP, frameP, subframeP,i,j,msi_flag,mcch_flag,mtch_flag);
                  break;
              }
          }
          else {// four-frameP format
          }
      }
  } // end of for loop
  eNB_mac_inst[module_idP].common_channels[CC_id].msi_active=0;
  eNB_mac_inst[module_idP].common_channels[CC_id].mcch_active=0;
  eNB_mac_inst[module_idP].common_channels[CC_id].mtch_active=0;
  // Calculate the mcs
  if ((msi_flag==1) || (mcch_flag==1)) {
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mcs = mcch_mcs;
  }
  else if (mtch_flag == 1) { // only MTCH in this subframeP
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mcs = eNB_mac_inst[module_idP].common_channels[CC_id].pmch_Config[0]->dataMCS_r9;
  }


  // 2nd: Create MSI, get MCCH from RRC and MTCHs from RLC

  // there is MSI (MCH Scheduling Info)
  if (msi_flag == 1) {
      // Create MSI here
      uint16_t msi_control_element[29], *msi_ptr;

      msi_ptr = &msi_control_element[0];
      ((MSI_ELEMENT *) msi_ptr)->lcid = MCCH_LCHANID; //MCCH
      if (mcch_flag==1) {
          ((MSI_ELEMENT *) msi_ptr)->stop_sf_MSB = 0;
          ((MSI_ELEMENT *) msi_ptr)->stop_sf_LSB = 0;
      }
      else {                    // no mcch for this MSP
          ((MSI_ELEMENT *) msi_ptr)->stop_sf_MSB = 0x7;// stop value is 2047
          ((MSI_ELEMENT *) msi_ptr)->stop_sf_LSB = 0xff;
      }
      msi_ptr+= sizeof(MSI_ELEMENT);

      //Header for MTCHs
      num_mtch = eNB_mac_inst[module_idP].common_channels[CC_id].mbms_SessionList[0]->list.count;
      for (k=0;k<num_mtch;k++) { // loop for all session in this MCH (MCH[0]) at this moment
          ((MSI_ELEMENT *) msi_ptr)->lcid = eNB_mac_inst[module_idP].common_channels[CC_id].mbms_SessionList[0]->list.array[k]->logicalChannelIdentity_r9;//mtch_lcid;
          ((MSI_ELEMENT *) msi_ptr)->stop_sf_MSB = 0; // last subframeP of this mtch (only one mtch now)
          ((MSI_ELEMENT *) msi_ptr)->stop_sf_LSB = 0xB;
          msi_ptr+=sizeof(MSI_ELEMENT);
      }
      msi_length = msi_ptr-msi_control_element;
      if (msi_length<128)
        header_len_msi = 2;
      else
        header_len_msi = 3;

      LOG_D(MAC,"[eNB %d] Frame %d : MSI->MCH, length of MSI is %d bytes \n",module_idP,frameP,msi_length);
      //LOG_D(MAC,"Scheduler: MSI is transmitted in this subframeP \n" );

      //   LOG_D(MAC,"Scheduler: MSI length is %d bytes\n",msi_length);
      // Store MSI data to mch_buffer[0]
      memcpy((char *)&mch_buffer[sdu_length_total],
          msi_control_element,
          msi_length);

      sdu_lcids[num_sdus] = MCH_SCHDL_INFO;
      sdu_lengths[num_sdus] = msi_length;
      sdu_length_total += sdu_lengths[num_sdus];
      LOG_I(MAC,"[eNB %d] Create %d bytes for MSI\n",module_idP,sdu_lengths[num_sdus]);
      num_sdus++;
      eNB_mac_inst[module_idP].common_channels[CC_id].msi_active=1;
  }

  // there is MCCH
  if (mcch_flag == 1) {
      LOG_D(MAC,"[eNB %d] Frame %d Subframe %d: Schedule MCCH MESSAGE (area %d, sfAlloc %d)\n",
          module_idP,frameP, subframeP, i, j);

      mcch_sdu_length = mac_rrc_data_req(module_idP,
					 CC_id,
					 frameP,
					 MCCH,1,
					 &eNB_mac_inst[module_idP].common_channels[CC_id].MCCH_pdu.payload[0],
					 1,// this is eNB
					 module_idP, // index
					 i); // this is the mbsfn sync area index

      if (mcch_sdu_length > 0) {
          LOG_D(MAC,"[eNB %d] Frame %d subframeP %d : MCCH->MCH, Received %d bytes from RRC \n",module_idP,frameP,subframeP,mcch_sdu_length);

          header_len_mcch = 2;
          if (mac_xface->lte_frame_parms->frame_type == TDD) {
              LOG_D(MAC,"[eNB %d] Frame %d subframeP %d: Scheduling MCCH->MCH (TDD) for MCCH message %d bytes (mcs %d )\n",
                  module_idP,
                  frameP,subframeP,
                  mcch_sdu_length,
                  mcch_mcs);
          }
          else {
              LOG_I(MAC,"[eNB %d] Frame %d subframeP %d: Scheduling MCCH->MCH (FDD) for MCCH message %d bytes (mcs %d)\n",
                  module_idP,
                  frameP, subframeP,
                  mcch_sdu_length,
                  mcch_mcs);
          }
          eNB_mac_inst[module_idP].common_channels[CC_id].mcch_active=1;

          memcpy((char *)&mch_buffer[sdu_length_total],
              &eNB_mac_inst[module_idP].common_channels[CC_id].MCCH_pdu.payload[0],
              mcch_sdu_length);
          sdu_lcids[num_sdus] = MCCH_LCHANID;
          sdu_lengths[num_sdus] = mcch_sdu_length;
          if (sdu_lengths[num_sdus]>128)
            header_len_mcch = 3;
          sdu_length_total += sdu_lengths[num_sdus];
          LOG_D(MAC,"[eNB %d] Got %d bytes for MCCH from RRC \n",module_idP,sdu_lengths[num_sdus]);
          num_sdus++;
      }
  }
  TBS = mac_xface->get_TBS_DL(eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mcs, mac_xface->lte_frame_parms->N_RB_DL);
#ifdef Rel10
  // do not let mcch and mtch multiplexing when relaying is active
  // for sync area 1, so not transmit data
  //if ((i == 0) && ((eNB_mac_inst[module_idP].MBMS_flag != multicast_relay) || (eNB_mac_inst[module_idP].mcch_active==0))) {
#endif
  // there is MTCHs, loop if there are more than 1
  if (mtch_flag == 1) {
      // Calculate TBS
      /* if ((msi_flag==1) || (mcch_flag==1)) {
       TBS = mac_xface->get_TBS(mcch_mcs, mac_xface->lte_frame_parms->N_RB_DL);
       }
       else { // only MTCH in this subframeP
       TBS = mac_xface->get_TBS(eNB_mac_inst[module_idP].pmch_Config[0]->dataMCS_r9, mac_xface->lte_frame_parms->N_RB_DL);
       }

    // get MTCH data from RLC (like for DTCH)
    LOG_D(MAC,"[eNB %d] Frame %d subframe %d: Schedule MTCH (area %d, sfAlloc %d)\n",Mod_id,frame,subframe,i,j);

    header_len_mtch = 3;
	  LOG_D(MAC,"[eNB %d], Frame %d, MTCH->MCH, Checking RLC status (rab %d, tbs %d, len %d)\n",
	  Mod_id,frame,MTCH,TBS,
	  TBS-header_len_mcch-header_len_msi-sdu_length_total-header_len_mtch);

    rlc_status = mac_rlc_status_ind(Mod_id,frame,1,RLC_MBMS_YES,MTCH+ (maxDRB + 3) * MAX_MOBILES_PER_RG,
				    TBS-header_len_mcch-header_len_msi-sdu_length_total-header_len_mtch);
    printf("frame %d, subframe %d,  rlc_status.bytes_in_buffer is %d\n",frame,subframe, rlc_status.bytes_in_buffer);

       */

      // get MTCH data from RLC (like for DTCH)
      LOG_D(MAC,"[eNB %d] Frame %d subframeP %d: Schedule MTCH (area %d, sfAlloc %d)\n",module_idP,frameP,subframeP,i,j);

      header_len_mtch = 3;
      LOG_D(MAC,"[eNB %d], Frame %d, MTCH->MCH, Checking RLC status (rab %d, tbs %d, len %d)\n",
          module_idP,frameP,MTCH,TBS,
          TBS-header_len_mcch-header_len_msi-sdu_length_total-header_len_mtch);

      rlc_status = mac_rlc_status_ind(module_idP,0,frameP,ENB_FLAG_YES,MBMS_FLAG_YES,MTCH,
          TBS-header_len_mcch-header_len_msi-sdu_length_total-header_len_mtch);
      LOG_D(MAC,"e-MBMS log channel %u frameP %d, subframeP %d,  rlc_status.bytes_in_buffer is %d\n",
            MTCH,frameP,subframeP, rlc_status.bytes_in_buffer);

      if (rlc_status.bytes_in_buffer >0) {
          LOG_I(MAC,"[eNB %d][MBMS USER-PLANE], Frame %d, MTCH->MCH, Requesting %d bytes from RLC (header len mtch %d)\n",
              module_idP,frameP,TBS-header_len_mcch-header_len_msi-sdu_length_total-header_len_mtch,header_len_mtch);

          sdu_lengths[num_sdus] = mac_rlc_data_req(
              module_idP,
              0,
              frameP,
              ENB_FLAG_YES,
              MBMS_FLAG_YES,
              MTCH,
              (char*)&mch_buffer[sdu_length_total]);
          //sdu_lengths[num_sdus] = mac_rlc_data_req(module_idP,frameP, MBMS_FLAG_NO,  MTCH+(MAX_NUM_RB*(NUMBER_OF_UE_MAX+1)), (char*)&mch_buffer[sdu_length_total]);
          LOG_I(MAC,"[eNB %d][MBMS USER-PLANE] Got %d bytes for MTCH %d\n",module_idP,sdu_lengths[num_sdus],MTCH);
          eNB_mac_inst[module_idP].common_channels[CC_id].mtch_active=1;
          sdu_lcids[num_sdus] = MTCH;
          sdu_length_total += sdu_lengths[num_sdus];
          if (sdu_lengths[num_sdus] < 128)
            header_len_mtch = 2;
          num_sdus++;
      }
      else {
          header_len_mtch = 0;
      }
  }
#ifdef Rel10
  //  }
#endif
  // FINAL STEP: Prepare and multiplexe MSI, MCCH and MTCHs
  if ((sdu_length_total + header_len_msi + header_len_mcch + header_len_mtch) >0) {
      // Adjust the last subheader
      /*                                 if ((msi_flag==1) || (mcch_flag==1)) {
                                         eNB_mac_inst[module_idP].MCH_pdu.mcs = mcch_mcs;
                                          }
                                        else if (mtch_flag == 1) { // only MTCH in this subframeP
                                       eNB_mac_inst[module_idP].MCH_pdu.mcs = eNB_mac_inst[module_idP].pmch_Config[0]->dataMCS_r9;
                                          }
       */
      header_len_mtch_temp = header_len_mtch;
      header_len_mcch_temp = header_len_mcch;
      header_len_msi_temp = header_len_msi;
      if (header_len_mtch>0)
        header_len_mtch=1;         // remove Length field in the  subheader for the last PDU
      else if (header_len_mcch>0)
        header_len_mcch=1;
      else header_len_msi=1;
      // Calculate the padding
      if ((TBS - header_len_mtch - header_len_mcch - header_len_msi - sdu_length_total) < 0) {
	LOG_E(MAC,"Error in building MAC PDU, TBS %d < PDU %d \n", 
	      TBS, header_len_mtch + header_len_mcch + header_len_msi + sdu_length_total);
	return;
      }else if ((TBS - header_len_mtch - header_len_mcch - header_len_msi - sdu_length_total) <= 2) {
          padding = (TBS - header_len_mtch - header_len_mcch - header_len_msi - sdu_length_total);
          post_padding = 0;
      }
      else {// using post_padding, give back the Length field of subheader  for the last PDU
          padding = 0;
          if (header_len_mtch>0)
            header_len_mtch = header_len_mtch_temp;
          else if (header_len_mcch>0)
            header_len_mcch = header_len_mcch_temp;
          else header_len_msi = header_len_msi_temp;
          post_padding = TBS - sdu_length_total - header_len_msi - header_len_mcch - header_len_mtch;
      }
      // Generate the MAC Header for MCH
      // here we use the function for DLSCH because DLSCH & MCH have the same Header structure
      offset = generate_dlsch_header((unsigned char*)eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.payload,
          num_sdus,
          sdu_lengths,
          sdu_lcids,
          255,    // no drx
          0,  // no timing advance
          NULL,  // no contention res id
          padding,
          post_padding);

      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.Pdu_size=TBS;
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.sync_area=i;
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.msi_active= eNB_mac_inst[module_idP].common_channels[CC_id].msi_active;
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mcch_active= eNB_mac_inst[module_idP].common_channels[CC_id].mcch_active;
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mtch_active= eNB_mac_inst[module_idP].common_channels[CC_id].mtch_active;
      LOG_D(MAC," MCS for this sf is %d (mcch active %d, mtch active %d)\n", eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mcs,
          eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mcch_active,eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mtch_active );
      LOG_I(MAC,"[eNB %d][MBMS USER-PLANE ] Generate header : sdu_length_total %d, num_sdus %d, sdu_lengths[0] %d, sdu_lcids[0] %d => payload offset %d,padding %d,post_padding %d (mcs %d, TBS %d), header MTCH %d, header MCCH %d, header MSI %d\n",
          module_idP,sdu_length_total,num_sdus,sdu_lengths[0],sdu_lcids[0],offset,padding,post_padding,eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mcs,TBS,header_len_mtch, header_len_mcch, header_len_msi);
      // copy SDU to mch_pdu after the MAC Header
      memcpy(&eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.payload[offset],mch_buffer,sdu_length_total);
      // filling remainder of MCH with random data if necessery
      for (j=0;j<(TBS-sdu_length_total-offset);j++)
        eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.payload[offset+sdu_length_total+j] = (char)(taus()&0xff);

#if defined(USER_MODE) && defined(OAI_EMU)
      /* Tracing of PDU is done on UE side */
      if (oai_emulation.info.opt_enabled)
        trace_pdu(1, (uint8_t *)eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.payload,
            TBS, module_idP, 6, 0xffff,  // M_RNTI = 6 in wirehsark
            eNB_mac_inst[module_idP].subframe,0,0);
      LOG_D(OPT,"[eNB %d][MCH] Frame %d : MAC PDU with size %d\n",
          module_idP, frameP, TBS);
#endif
      /*
   for (j=0;j<sdu_length_total;j++)
      printf("%2x.",eNB_mac_inst[module_idP].MCH_pdu.payload[j+offset]);
      printf(" \n");*/
      return 1;
  }
  else {
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.Pdu_size=0;
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.sync_area=0;
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.msi_active=0;
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mcch_active=0;
      eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu.mtch_active=0;
      // for testing purpose, fill with random data
      //for (j=0;j<(TBS-sdu_length_total-offset);j++)
      //  eNB_mac_inst[module_idP].MCH_pdu.payload[offset+sdu_length_total+j] = (char)(taus()&0xff);
      return 0;
  }
  //this is for testing
  /*
  if (mtch_flag == 1) {
  //  LOG_D(MAC,"DUY: mch_buffer length so far is : %ld\n", &mch_buffer[sdu_length_total]-&mch_buffer[0]);
  return 1;
  }
  else
  return 0;
   */
}
MCH_PDU *get_mch_sdu(uint8_t module_idP, uint8_t CC_id, uint32_t frameP, sub_frame_t subframeP) {
  //  eNB_mac_inst[module_idP].MCH_pdu.mcs=0;
  //LOG_D(MAC," MCH_pdu.mcs is %d\n", eNB_mac_inst[module_idP].MCH_pdu.mcs);
#warning "MCH pdu should take the CC_id index"
  return(&eNB_mac_inst[module_idP].common_channels[CC_id].MCH_pdu);
}

#endif
