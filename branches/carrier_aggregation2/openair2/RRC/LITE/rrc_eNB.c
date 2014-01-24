/*******************************************************************************

  Eurecom OpenAirInterface 2
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


/*! \file rrc_eNB.c
* \brief rrc procedures for eNB
* \author Raymond Knopp and Navid Nikaein
* \date 2011
* \version 1.0
* \company Eurecom
* \email: raymond.knopp@eurecom.fr and navid.nikaein@eurecom.fr
*/

#include "defs.h"
#include "extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "UTIL/LOG/log.h"
#include "COMMON/mac_rrc_primitives.h"
#include "RRC/LITE/MESSAGES/asn1_msg.h"
#include "RRCConnectionRequest.h"
#include "UL-CCCH-Message.h"
#include "DL-CCCH-Message.h"
#include "UL-DCCH-Message.h"
#include "DL-DCCH-Message.h"
#include "TDD-Config.h"
#include "rlc.h"
#include "SIMULATION/ETH_TRANSPORT/extern.h"

#ifdef Rel10
#include "MeasResults.h"
#include "SCellToAddMod-r10.h"
#endif

#ifdef USER_MODE
# include "RRC/NAS/nas_config.h"
# include "RRC/NAS/rb_config.h"
# include "OCG.h"
# include "OCG_extern.h"
#endif

#if defined(ENABLE_USE_MME)
# include "../../S1AP/s1ap_eNB.h"
#endif

#ifdef PHY_EMUL
extern EMULATION_VARS *Emul_vars;
#endif
extern eNB_MAC_INST *eNB_mac_inst;
extern UE_MAC_INST *UE_mac_inst;
#ifdef BIGPHYSAREA
extern void *bigphys_malloc(int);
#endif

extern inline unsigned int taus(void);
void init_SI(u8 Mod_id) {

  u8 SIwindowsize=1;
  u16 SIperiod=8;
  uint32_t mib=0;
  int i;
  int N_RB_DL,phich_resource;

  /*
  do_MIB(mac_xface->lte_frame_parms,0x321,&mib);

  for (i=0;i<1024;i+=4)
    do_MIB(mac_xface->lte_frame_parms,i,&mib);

  N_RB_DL=6;
  while (N_RB_DL != 0) {
    phich_resource = 1;
    while (phich_resource != 0) {
      for (i=0;i<2;i++) {
	mac_xface->lte_frame_parms->N_RB_DL = N_RB_DL;
	mac_xface->lte_frame_parms->phich_config_common.phich_duration=i;
	mac_xface->lte_frame_parms->phich_config_common.phich_resource = phich_resource;
	do_MIB(mac_xface->lte_frame_parms,0,&mib);
      }
      if (phich_resource == 1)
	phich_resource = 3;
      else if (phich_resource == 3)
	phich_resource = 6;
      else if (phich_resource == 6)
	phich_resource = 12;
      else if (phich_resource == 12)
	phich_resource = 0;
    }
    if (N_RB_DL == 6)
      N_RB_DL = 15;
    else if (N_RB_DL == 15)
      N_RB_DL = 25;
    else if (N_RB_DL == 25)
      N_RB_DL = 50;
    else if (N_RB_DL == 50)
      N_RB_DL = 75;
    else if (N_RB_DL == 75)
      N_RB_DL = 100;
    else if (N_RB_DL == 100)
      N_RB_DL = 0;
  }
  exit(-1);
  */

  eNB_rrc_inst[Mod_id].sizeof_SIB1 = 0;
  eNB_rrc_inst[Mod_id].sizeof_SIB23 = 0;

  eNB_rrc_inst[Mod_id].SIB1 = (u8 *)malloc16(32);

  /*
  printf ("before SIB1 init : Nid_cell %d\n", mac_xface->lte_frame_parms->Nid_cell);
  printf ("before SIB1 init : frame_type %d,tdd_config %d\n",
	  mac_xface->lte_frame_parms->frame_type,
	  mac_xface->lte_frame_parms->tdd_config);
  */

  if (eNB_rrc_inst[Mod_id].SIB1)
    eNB_rrc_inst[Mod_id].sizeof_SIB1 = do_SIB1(mac_xface->lte_frame_parms,
					       (uint8_t *)eNB_rrc_inst[Mod_id].SIB1,
					       &eNB_rrc_inst[Mod_id].siblock1,
					       &eNB_rrc_inst[Mod_id].sib1);
  else {
    LOG_E(RRC,"[eNB] init_SI: FATAL, no memory for SIB1 allocated\n");
    mac_xface->macphy_exit("");
  }
  /*
  printf ("after SIB1 init : Nid_cell %d\n", mac_xface->lte_frame_parms->Nid_cell);
  printf ("after SIB1 init : frame_type %d,tdd_config %d\n",
	  mac_xface->lte_frame_parms->frame_type,
	  mac_xface->lte_frame_parms->tdd_config);
  */
  if (eNB_rrc_inst[Mod_id].sizeof_SIB1 == 255)
    mac_xface->macphy_exit("");

  eNB_rrc_inst[Mod_id].SIB23 = (u8 *)malloc16(64);
  if (eNB_rrc_inst[Mod_id].SIB23) {

    eNB_rrc_inst[Mod_id].sizeof_SIB23 = do_SIB23(Mod_id,
						 mac_xface->lte_frame_parms,
						 eNB_rrc_inst[Mod_id].SIB23,
						 &eNB_rrc_inst[Mod_id].systemInformation,
						 &eNB_rrc_inst[Mod_id].sib2,
						 &eNB_rrc_inst[Mod_id].sib3
#ifdef Rel10
						 ,
						 &eNB_rrc_inst[Mod_id].sib13,
						 eNB_rrc_inst[Mod_id].MBMS_flag
#endif
						 );
    /*
    eNB_rrc_inst[Mod_id].sizeof_SIB23 = do_SIB2_AT4(Mod_id,
						    eNB_rrc_inst[Mod_id].SIB23,
						    &eNB_rrc_inst[Mod_id].systemInformation,
						    &eNB_rrc_inst[Mod_id].sib2);
    */
    if (eNB_rrc_inst[Mod_id].sizeof_SIB23 == 255)
      mac_xface->macphy_exit("");

    LOG_D(RRC,"[eNB %d] SIB2/3 Contents (partial)\n", Mod_id);

    LOG_D(RRC,"[eNB %d] pusch_config_common.n_SB = %ld\n", Mod_id,eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB);


    LOG_D(RRC,"[eNB %d] pusch_config_common.hoppingMode = %ld\n", Mod_id, eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);

    LOG_D(RRC,"[eNB %d] pusch_config_common.pusch_HoppingOffset = %ld\n", Mod_id,eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);

    LOG_D(RRC,"[eNB %d] pusch_config_common.enable64QAM = %d\n", Mod_id,(int)eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);

    LOG_D(RRC,"[eNB %d] pusch_config_common.groupHoppingEnabled = %d\n", Mod_id,(int)eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);


    LOG_D(RRC,"[eNB %d] pusch_config_common.groupAssignmentPUSCH = %ld\n", Mod_id,eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);


    LOG_D(RRC,"[eNB %d] pusch_config_common.sequenceHoppingEnabled = %d\n", Mod_id,(int)eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);


    LOG_D(RRC, "[eNB %d] pusch_config_common.cyclicShift  = %ld\n",Mod_id, eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift);
#ifdef Rel10
    if (eNB_rrc_inst[Mod_id].MBMS_flag ==1) {
   //   LOG_D(RRC, "[eNB %d] mbsfn_SubframeConfigList.list.count = %ld\n", Mod_id, eNB_rrc_inst[Mod_id].sib2->mbsfn_SubframeConfigList->list.count);
    LOG_D(RRC, "[eNB %d] mbsfn_Subframe_pattern is  = %ld\n", Mod_id, eNB_rrc_inst[Mod_id].sib2->mbsfn_SubframeConfigList->list.array[0]->subframeAllocation.choice.oneFrame.buf[0]);

    LOG_D(RRC, "[eNB %d] SIB13 contents (partial)\n", Mod_id);
    LOG_D(RRC,"[eNB %d] Number of MBSFN Area:  %ld\n",Mod_id, eNB_rrc_inst[Mod_id].sib13->mbsfn_AreaInfoList_r9.list.count);
    LOG_D(RRC, "[eNB %d] MCCH Info of first MBSFN Area(partial)\n", Mod_id);
    LOG_D(RRC, "[eNB %d] MCCH Repetition Period: %d\n", Mod_id, eNB_rrc_inst[Mod_id].sib13->mbsfn_AreaInfoList_r9.list.array[0]->mcch_Config_r9.mcch_RepetitionPeriod_r9);
    LOG_D(RRC, "[eNB %d] MCCH Offset: %d\n", Mod_id, eNB_rrc_inst[Mod_id].sib13->mbsfn_AreaInfoList_r9.list.array[0]->mcch_Config_r9.mcch_Offset_r9);
    }
#endif
  LOG_D(RRC, "[MSC_MSG][FRAME unknown][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ (SIB1.tdd & SIB2 params) --->][MAC_UE][MOD %02d][]\n",
             Mod_id, Mod_id);

    rrc_mac_config_req(Mod_id,1,0,0,
		       (RadioResourceConfigCommonSIB_t *)&eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon,
		       (struct PhysicalConfigDedicated *)NULL,
#ifdef Rel10
		       (SCellToAddMod_r10_t *)NULL,
#endif
		       (MeasObjectToAddMod_t **)NULL,
		       (MAC_MainConfig_t *)NULL,
		       0,
		       (struct LogicalChannelConfig *)NULL,
		       (MeasGapConfig_t *)NULL,
		       eNB_rrc_inst[Mod_id].sib1->tdd_Config,
		       &SIwindowsize,
		       &SIperiod,
		       eNB_rrc_inst[Mod_id].sib2->freqInfo.ul_CarrierFreq,
		       eNB_rrc_inst[Mod_id].sib2->freqInfo.ul_Bandwidth,
		       &eNB_rrc_inst[Mod_id].sib2->freqInfo.additionalSpectrumEmission,
		       (MBSFN_SubframeConfigList_t *)eNB_rrc_inst[Mod_id].sib2->mbsfn_SubframeConfigList
#ifdef Rel10
		       ,
		       eNB_rrc_inst[Mod_id].MBMS_flag,		        		       
		       (MBSFN_AreaInfoList_r9_t *)&eNB_rrc_inst[Mod_id].sib13->mbsfn_AreaInfoList_r9
#endif 
		       );
  }
  else {
    LOG_E(RRC,"[eNB] init_SI: FATAL, no memory for SIB2/3 allocated\n");
    mac_xface->macphy_exit("");
  }
}

#ifdef Rel10
init_MCCH(u8 Mod_id) {
  // initialize RRC_eNB_INST MCCH entry
  eNB_rrc_inst[Mod_id].sizeof_MCCH_MESSAGE = 0;
  eNB_rrc_inst[Mod_id].MCCH_MESSAGE = (u8 *)malloc16(32);
  if (eNB_rrc_inst[Mod_id].MCCH_MESSAGE) {
    eNB_rrc_inst[Mod_id].sizeof_MCCH_MESSAGE = do_MBSFNAreaConfig(mac_xface->lte_frame_parms,
                                                                     (uint8_t *)eNB_rrc_inst[Mod_id].MCCH_MESSAGE,
                                                                     &eNB_rrc_inst[Mod_id].mcch,
                                                                     &eNB_rrc_inst[Mod_id].mcch_message);
    
    LOG_D(RRC, "[eNB %d] MCCH_MESSAGE  contents (partial)\n", Mod_id);
    LOG_D(RRC, "[eNB %d] CommonSF_AllocPeriod_r9 %d\n", Mod_id, eNB_rrc_inst[Mod_id].mcch_message->commonSF_AllocPeriod_r9);
    LOG_D(RRC, "[eNB %d] CommonSF_Alloc_r9.list.count (number of MBSFN Subframe Pattern) %d\n", Mod_id, eNB_rrc_inst[Mod_id].mcch_message->commonSF_Alloc_r9.list.count);
    LOG_D(RRC, "[eNB %d] Add of First MBSFN Subframe Pattern: %d\n", Mod_id, eNB_rrc_inst[Mod_id].mcch_message->commonSF_Alloc_r9.list.array[0]);
    LOG_D(RRC, "[eNB %d] First MBSFN Subframe Pattern: %d\n", Mod_id, eNB_rrc_inst[Mod_id].mcch_message->commonSF_Alloc_r9.list.array[0]->subframeAllocation.choice.oneFrame.buf[0]);

  }
  else {
    LOG_E(RRC, "[eNB] init_MCCH: FATAL, no memory for MCCH MESSAGE allocated\n");
    mac_xface->macphy_exit("");
  }
  
  if (eNB_rrc_inst[Mod_id].sizeof_MCCH_MESSAGE == 255)
    mac_xface->macphy_exit("");

  //Set the eNB_rrc_inst[Mod_id].MCCH_MESS.Active to 1 (allow to  transfer MCCH message RCC->MAC
  // in function mac_rrc_data_req

  //  rrc_config_buffer(&eNB_rrc_inst[Mod_id].MCCH_MESS,MCCH,1);
  eNB_rrc_inst[Mod_id].MCCH_MESS.Active =1;

  // Configure MCCH logical channel
  // call mac_config_req with appropriate structure from ASN.1 description

  }
#endif

/*------------------------------------------------------------------------------*/
char openair_rrc_lite_eNB_init(u8 Mod_id){
  /*-----------------------------------------------------------------------------*/

  unsigned char j;
  LOG_I(RRC,"[eNB %d] Init (UE State = RRC_IDLE)...\n", Mod_id);
  LOG_D(RRC, "[MSC_NEW][FRAME 00000][RRC_eNB][MOD %02d][]\n", Mod_id);
  LOG_D(RRC, "[MSC_NEW][FRAME 00000][IP][MOD %02d][]\n", Mod_id);

  for (j=0; j<NUMBER_OF_UE_MAX; j++)
    eNB_rrc_inst[Mod_id].Info.Status[j] =  RRC_IDLE;//CH_READY;

#if defined(ENABLE_USE_MME)
    /* Connect eNB to MME */
    if (oai_emulation.info.mme_enabled > 0) {
        if (s1ap_eNB_init(oai_emulation.info.mme_ip_address, Mod_id) < 0) {
            mac_xface->macphy_exit("");
            return -1;
        }
    }
#endif

  eNB_rrc_inst[Mod_id].Info.Nb_ue=0;

  eNB_rrc_inst[Mod_id].Srb0.Active=0;

  for(j=0;j<(NUMBER_OF_UE_MAX+1);j++){
    eNB_rrc_inst[Mod_id].Srb2[j].Active=0;
  }


  /// System Information INIT


  LOG_I(RRC,"Checking release \n"); 
#ifdef Rel10
  // This has to come from some top-level configuration
  eNB_rrc_inst[Mod_id].MBMS_flag = 0;
#else 
  printf("Rel8 RRC\n");
#endif

  init_SI(Mod_id);

#ifdef Rel10
  /// MCCH INIT
  init_MCCH(Mod_id);
#endif

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id,1);
#else
  eNB_rrc_inst[Mod_id].Last_scan_req=0;
   send_msg(&S_rrc,msg_rrc_phy_synch_to_MR_ind(Mod_id,eNB_rrc_inst[Mod_id].Mac_id));
#endif

  return 0;

}


u8 get_next_UE_index(u8 Mod_id,u8 *UE_identity) {

  u8 i,first_index = 255,reg=0;

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {


    if ((first_index == 255) && (*(unsigned int*)eNB_rrc_inst[Mod_id].Info.UE_list[i] == 0x00000000))
      first_index = i;  // save first free position

    if ((eNB_rrc_inst[Mod_id].Info.UE_list[i][0]==UE_identity[0]) &&
	(eNB_rrc_inst[Mod_id].Info.UE_list[i][1]==UE_identity[1]) &&
	(eNB_rrc_inst[Mod_id].Info.UE_list[i][2]==UE_identity[2]) &&
	(eNB_rrc_inst[Mod_id].Info.UE_list[i][3]==UE_identity[3]) &&
	(eNB_rrc_inst[Mod_id].Info.UE_list[i][4]==UE_identity[4]))      // UE_identity already registered
      reg=1;

  }

  if (reg==0) {
    LOG_I(RRC,"Adding UE %d\n",first_index);
    return(first_index);
  }
  else
    return(255);
}

void rrc_remove_UE(u8 Mod_id,u8 UE_id) {

  int i;
  LOG_I(RRC,"Removing UE %d\n",UE_id);
  eNB_rrc_inst[Mod_id].Info.Status[UE_id] = RRC_IDLE;
  *(unsigned int*)eNB_rrc_inst[Mod_id].Info.UE_list[UE_id] = 0x00000000;
}


/*------------------------------------------------------------------------------*/
int rrc_eNB_decode_dcch(u8 Mod_id, u32 frame, u8 Srb_id, u8 UE_index, u8 *Rx_sdu, u8 sdu_size) {
  /*------------------------------------------------------------------------------*/

  asn_dec_rval_t dec_rval;
  //UL_DCCH_Message_t uldcchmsg;
  UL_DCCH_Message_t *ul_dcch_msg=NULL;//&uldcchmsg;
  UE_EUTRA_Capability_t *UE_EUTRA_Capability=NULL;

  int i;

  if (Srb_id != 1) {
    LOG_E(RRC,"[eNB %d] Frame %d: Received message on SRB%d, should not have ...\n",Mod_id,frame,Srb_id);
  }

  //memset(ul_dcch_msg,0,sizeof(UL_DCCH_Message_t));

  LOG_D(RRC,"[eNB %d] Frame %d: Decoding UL-DCCH Message\n",
      Mod_id,frame);
  dec_rval = uper_decode(NULL,
                         &asn_DEF_UL_DCCH_Message,
                         (void**)&ul_dcch_msg,
                         Rx_sdu,
                         sdu_size,
                         0,
                         0);
  for (i=0;i<sdu_size;i++)
    msg("%x.",Rx_sdu[i]);
  msg("\n");

  if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
    LOG_E(RRC,"[UE %d] Frame %d : Failed to decode UL-DCCH (%d bytes)\n",Mod_id,frame,dec_rval.consumed);
    return -1;
  }

  if (ul_dcch_msg->message.present == UL_DCCH_MessageType_PR_c1) {

    switch (ul_dcch_msg->message.choice.c1.present) {

    case UL_DCCH_MessageType__c1_PR_NOTHING:     /* No components present */
      break;
    case UL_DCCH_MessageType__c1_PR_csfbParametersRequestCDMA2000:
      break;
    case UL_DCCH_MessageType__c1_PR_measurementReport:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND "
                 "%d bytes (measurementReport) --->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, DCCH, sdu_size, Mod_id);
      rrc_eNB_process_MeasurementReport(Mod_id,UE_index,&ul_dcch_msg->message.choice.c1.choice.measurementReport.criticalExtensions.choice.c1.choice.measurementReport_r8.measResults);
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionReconfigurationComplete:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes "
                 "(RRCConnectionReconfigurationComplete) --->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, DCCH, sdu_size, Mod_id);
      if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionReconfigurationComplete.criticalExtensions.present == RRCConnectionReconfigurationComplete__criticalExtensions_PR_rrcConnectionReconfigurationComplete_r8) {
        rrc_eNB_process_RRCConnectionReconfigurationComplete(Mod_id,frame,UE_index,&ul_dcch_msg->message.choice.c1.choice.rrcConnectionReconfigurationComplete.criticalExtensions.choice.rrcConnectionReconfigurationComplete_r8);
        eNB_rrc_inst[Mod_id].Info.Status[UE_index] = RRC_RECONFIGURED;
        LOG_D(RRC,"[eNB %d] UE %d State = RRC_RECONFIGURED \n",Mod_id,UE_index);
      }
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionReestablishmentComplete:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes "
                 "(rrcConnectionReestablishmentComplete) --->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, DCCH, sdu_size, Mod_id);
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionSetupComplete:
        LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes "
                   "(RRCConnectionSetupComplete) --->][RRC_eNB][MOD %02d][]\n",
              frame, Mod_id, DCCH, sdu_size, Mod_id);

        if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.present == RRCConnectionSetupComplete__criticalExtensions_PR_c1) {
            if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.choice.c1.present == RRCConnectionSetupComplete__criticalExtensions__c1_PR_rrcConnectionSetupComplete_r8) {
                rrc_eNB_process_RRCConnectionSetupComplete(Mod_id, frame, UE_index, &ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8);
                eNB_rrc_inst[Mod_id].Info.Status[UE_index] = RRC_CONNECTED;
                LOG_D(RRC, "[eNB %d] UE %d State = RRC_CONNECTED \n", Mod_id, UE_index);
                LOG_D(RRC, "[MSC_NBOX][FRAME %05d][RRC_eNB][MOD %02d][][Rx RRCConnectionSetupComplete\n"
                           "Now CONNECTED with UE %d][RRC_eNB][MOD %02d][]\n",
                      frame, Mod_id, UE_index, Mod_id);
            }
        }
        break;
    case UL_DCCH_MessageType__c1_PR_securityModeComplete:
       LOG_D(RRC,"[eNB %d] Frame %d received securityModeComplete on UL-DCCH %d from UE %d\n",
	    Mod_id,  frame, DCCH, UE_index );
       LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes "
	    "(securityModeComplete) --->][RRC_eNB][MOD %02d][]\n",
	    frame, Mod_id, DCCH, sdu_size, Mod_id);
       xer_fprint(stdout, &asn_DEF_UL_DCCH_Message, (void*)ul_dcch_msg);
       // confirm with PDCP about the security mode for DCCH
       rrc_pdcp_config_req (Mod_id, frame, 1,ACTION_SET_SECURITY_MODE, (UE_index * MAX_NUM_RB) + DCCH, 0x77);
       // continue the procedure
       rrc_eNB_generate_UECapabilityEnquiry(Mod_id,frame,UE_index);
      break;
    case UL_DCCH_MessageType__c1_PR_securityModeFailure:
       LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes "
	    "(securityModeFailure) --->][RRC_eNB][MOD %02d][]\n",
	    frame, Mod_id, DCCH, sdu_size, Mod_id);
       xer_fprint(stdout, &asn_DEF_UL_DCCH_Message, (void*)ul_dcch_msg);
       // cancel the security mode in PDCP
       
       // followup with the remaining procedure
       rrc_eNB_generate_UECapabilityEnquiry(Mod_id,frame,UE_index);
       break;
    case UL_DCCH_MessageType__c1_PR_ueCapabilityInformation:
      LOG_D(RRC,"[eNB %d] Frame %d received ueCapabilityInformation on UL-DCCH %d from UE %d\n",
	    Mod_id,  frame, DCCH, UE_index );
       LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes "
	    "(UECapabilityInformation) --->][RRC_eNB][MOD %02d][]\n",
	    frame, Mod_id, DCCH, sdu_size, Mod_id);
      xer_fprint(stdout, &asn_DEF_UL_DCCH_Message, (void*)ul_dcch_msg);
      dec_rval = uper_decode(NULL,
			     &asn_DEF_UE_EUTRA_Capability,
			     (void**)&UE_EUTRA_Capability,
			     ul_dcch_msg->message.choice.c1.choice.ueCapabilityInformation.criticalExtensions.choice.c1.choice.ueCapabilityInformation_r8.ue_CapabilityRAT_ContainerList.list.array[0]->ueCapabilityRAT_Container.buf,
			     ul_dcch_msg->message.choice.c1.choice.ueCapabilityInformation.criticalExtensions.choice.c1.choice.ueCapabilityInformation_r8.ue_CapabilityRAT_ContainerList.list.array[0]->ueCapabilityRAT_Container.size,
			     0,
			     0);      
      xer_fprint(stdout,&asn_DEF_UE_EUTRA_Capability,(void*)UE_EUTRA_Capability);

      rrc_eNB_generate_RRCConnectionReconfiguration(Mod_id,frame,UE_index, NULL, 0);
      break;
    case UL_DCCH_MessageType__c1_PR_ulHandoverPreparationTransfer:
      break;
    case UL_DCCH_MessageType__c1_PR_ulInformationTransfer:
#if defined(ENABLE_USE_MME)
    {
        if (oai_emulation.info.mme_enabled == 1) {
            ULInformationTransfer_t *ulInformationTransfer;
            ulInformationTransfer = &ul_dcch_msg->message.choice.c1.choice.ulInformationTransfer;

            if (ulInformationTransfer->criticalExtensions.present ==
                ULInformationTransfer__criticalExtensions_PR_c1) {
                if (ulInformationTransfer->criticalExtensions.choice.c1.present ==
                    ULInformationTransfer__criticalExtensions__c1_PR_ulInformationTransfer_r8) {

                    ULInformationTransfer_r8_IEs_t *ulInformationTransferR8;
                    ulInformationTransferR8 = &ulInformationTransfer->criticalExtensions.choice.c1.choice.ulInformationTransfer_r8;
                    if (ulInformationTransferR8->dedicatedInfoType.present == ULInformationTransfer_r8_IEs__dedicatedInfoType_PR_dedicatedInfoNAS)
                        s1ap_eNB_new_data_request(Mod_id,
                                                  UE_index,
                                                  ulInformationTransferR8->dedicatedInfoType.choice.dedicatedInfoNAS.buf,
                                                  ulInformationTransferR8->dedicatedInfoType.choice.dedicatedInfoNAS.size);
                }
            }
        }
    }
#endif
      break;
    case UL_DCCH_MessageType__c1_PR_counterCheckResponse:
      break;
#ifdef Rel10
    case UL_DCCH_MessageType__c1_PR_ueInformationResponse_r9:
      break;
    case UL_DCCH_MessageType__c1_PR_proximityIndication_r9:
      break;
    case UL_DCCH_MessageType__c1_PR_rnReconfigurationComplete_r10:
      break;
    case UL_DCCH_MessageType__c1_PR_mbmsCountingResponse_r10:
      break;
    case UL_DCCH_MessageType__c1_PR_interFreqRSTDMeasurementIndication_r10:
      break;
#endif
    default:
     LOG_E(RRC,"[UE %d] Frame %d : Unknown message\n",Mod_id,frame);
     return -1;
    }
    return 0;
  }
  else {
    LOG_E(RRC,"[UE %d] Frame %d : Unknown error\n",Mod_id,frame);
    return -1;
  }

}


/*------------------------------------------------------------------------------*/
int rrc_eNB_decode_ccch(u8 Mod_id, u32 frame, SRB_INFO *Srb_info){
  /*------------------------------------------------------------------------------*/

  u16 Idx,UE_index;

  asn_dec_rval_t dec_rval;
  //UL_CCCH_Message_t ulccchmsg;
  UL_CCCH_Message_t *ul_ccch_msg=NULL; //&ulccchmsg;
  RRCConnectionRequest_r8_IEs_t *rrcConnectionRequest;
  int i,rval;


  //memset(ul_ccch_msg,0,sizeof(UL_CCCH_Message_t));

  LOG_D(RRC,"[eNB %d] Frame %d: Decoding UL CCCH %x.%x.%x.%x.%x.%x (%p)\n", Mod_id,frame,
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[0],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[1],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[2],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[3],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[4],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[5],
      (uint8_t*)Srb_info->Rx_buffer.Payload);
  dec_rval = uper_decode(NULL,
			 &asn_DEF_UL_CCCH_Message,
			 (void**)&ul_ccch_msg,
			 (uint8_t*)Srb_info->Rx_buffer.Payload,
			 100,0,0);
  for (i=0;i<8;i++)
    msg("%x.",((u8*)&ul_ccch_msg)[i]);
  if (dec_rval.consumed == 0) {
    LOG_E(RRC,"[eNB %d] FATAL Error in receiving CCCH\n", Mod_id);
    return -1; //mac_xface->macphy_exit(""); //exit(-1);
  }
  if (ul_ccch_msg->message.present == UL_CCCH_MessageType_PR_c1) {

    switch (ul_ccch_msg->message.choice.c1.present) {

    case UL_CCCH_MessageType__c1_PR_NOTHING :
      LOG_I(RRC,"[eNB %d] Frame %d : Received PR_NOTHING on UL-CCCH-Message\n",Mod_id,frame);
      break;

    case UL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentRequest :
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_eNB][MOD %02d][][--- MAC_DATA_IND (rrcConnectionReestablishmentRequest on SRB0) -->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, Mod_id);
      LOG_I(RRC,"[eNB %d] Frame %d : RRCConnectionReestablishmentRequest not supported yet\n",Mod_id,frame);
      break;

    case UL_CCCH_MessageType__c1_PR_rrcConnectionRequest :
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_eNB][MOD %02d][][--- MAC_DATA_IND  (rrcConnectionRequest on SRB0) -->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, Mod_id);

      rrcConnectionRequest = &ul_ccch_msg->message.choice.c1.choice.rrcConnectionRequest.criticalExtensions.choice.rrcConnectionRequest_r8;
      UE_index = get_next_UE_index(Mod_id,(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf);

      if (UE_index!=255) {

	//	memcpy(&Rrc_xface->UE_id[Mod_id][UE_index],(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf,5);
	memcpy(&eNB_rrc_inst[Mod_id].Info.UE_list[UE_index],(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf,5);

	LOG_I(RRC,"[eNB %d] Frame %d : Accept new connection from UE %d (%x%x%x%x%x)\n",Mod_id,frame,UE_index,
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][0],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][1],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][2],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][3],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][4]);

	//CONFIG SRB2  (DCCHs, ONE per User)  //meas && lchan Cfg
	//eNB_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Status=NEED_RADIO_CONFIG;
	//eNB_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Next_eNBeck_frame=Rrc_xface->Frame_index+1;
	eNB_rrc_inst[Mod_id].Info.Nb_ue++;

#ifndef NO_RRM
	send_msg(&S_rrc,msg_rrc_MR_attach_ind(Mod_id,Mac_id));
#else


	Idx = (UE_index * MAX_NUM_RB) + DCCH;
	// SRB1
	eNB_rrc_inst[Mod_id].Srb1[UE_index].Active = 1;
	eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Srb_id = Idx;
	memcpy(&eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
	memcpy(&eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);

	// SRB2
	eNB_rrc_inst[Mod_id].Srb2[UE_index].Active = 1;
	eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Srb_id = Idx;
	memcpy(&eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
	memcpy(&eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);

	rrc_eNB_generate_RRCConnectionSetup(Mod_id,frame,UE_index);
      //LOG_D(RRC, "[MSC_NBOX][FRAME %05d][RRC_eNB][MOD %02d][][Tx RRCConnectionSetup][RRC_eNB][MOD %02d][]\n",
      //      frame, Mod_id, Mod_id);

	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@0 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[0].allocation);
	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@1 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[1].allocation);
	LOG_I(RRC,"[eNB %d] CALLING RLC CONFIG SRB1 (rbid %d) for UE %d\n",
	    Mod_id,Idx,UE_index);
	rrc_pdcp_config_req (Mod_id, frame, 1, ACTION_ADD, Idx, UNDEF_SECURITY_MODE);
	rrc_rlc_config_req(Mod_id,frame,1,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);

	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@0 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[0].allocation);
	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@1 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[1].allocation);

	/*

	LOG_D(RRC,"[eNB %d] CALLING RLC CONFIG SRB2 (rbid %d) for UE %d\n",
	    Mod_id,Idx+1,UE_index);
	Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx+1,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
	LOG_D(RRC,"[eNB %d] RLC AM allocation index@0 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[0].allocation);
	LOG_D(RRC,"[eNB %d] RLC AM allocation index@1 is %d\n",rlc[Mod_id].m_rlc_am_array[1].allocation);
	*/
#endif //NO_RRM
      }
      else {
	LOG_E(RRC,"can't add UE, max user count reached!\n");
      }
	break;

    default:
      LOG_E(RRC,"[eNB %d] Frame %d : Unknown message\n",Mod_id,frame);
      rval = -1;
    }
    rval = 0;
  }
  else{
    LOG_E(RRC,"[eNB %d] Frame %d : Unknown error \n",Mod_id,frame);
      rval = -1;
  }
  return rval;
}



void rrc_eNB_process_RRCConnectionSetupComplete(
    u8 Mod_id,
    u32 frame,
    u8 UE_index,RRCConnectionSetupComplete_r8_IEs_t *rrcConnectionSetupComplete) {

    LOG_I(RRC, "[eNB %d][RAPROC] Frame %d : Logical Channel UL-DCCH, "
               "processing RRCConnectionSetupComplete from UE %d\n",
          Mod_id, frame, UE_index);

    // Forward message to S1AP layer
#if defined(ENABLE_USE_MME)
    if (oai_emulation.info.mme_enabled == 1)
        s1ap_eNB_new_data_request(Mod_id, UE_index,
                                rrcConnectionSetupComplete->dedicatedInfoNAS.buf,
                                rrcConnectionSetupComplete->dedicatedInfoNAS.size);
    else
#endif
      rrc_eNB_generate_SecurityModeCommand(Mod_id,frame,UE_index);
    //rrc_eNB_generate_UECapabilityEnquiry(Mod_id,frame,UE_index);


}

mui_t rrc_eNB_mui=0;

void rrc_eNB_generate_SecurityModeCommand(u8 Mod_id, u32 frame, u16 UE_index) { 

  uint8_t buffer[100];
  uint8_t size;
  int i;

  size = do_SecurityModeCommand(Mod_id,buffer,UE_index,0);

  LOG_I(RRC,"[eNB %d] Frame %d, Logical Channel DL-DCCH, Generate SecurityModeCommand (bytes %d, UE id %d)\n",
        Mod_id,frame, size, UE_index);


  LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (securityModeCommand to UE %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
        frame, Mod_id, size, UE_index, rrc_eNB_mui, Mod_id, (UE_index*MAX_NUM_RB)+DCCH);
  //rrc_rlc_data_req(Mod_id,frame, 1,(UE_index*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer);
  pdcp_data_req(Mod_id, frame, 1, (UE_index * MAX_NUM_RB) + DCCH, rrc_eNB_mui++, 0, size, (char*)buffer, 1);

}

void rrc_eNB_generate_UECapabilityEnquiry(u8 Mod_id, u32 frame, u16 UE_index) { 

  uint8_t buffer[100];
  uint8_t size;
  int i;

  size = do_UECapabilityEnquiry(Mod_id,buffer,UE_index,0);

  LOG_I(RRC,"[eNB %d] Frame %d, Logical Channel DL-DCCH, Generate UECapabilityEnquiry (bytes %d, UE id %d)\n",
        Mod_id,frame, size, UE_index);


  LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (UECapabilityEnquiry to UE %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
        frame, Mod_id, size, UE_index, rrc_eNB_mui, Mod_id, (UE_index*MAX_NUM_RB)+DCCH);
  //rrc_rlc_data_req(Mod_id,frame, 1,(UE_index*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer);
  pdcp_data_req(Mod_id, frame, 1, (UE_index * MAX_NUM_RB) + DCCH, rrc_eNB_mui++, 0, size, (char*)buffer, 1);

}


int rrc_eNB_generate_RRCConnectionReconfiguration_SCell(u8 Mod_id, u32 frame, u16 UE_index, u32 dl_CarrierFreq_r10) {

  u8 size;
  u8 buffer[100];
  u8 sCellIndexToAdd = 0; //one SCell so far

  if (eNB_rrc_inst[Mod_id].sCell_config[UE_index][sCellIndexToAdd]) {
    eNB_rrc_inst[Mod_id].sCell_config[UE_index][sCellIndexToAdd]->cellIdentification_r10->dl_CarrierFreq_r10 = dl_CarrierFreq_r10;
  }
  else {
    LOG_E(RRC,"Scell not configured!\n");
    return(-1);
  }

  size = do_RRCConnectionReconfiguration(Mod_id,
                                         buffer,
                                         UE_index,
                                         0,//Transaction_id,
                                         (SRB_ToAddModList_t*)NULL,
                                         (DRB_ToAddModList_t*)NULL,
                                         (DRB_ToReleaseList_t*)NULL,
                                         (struct SPS_Config*)NULL,
                                         (struct PhysicalConfigDedicated*)NULL,
#ifdef Rel10
					 eNB_rrc_inst[Mod_id].sCell_config[UE_index][sCellIndexToAdd],
#endif
                                         (MeasObjectToAddModList_t*)NULL,
                                         (ReportConfigToAddModList_t*)NULL,
                                         (QuantityConfig_t*)NULL, 
                                         (MeasIdToAddModList_t*)NULL,
                                         (MAC_MainConfig_t*)NULL,
                                         (MeasGapConfig_t*)NULL,
                                         (uint8_t*)NULL,
                                         0); 

  LOG_I(RRC,"[eNB %d] Frame %d, Logical Channel DL-DCCH, Generate RRCConnectionReconfiguration (bytes %d, UE id %d)\n",
        Mod_id,frame, size, UE_index);

  LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (rrcConnectionReconfiguration to UE %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
        frame, Mod_id, size, UE_index, rrc_eNB_mui, Mod_id, (UE_index*MAX_NUM_RB)+DCCH);
  //rrc_rlc_data_req(Mod_id,frame, 1,(UE_index*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer);
  pdcp_data_req(Mod_id, frame, 1, (UE_index * MAX_NUM_RB) + DCCH, rrc_eNB_mui++, 0, size, (char*)buffer, 1);

  return(0);
}




void rrc_eNB_generate_RRCConnectionReconfiguration(u8 Mod_id, u32 frame, u16 UE_index, u8 *nas_pdu, u32 nas_length) {


  u8 buffer[100];
  u8 size;
  int i;

  // configure SRB1/SRB2, PhysicalConfigDedicated, MAC_MainConfig for UE
  eNB_RRC_INST *rrc_inst = &eNB_rrc_inst[Mod_id];

  struct SRB_ToAddMod **SRB2_config                         = &rrc_inst->SRB2_config[UE_index];
  struct DRB_ToAddMod **DRB_config                          = &rrc_inst->DRB_config[UE_index][0];
  struct PhysicalConfigDedicated  **physicalConfigDedicated = &rrc_inst->physicalConfigDedicated[UE_index];


  struct SRB_ToAddMod *SRB2_config2;
  struct SRB_ToAddMod__rlc_Config *SRB2_rlc_config;
  struct SRB_ToAddMod__logicalChannelConfig *SRB2_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *SRB2_ul_SpecificParameters;
  SRB_ToAddModList_t *SRB_list;

  struct DRB_ToAddMod *DRB_config2;
  struct RLC_Config *DRB_rlc_config;
  struct LogicalChannelConfig *DRB_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *DRB_ul_SpecificParameters;
  DRB_ToAddModList_t *DRB_list;
  MAC_MainConfig_t *mac_MainConfig;
  MeasObjectToAddModList_t *MeasObj_list;
  MeasObjectToAddMod_t *MeasObj;
  ReportConfigToAddModList_t *ReportConfig_list;
  ReportConfigToAddMod_t *ReportConfig_per,*ReportConfig_A1,*ReportConfig_A2,*ReportConfig_A3,*ReportConfig_A4,*ReportConfig_A5;
  MeasIdToAddModList_t *MeasId_list;
  MeasIdToAddMod_t *MeasId0,*MeasId1,*MeasId2,*MeasId3,*MeasId4,*MeasId5;
#if Rel10
  long * sr_ProhibitTimer_r9;
  struct PUSCH_CAConfigDedicated_vlola  *pusch_CAConfigDedicated_vlola;
  uint8_t sCellIndexToAdd = rrc_find_free_SCell_index(Mod_id, UE_index, 1);
  SCellToAddModList_r10_t *sCellToAddList;
  SCellToAddMod_r10_t *sCell1_config_ptr;
  struct RadioResourceConfigDedicatedSCell_r10 *radioResourceConfigDedicatedSCell;
  struct RadioResourceConfigCommonSCell_r10 *radioResourceConfigCommonSCell;
  struct PhysicalConfigDedicatedSCell_r10 *physicalConfigDedicatedSCell_r10;
#endif

  long *logicalchannelgroup,*logicalchannelgroup_drb;
  long *maxHARQ_Tx, *periodicBSR_Timer;

  RSRP_Range_t *rsrp;
  struct MeasConfig__speedStatePars *Sparams;
  CellsToAddMod_t *CellToAdd;
  CellsToAddModList_t *CellsToAddModList;



  //
  // Configure SRB2

  SRB_list = CALLOC(1,sizeof(*SRB_list));

  /// SRB2
  SRB2_config2 = CALLOC(1,sizeof(*SRB2_config2));
  *SRB2_config = SRB2_config2;

  SRB2_config2->srb_Identity = 2;
  SRB2_rlc_config = CALLOC(1,sizeof(*SRB2_rlc_config));
  SRB2_config2->rlc_Config   = SRB2_rlc_config;

  SRB2_rlc_config->present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  SRB2_rlc_config->choice.explicitValue.present=RLC_Config_PR_am;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit = T_PollRetransmit_ms45;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU          = PollPDU_pInfinity;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte         = PollPDU_pInfinity;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold = UL_AM_RLC__maxRetxThreshold_t4;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering     = T_Reordering_ms35;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit = T_StatusProhibit_ms0;


  SRB2_lchan_config = CALLOC(1,sizeof(*SRB2_lchan_config));
  SRB2_config2->logicalChannelConfig   = SRB2_lchan_config;

  SRB2_lchan_config->present                                    = SRB_ToAddMod__logicalChannelConfig_PR_explicitValue;


  SRB2_ul_SpecificParameters = CALLOC(1,sizeof(*SRB2_ul_SpecificParameters));

  SRB2_ul_SpecificParameters->priority           = 1;
  SRB2_ul_SpecificParameters->prioritisedBitRate = LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;
  SRB2_ul_SpecificParameters->bucketSizeDuration = LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup = CALLOC(1,sizeof(long));
  *logicalchannelgroup=0;

  SRB2_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup;

  SRB2_lchan_config->choice.explicitValue.ul_SpecificParameters = SRB2_ul_SpecificParameters;
  ASN_SEQUENCE_ADD(&SRB_list->list,SRB2_config2);

  // Configure DRB

  DRB_list = CALLOC(1,sizeof(*DRB_list));

  /// DRB
  DRB_config2 = CALLOC(1,sizeof(*DRB_config2));
  *DRB_config = DRB_config2;

  //DRB_config2->drb_Identity = (DRB_Identity_t) 1; //allowed values 1..32
  DRB_config2->drb_Identity = (DRB_Identity_t) (UE_index+1); //allowed values 1..32
  DRB_config2->logicalChannelIdentity = CALLOC(1,sizeof(long));
  *(DRB_config2->logicalChannelIdentity) = (long) 3;
  DRB_rlc_config = CALLOC(1,sizeof(*DRB_rlc_config));
  DRB_config2->rlc_Config   = DRB_rlc_config;

  DRB_rlc_config->present=RLC_Config_PR_um_Bi_Directional;
  DRB_rlc_config->choice.um_Bi_Directional.ul_UM_RLC.sn_FieldLength=SN_FieldLength_size5;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.sn_FieldLength=SN_FieldLength_size5;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.t_Reordering=T_Reordering_ms35;
  DRB_lchan_config = CALLOC(1,sizeof(*DRB_lchan_config));
  DRB_config2->logicalChannelConfig   = DRB_lchan_config;
  DRB_ul_SpecificParameters = CALLOC(1,sizeof(*DRB_ul_SpecificParameters));
  DRB_lchan_config->ul_SpecificParameters = DRB_ul_SpecificParameters;


  DRB_ul_SpecificParameters->priority = 2; // lower priority than srb1, srb2
  DRB_ul_SpecificParameters->prioritisedBitRate=LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;
  DRB_ul_SpecificParameters->bucketSizeDuration=LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup_drb = CALLOC(1,sizeof(long));
  *logicalchannelgroup_drb=0;
  DRB_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup_drb;


  ASN_SEQUENCE_ADD(&DRB_list->list,DRB_config2);

  mac_MainConfig = CALLOC(1,sizeof(*mac_MainConfig));
  eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index] = mac_MainConfig;

  mac_MainConfig->ul_SCH_Config = CALLOC(1,sizeof(*mac_MainConfig->ul_SCH_Config));

  maxHARQ_Tx = CALLOC(1,sizeof(long));
  *maxHARQ_Tx=MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
  mac_MainConfig->ul_SCH_Config->maxHARQ_Tx = maxHARQ_Tx;

  periodicBSR_Timer = CALLOC(1,sizeof(long));
  *periodicBSR_Timer = MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf64;
  mac_MainConfig->ul_SCH_Config->periodicBSR_Timer =  periodicBSR_Timer;

  mac_MainConfig->ul_SCH_Config->retxBSR_Timer =  MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf320;

  mac_MainConfig->ul_SCH_Config->ttiBundling=0; // FALSE

  mac_MainConfig->drx_Config = NULL;

  mac_MainConfig->phr_Config = CALLOC(1,sizeof(*mac_MainConfig->phr_Config));

  mac_MainConfig->phr_Config->present = MAC_MainConfig__phr_Config_PR_setup;
  mac_MainConfig->phr_Config->choice.setup.periodicPHR_Timer= MAC_MainConfig__phr_Config__setup__periodicPHR_Timer_sf20; // sf20 = 20 subframes

  mac_MainConfig->phr_Config->choice.setup.prohibitPHR_Timer=MAC_MainConfig__phr_Config__setup__prohibitPHR_Timer_sf20; // sf20 = 20 subframes

  mac_MainConfig->phr_Config->choice.setup.dl_PathlossChange=MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB1; // Value dB1 =1 dB, dB3 = 3 dB

#ifdef Rel10
  sr_ProhibitTimer_r9 = CALLOC(1,sizeof(long));
  *sr_ProhibitTimer_r9=0; // SR tx on PUCCH, Value in number of SR period(s). Value 0 = no timer for SR, Value 2= 2*SR
  mac_MainConfig->sr_ProhibitTimer_r9=sr_ProhibitTimer_r9;
  //sps_RA_ConfigList_rlola = NULL;
#endif


  // Measurement ID list
  MeasId_list       = CALLOC(1,sizeof(*MeasId_list));
  memset((void *)MeasId_list,0,sizeof(*MeasId_list));

  MeasId0            = CALLOC(1,sizeof(*MeasId0));
  MeasId0->measId = 1;
  MeasId0->measObjectId = 1;
  MeasId0->reportConfigId = 1;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId0);

  MeasId1            = CALLOC(1,sizeof(*MeasId1));
  MeasId1->measId = 2;
  MeasId1->measObjectId = 1;
  MeasId1->reportConfigId = 2;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId1);

  MeasId2            = CALLOC(1,sizeof(*MeasId2));
  MeasId2->measId = 3;
  MeasId2->measObjectId = 1;
  MeasId2->reportConfigId = 3;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId2);

  MeasId3            = CALLOC(1,sizeof(*MeasId3));
  MeasId3->measId = 4;
  MeasId3->measObjectId = 1;
  MeasId3->reportConfigId = 4;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId3);

  MeasId4            = CALLOC(1,sizeof(*MeasId4));
  MeasId4->measId = 5;
  MeasId4->measObjectId = 1;
  MeasId4->reportConfigId = 5;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId4);

  MeasId5            = CALLOC(1,sizeof(*MeasId5));
  MeasId5->measId = 6;
  MeasId5->measObjectId = 1;
  MeasId5->reportConfigId = 6;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId5);

  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measIdToAddModList = MeasId_list;

  // Add one EUTRA Measurement Object
  MeasObj_list      = CALLOC(1,sizeof(*MeasObj_list));
  memset((void *)MeasObj_list,0,sizeof(*MeasObj_list));

  // Configure MeasObject

  MeasObj           = CALLOC(1,sizeof(*MeasObj));
  memset((void *)MeasObj,0,sizeof(*MeasObj));

  MeasObj->measObjectId           = 1;
  MeasObj->measObject.present                = MeasObjectToAddMod__measObject_PR_measObjectEUTRA;
  MeasObj->measObject.choice.measObjectEUTRA.carrierFreq                 = 36090;
  MeasObj->measObject.choice.measObjectEUTRA.allowedMeasBandwidth        = AllowedMeasBandwidth_mbw25;
  MeasObj->measObject.choice.measObjectEUTRA.presenceAntennaPort1        = 1;
  MeasObj->measObject.choice.measObjectEUTRA.neighCellConfig.buf         = CALLOC(1,sizeof(uint8_t));
  MeasObj->measObject.choice.measObjectEUTRA.neighCellConfig.buf[0]      = 0;
  MeasObj->measObject.choice.measObjectEUTRA.neighCellConfig.size        = 1;
  MeasObj->measObject.choice.measObjectEUTRA.neighCellConfig.bits_unused = 6;
  MeasObj->measObject.choice.measObjectEUTRA.offsetFreq                  = NULL; // Default is 15 or 0dB

  MeasObj->measObject.choice.measObjectEUTRA.cellsToAddModList = (CellsToAddModList_t *)CALLOC(1,sizeof(*CellsToAddModList));

  CellsToAddModList  = MeasObj->measObject.choice.measObjectEUTRA.cellsToAddModList;

  // Add adjacent cell lists (6 per eNB)
  for (i=0;i<6;i++) {
    CellToAdd                       = (CellsToAddMod_t *)CALLOC(1,sizeof(*CellToAdd));
    CellToAdd->cellIndex            = i+1;
    CellToAdd->physCellId           = get_adjacent_cell_id(Mod_id,i);
    CellToAdd->cellIndividualOffset = Q_OffsetRange_dB0;

    ASN_SEQUENCE_ADD(&CellsToAddModList->list,CellToAdd);
  }

  ASN_SEQUENCE_ADD(&MeasObj_list->list,MeasObj);
  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measObjectToAddModList = MeasObj_list;

  // Report Configurations for periodical, A1-A5 events
  ReportConfig_list = CALLOC(1,sizeof(*ReportConfig_list));
  memset((void *)ReportConfig_list,0,sizeof(*ReportConfig_list));

  ReportConfig_per  = CALLOC(1,sizeof(*ReportConfig_per));
  memset((void *)ReportConfig_per,0,sizeof(*ReportConfig_per));

  ReportConfig_A1   = CALLOC(1,sizeof(*ReportConfig_A1));
  memset((void *)ReportConfig_A1,0,sizeof(*ReportConfig_A1));

  ReportConfig_A2   = CALLOC(1,sizeof(*ReportConfig_A2));
  memset((void *)ReportConfig_A2,0,sizeof(*ReportConfig_A2));

  ReportConfig_A3   = CALLOC(1,sizeof(*ReportConfig_A3));
  memset((void *)ReportConfig_A3,0,sizeof(*ReportConfig_A3));

  ReportConfig_A4   = CALLOC(1,sizeof(*ReportConfig_A4));
  memset((void *)ReportConfig_A4,0,sizeof(*ReportConfig_A4));

  ReportConfig_A5   = CALLOC(1,sizeof(*ReportConfig_A5));
  memset((void *)ReportConfig_A5,0,sizeof(*ReportConfig_A5));

  ReportConfig_per->reportConfigId                                                              = 1;
  ReportConfig_per->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.triggerType.present                   = ReportConfigEUTRA__triggerType_PR_periodical;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.triggerType.choice.periodical.purpose = ReportConfigEUTRA__triggerType__periodical__purpose_reportStrongestCells;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_per);

  ReportConfig_A1->reportConfigId                                                              = 2;
  ReportConfig_A1->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA1;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA1.a1_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA1.a1_Threshold.choice.threshold_RSRP = 10;

  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A1);
  /*
  ReportConfig_A2->reportConfigId                                                              = 3;
  ReportConfig_A2->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA2;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA2.a2_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA2.a2_Threshold.choice.threshold_RSRP = 10;

  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A2);

  ReportConfig_A3->reportConfigId                                                              = 4;
  ReportConfig_A3->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA3;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.a3_Offset = 10;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.reportOnLeave = 1;

  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A3);

  ReportConfig_A4->reportConfigId                                                              = 5;
  ReportConfig_A4->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA4;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA4.a4_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA4.a4_Threshold.choice.threshold_RSRP = 10;

  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A4);

  ReportConfig_A5->reportConfigId                                                              = 6;
  ReportConfig_A5->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA5;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA5.a5_Threshold1.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA5.a5_Threshold2.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA5.a5_Threshold1.choice.threshold_RSRP = 10;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA5.a5_Threshold2.choice.threshold_RSRP = 10;

  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A5);
  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->reportConfigToAddModList = ReportConfig_list;
  */
  /*
  rsrp=CALLOC(1,sizeof(RSRP_Range_t));
  *rsrp=20;


  Sparams = CALLOC(1,sizeof(*Sparams));
  Sparams->present=MeasConfig__speedStatePars_PR_setup;
  Sparams->choice.setup.timeToTrigger_SF.sf_High=SpeedStateScaleFactors__sf_Medium_oDot75;
  Sparams->choice.setup.timeToTrigger_SF.sf_Medium=SpeedStateScaleFactors__sf_High_oDot5;
  Sparams->choice.setup.mobilityStateParameters.n_CellChangeHigh=10;
  Sparams->choice.setup.mobilityStateParameters.n_CellChangeMedium=5;
  Sparams->choice.setup.mobilityStateParameters.t_Evaluation=MobilityStateParameters__t_Evaluation_s60;
  Sparams->choice.setup.mobilityStateParameters.t_HystNormal=MobilityStateParameters__t_HystNormal_s120;

  speedStatePars=Sparams;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->s_Measure=rsrp;

  */

#ifdef Rel10
  if (sCellIndexToAdd != (uint8_t)MAX_U8) {

 	  sCell1_config_ptr = CALLOC(1,sizeof(SCellToAddMod_r10_t));
	  eNB_rrc_inst[Mod_id].sCell_config[UE_index][0] = sCell1_config_ptr;

	  //Check all initialized values
	  sCell1_config_ptr->sCellIndex_r10 = 1;
	  sCell1_config_ptr->cellIdentification_r10 = CALLOC(1,sizeof(*sCell1_config_ptr->cellIdentification_r10));
	  sCell1_config_ptr->cellIdentification_r10->physCellId_r10 = 1;
	  sCell1_config_ptr->cellIdentification_r10->dl_CarrierFreq_r10 = 6425; //36126; //6425;

	  radioResourceConfigDedicatedSCell = CALLOC(1,sizeof(*radioResourceConfigDedicatedSCell));

	  radioResourceConfigCommonSCell = CALLOC(1,sizeof(*radioResourceConfigCommonSCell));

	  physicalConfigDedicatedSCell_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10));

	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10));
	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->antennaInfo_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->antennaInfo_r10));
	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->antennaInfo_r10->transmissionMode_r10 = 1;
	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->antennaInfo_r10->codebookSubsetRestriction_r10 = NULL;
	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->antennaInfo_r10->ue_TransmitAntennaSelection.present = AntennaInfoDedicated_r10__ue_TransmitAntennaSelection_PR_setup;
	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->antennaInfo_r10->ue_TransmitAntennaSelection.choice.setup = AntennaInfoDedicated_r10__ue_TransmitAntennaSelection__setup_closedLoop;

	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->crossCarrierSchedulingConfig_r10 = NULL; //CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->crossCarrierSchedulingConfig_r10));
	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->pdsch_ConfigDedicated_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->pdsch_ConfigDedicated_r10));
	  physicalConfigDedicatedSCell_r10->nonUL_Configuration_r10->pdsch_ConfigDedicated_r10->p_a = PDSCH_ConfigDedicated__p_a_dB0;

	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10));
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->soundingRS_UL_ConfigDedicated_r10 = NULL, //CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->soundingRS_UL_ConfigDedicated_r10));
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->soundingRS_UL_ConfigDedicated_v1020 = NULL; //CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->soundingRS_UL_ConfigDedicated_v1020));
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->soundingRS_UL_ConfigDedicatedAperiodic_r10 = NULL; //CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->soundingRS_UL_ConfigDedicatedAperiodic_r10));

	  //pusch_ConfigDedicatedSCell_r10
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10));
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->p0_UE_PUSCH_r10 = 0; // 0 dB
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->deltaMCS_Enabled_r10 = \
	    UplinkPowerControlDedicated__deltaMCS_Enabled_en1;
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->accumulationEnabled_r10 = 1; // FALSE
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->pSRS_Offset_r10 =  0; // 0 dB
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->pSRS_OffsetAp_r10 = NULL;
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->filterCoefficient_r10 = \
	    CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->filterCoefficient_r10));
	  *(physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->filterCoefficient_r10) = \
	    FilterCoefficient_fc4; //4db
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->uplinkPowerControlDedicatedSCell_r10->pathlossReferenceLinking_r10 = 0; // Verify this value!

	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->antennaInfoUL_r10 = \
	    CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->antennaInfoUL_r10));
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->antennaInfoUL_r10->transmissionModeUL_r10 = \
	    CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->antennaInfoUL_r10->transmissionModeUL_r10));
	  *(physicalConfigDedicatedSCell_r10->ul_Configuration_r10->antennaInfoUL_r10->transmissionModeUL_r10) = \
	    AntennaInfoUL_r10__transmissionModeUL_r10_tm1;
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->antennaInfoUL_r10->fourAntennaPortActivated_r10 = \
	    CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->antennaInfoUL_r10->fourAntennaPortActivated_r10));
	  *(physicalConfigDedicatedSCell_r10->ul_Configuration_r10->antennaInfoUL_r10->fourAntennaPortActivated_r10) = \
	    AntennaInfoUL_r10__fourAntennaPortActivated_r10_setup;

	  //PUSCH_ConfigDedicatedSCell_r10
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->pusch_ConfigDedicatedSCell_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->pusch_ConfigDedicatedSCell_r10));
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->pusch_ConfigDedicatedSCell_r10->groupHoppingDisabled_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->pusch_ConfigDedicatedSCell_r10->groupHoppingDisabled_r10));
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->pusch_ConfigDedicatedSCell_r10->dmrs_WithOCC_Activated_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->pusch_ConfigDedicatedSCell_r10->dmrs_WithOCC_Activated_r10));

	  //cqi_ReportModeAperiodic_r10
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10));
	  //physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->cqi_ReportModeAperiodic_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->cqi_ReportModeAperiodic_r10));
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->cqi_ReportModeAperiodic_r10 = NULL;
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->nomPDSCH_RS_EPRE_Offset_r10 = NULL;
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->cqi_ReportPeriodicSCell_r10 = NULL;
	  physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->pmi_RI_Report_r10 = NULL;
	  //physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->cqi_ReportPeriodicSCell_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->cqi_ReportPeriodicSCell_r10));
	  //physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->cqi_ReportPeriodicSCell_r10->present = CQI_ReportPeriodic_r10__setup__cqi_FormatIndicatorPeriodic_r10_PR_widebandCQI_r10;
	  //physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->cqi_ReportPeriodicSCell_r10->choice.widebandCQI_r10.csi_ReportMode_r10 = NULL; // calloc if reqd
	  //physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->pmi_RI_Report_r10 = CALLOC(1,sizeof(*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->pmi_RI_Report_r10));
	  //*physicalConfigDedicatedSCell_r10->ul_Configuration_r10->cqi_ReportConfigSCell_r10->pmi_RI_Report_r10 = init_val;

	  radioResourceConfigDedicatedSCell->physicalConfigDedicatedSCell_r10 = physicalConfigDedicatedSCell_r10;
	  sCell1_config_ptr->radioResourceConfigDedicatedSCell_r10 = radioResourceConfigDedicatedSCell;
	  sCell1_config_ptr->radioResourceConfigCommonSCell_r10 = radioResourceConfigCommonSCell; //Check this!!

	  // FK: this is now in asn1_msg.c 
	  /*
          sCellToAddList = CALLOC(1,sizeof(*sCellToAddList));
	  ASN_SEQUENCE_ADD(&sCellToAddList->list,sCell1_config_ptr);
	  */
	  LOG_W(RRC,"Adding SCell configuration in RRC Reconfig Req with index %d ...\n",sCell1_config_ptr->sCellIndex_r10);
  }
  else {
	  msg("RRCConnectionReconfiguration SCell addition failed: Not enough SCell resources");
  }
#endif


  size = do_RRCConnectionReconfiguration(Mod_id,
                                         buffer,
                                         UE_index,
                                         0,//Transaction_id,
                                         SRB_list,
                                         DRB_list,
                                         NULL, // DRB2_list,
                                         NULL, //*sps_Config,
                                         physicalConfigDedicated[UE_index],
#ifdef Rel10
					 eNB_rrc_inst[Mod_id].sCell_config[UE_index][sCellIndexToAdd],
#endif
                                         MeasObj_list,
                                         ReportConfig_list,
                                         NULL, //*QuantityConfig,
                                         MeasId_list,
                                         mac_MainConfig,
                                         NULL,
                                         nas_pdu,
                                         nas_length
                                        ); //*measGapConfig);

  LOG_I(RRC,"[eNB %d] Frame %d, Logical Channel DL-DCCH, Generate RRCConnectionReconfiguration (bytes %d, UE id %d)\n",
        Mod_id,frame, size, UE_index);


  LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (rrcConnectionReconfiguration to UE %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
        frame, Mod_id, size, UE_index, rrc_eNB_mui, Mod_id, (UE_index*MAX_NUM_RB)+DCCH);
  //rrc_rlc_data_req(Mod_id,frame, 1,(UE_index*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer);
  pdcp_data_req(Mod_id, frame, 1, (UE_index * MAX_NUM_RB) + DCCH, rrc_eNB_mui++, 0, size, (char*)buffer, 1);

}

void rrc_eNB_process_MeasurementReport(u8 Mod_id,u16 UE_index,MeasResults_t	 *measResults2) {

  LOG_I(RRC,"Received Measurement Report From UE %d (Measurement Id %d)\n",UE_index,(int)measResults2->measId);
  if (measResults2->measResultNeighCells->choice.measResultListEUTRA.list.count>0) {
    LOG_I(RRC,"Physical Cell Id %d\n",(int)measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->physCellId);
    LOG_I(RRC,"RSRP of Target %d\n",(int)*(measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->measResult.rsrpResult));
    LOG_I(RRC,"RSRQ of Target %d\n",(int)*(measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->measResult.rsrqResult));
  }
#ifdef Rel10
  LOG_I(RRC,"RSRP of Source %d\n",measResults2->measResultPCell.rsrpResult);
  LOG_I(RRC,"RSRQ of Source %d\n",measResults2->measResultPCell.rsrqResult);
#else
  LOG_I(RRC,"RSRP of Source %d\n",measResults2->measResultServCell.rsrpResult);
  LOG_I(RRC,"RSRQ of Source %d\n",measResults2->measResultServCell.rsrqResult);
#endif

  //Look for IP address of the target eNB
  //Send Handover Request -> target eNB
  //Wait for Handover Acknowledgement <- target eNB
  //Send Handover Command

  //x2delay();
  //	handover_request_x2(UE_index,Mod_id,measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->physCellId);

  //	u8 buffer[100];
  //    int size=rrc_eNB_generate_Handover_Command_TeNB(0,0,buffer);
//
//	  send_check_message((char*)buffer,size);
  //send_handover_command();

}

void rrc_eNB_process_RRCConnectionReconfigurationComplete(u8 Mod_id,u32 frame,u8 UE_index,RRCConnectionReconfigurationComplete_r8_IEs_t *rrcConnectionReconfigurationComplete){
  int i;
  int oip_ifup=0;
  int dest_ip_offset=0;

#ifdef Rel10
  /* Rel 10 optional parameters - not sure what to do with these */
  if(rrcConnectionReconfigurationComplete->nonCriticalExtension->nonCriticalExtension != NULL) {
    eNB_rrc_inst[Mod_id].rrcRel10IEs[0] = malloc(sizeof(RRCConnectionReconfigurationComplete_v1020_IEs_t));
    eNB_rrc_inst[Mod_id].rrcRel10IEs[0]->logMeasAvailable_r10 = rrcConnectionReconfigurationComplete->nonCriticalExtension->nonCriticalExtension->logMeasAvailable_r10;
    eNB_rrc_inst[Mod_id].rrcRel10IEs[0]->rlf_InfoAvailable_r10 = rrcConnectionReconfigurationComplete->nonCriticalExtension->nonCriticalExtension->rlf_InfoAvailable_r10;
  }
#endif

  // Loop through DRBs and establish if necessary
  for (i=0;i<8;i++) { // num max DRB (11-3-8)
    if (eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]) {
      LOG_I(RRC,"[eNB %d] Frame  %d : Logical Channel UL-DCCH, Received RRCConnectionReconfigurationComplete from UE %d, reconfiguring DRB %d/LCID %d\n",
	    Mod_id,frame, UE_index,
	  (int)eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->drb_Identity,
	  (UE_index * MAX_NUM_RB) + (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->logicalChannelIdentity);
      if (eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] == 0) {
	rrc_pdcp_config_req (Mod_id, frame, 1, ACTION_ADD,
			     (UE_index * MAX_NUM_RB) + (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity,UNDEF_SECURITY_MODE);
	rrc_rlc_config_req(Mod_id,frame,1,ACTION_ADD,
			   (UE_index * MAX_NUM_RB) + (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity,
			   RADIO_ACCESS_BEARER,Rlc_info_um);
	eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] = 1;

	LOG_D(RRC,"[eNB %d] Frame %d: Establish RLC UM Bidirectional, DRB %d Active\n",
	      Mod_id, frame, (int)eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->drb_Identity);
	
#ifdef NAS_NETLINK
// can mean also IPV6 since ether -> ipv6 autoconf
#    ifndef OAI_NW_DRIVER_TYPE_ETHERNET
	LOG_I(OIP,"[eNB %d] trying to bring up the OAI interface oai%d\n", Mod_id, Mod_id);
	oip_ifup = nas_config(Mod_id,// interface index
		   Mod_id+1, // thrid octet
		   Mod_id+1);// fourth octet

	 if (oip_ifup == 0 ){ // interface is up --> send a config the DRB
#        ifdef OAI_EMU
	  oai_emulation.info.oai_ifup[Mod_id]=1;
	  dest_ip_offset=NB_eNB_INST;
#        else
	  dest_ip_offset=8;
#        endif
	  LOG_I(OIP,"[eNB %d] Config the oai%d to send/receive pkt on DRB %d to/from the protocol stack\n",
		Mod_id,
		Mod_id,
		(UE_index * MAX_NUM_RB) + *eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity);
	  rb_conf_ipv4(0,//add
		       UE_index, //cx
		       Mod_id,//inst
		       (UE_index * MAX_NUM_RB) + *eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity,
		       0,//dscp
		       ipv4_address(Mod_id+1,Mod_id+1),//saddr
		       ipv4_address(Mod_id+1,dest_ip_offset+UE_index+1));//daddr

	   LOG_D(RRC,"[eNB %d] State = Attached (UE %d)\n",Mod_id,UE_index);
	 }
#    else
#        ifdef OAI_EMU
      oai_emulation.info.oai_ifup[Mod_id]=1;
#        endif
#    endif
#endif
      
	LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (DRB UE %d) --->][MAC_eNB][MOD %02d][]\n",
	      frame, Mod_id, UE_index, Mod_id);
	DRB2LCHAN[i] = (u8)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity;
	rrc_mac_config_req(Mod_id,1,UE_index,0,
			   (RadioResourceConfigCommonSIB_t *)NULL,
			   eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
#ifdef Rel10
			   eNB_rrc_inst[Mod_id].sCell_config[UE_index][0],
#endif
			   (MeasObjectToAddMod_t **)NULL,
			   eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
			   DRB2LCHAN[i],
			   eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelConfig,
			   eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
			   (TDD_Config_t *)NULL,
			   (u8 *)NULL,
			   (u16 *)NULL,
			   NULL,
			   NULL,
			   NULL,
			   (MBSFN_SubframeConfigList_t *)NULL
#ifdef Rel10	       
			   ,
			   0,
			   (MBSFN_AreaInfoList_r9_t *)NULL
#endif
			   );
      }
      
      else { // remove LCHAN from MAC/PHY
	/*
	if (eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] ==1) {
	  // DRB has just been removed so remove RLC + PDCP for DRB
	  rrc_pdcp_config_req (Mod_id, frame, 1, ACTION_REMOVE,
			       (UE_index * MAX_NUM_RB) + DRB2LCHAN[i],UNDEF_SECURITY_MODE);
	  rrc_rlc_config_req(Mod_id,frame,1,ACTION_REMOVE,
			     (UE_index * MAX_NUM_RB) + DRB2LCHAN[i],
			     RADIO_ACCESS_BEARER,Rlc_info_um);
	}
	eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] = 0;
    LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (DRB UE %d) --->][MAC_eNB][MOD %02d][]\n",
            frame, Mod_id, UE_index, Mod_id);
	*/
      }

    }
  }

#ifdef Rel10
  if ( eNB_rrc_inst[Mod_id].sCell_config[UE_index][0]) {
    rrc_mac_config_req(Mod_id,1,UE_index,0,
		       (RadioResourceConfigCommonSIB_t *)NULL,
		       NULL,
		       eNB_rrc_inst[Mod_id].sCell_config[UE_index][0],
		       (MeasObjectToAddMod_t **)NULL,
		       NULL,
		       NULL,
		       (LogicalChannelConfig_t *)NULL,
		       (MeasGapConfig_t *)NULL,
		       (TDD_Config_t *)NULL,
		       (u8 *)NULL,
		       (u16 *)NULL,
		       NULL,
		       NULL,
		       NULL,
		       NULL,
		       0,
		       (MBSFN_AreaInfoList_r9_t *)NULL);
#endif
    
  }
}

void rrc_eNB_generate_RRCConnectionSetup(u8 Mod_id,u32 frame, u16 UE_index) {

  LogicalChannelConfig_t *SRB1_logicalChannelConfig;//,*SRB2_logicalChannelConfig;

  eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.payload_size =
    do_RRCConnectionSetup((u8 *)eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.Payload,
			  mac_xface->get_transmission_mode(Mod_id,find_UE_RNTI(Mod_id,UE_index)),
			  UE_index,0,
			  mac_xface->lte_frame_parms,
			  &eNB_rrc_inst[Mod_id].SRB1_config[UE_index],
			  &eNB_rrc_inst[Mod_id].SRB2_config[UE_index],
			  &eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);

    // configure SRB1/SRB2, PhysicalConfigDedicated, MAC_MainConfig for UE

  if (eNB_rrc_inst[Mod_id].SRB1_config[UE_index]->logicalChannelConfig) {
    if (eNB_rrc_inst[Mod_id].SRB1_config[UE_index]->logicalChannelConfig->present == SRB_ToAddMod__logicalChannelConfig_PR_explicitValue) {
      SRB1_logicalChannelConfig = &eNB_rrc_inst[Mod_id].SRB1_config[UE_index]->logicalChannelConfig->choice.explicitValue;
    }
    else {
      SRB1_logicalChannelConfig = &SRB1_logicalChannelConfig_defaultValue;
    }
  }
  else {
    SRB1_logicalChannelConfig = &SRB1_logicalChannelConfig_defaultValue;
  }

  /*
  if (eNB_rrc_inst[Mod_id].SRB2_config[UE_index]->logicalChannelConfig) {
    if (eNB_rrc_inst[Mod_id].SRB2_config[UE_index]->logicalChannelConfig->present == SRB_ToAddMod__logicalChannelConfig_PR_explicitValue) {
      SRB2_logicalChannelConfig = &eNB_rrc_inst[Mod_id].SRB2_config[UE_index]->logicalChannelConfig->choice.explicitValue;
    }
    else {
      SRB2_logicalChannelConfig = &SRB2_logicalChannelConfig_defaultValue;
    }
  }
  else {
    SRB2_logicalChannelConfig  = &SRB2_logicalChannelConfig_defaultValue;
  }
  */

  LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (SRB1 UE %d) --->][MAC_eNB][MOD %02d][]\n",
            frame, Mod_id, UE_index, Mod_id);
  rrc_mac_config_req(Mod_id,1,UE_index,0,
		     (RadioResourceConfigCommonSIB_t *)NULL,
		     eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
#ifdef Rel10
		     (SCellToAddMod_r10_t *)NULL,
#endif
		     (MeasObjectToAddMod_t **)NULL,
		     eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
		     1,
		     SRB1_logicalChannelConfig,
		     eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
		     (TDD_Config_t *)NULL,
		     (u8 *)NULL,
		     (u16 *)NULL,
		     NULL,
		     NULL,
		     NULL,
		     (MBSFN_SubframeConfigList_t *)NULL
#ifdef Rel10	       
		     ,
		     0,

		     (MBSFN_AreaInfoList_r9_t *)NULL
#endif
		     );

  LOG_I(RRC,"[eNB %d][RAPROC] Frame %d : Logical Channel DL-CCCH, Generating RRCConnectionSetup (bytes %d, UE %d)\n",
	Mod_id,frame,eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.payload_size, UE_index);

}

/*
void ue_rrc_process_rrcConnectionReconfiguration(u8 Mod_id,u32 frame,
						 RRCConnectionReconfiguration_t *rrcConnectionReconfiguration,
						 u8 CH_index) {

  if (rrcConnectionReconfiguration->criticalExtensions.present == RRCConnectionReconfiguration__criticalExtensions_PR_c1)
    if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.present == RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8) {

      if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated) {
	rrc_ue_process_radioResourceConfigDedicated(Mod_id,frame,CH_index,
						    rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated);


      }

      // check other fields for
    }
}
*/
#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif