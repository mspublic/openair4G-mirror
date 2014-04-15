/*******************************************************************************
Eurecom OpenAirInterface 2
Copyright(c) 1999 - 2014 Eurecom

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
Address      : EURECOM,
               Campus SophiaTech,
               450 Route des Chappes,
               CS 50193
               06904 Biot Sophia Antipolis cedex,
               FRANCE
*******************************************************************************/
/*
                                rlc_rrc.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier at eurecom dot fr
*/

#define RLC_RRC_C
#include "rlc.h"
#include "rlc_am.h"
#include "rlc_um.h"
#include "rlc_tm.h"
#include "UTIL/LOG/log.h"
#ifdef OAI_EMU
#include "UTIL/OCG/OCG_vars.h"
#endif
#include "RLC-Config.h"
#include "DRB-ToAddMod.h"
#include "DRB-ToAddModList.h"
#include "SRB-ToAddMod.h"
#include "SRB-ToAddModList.h"
#include "DL-UM-RLC.h"
#ifdef Rel10
#include "PMCH-InfoList-r9.h"
#endif

#include "LAYER2/MAC/extern.h"
#include "assertions.h"
//-----------------------------------------------------------------------------
rlc_op_status_t rrc_rlc_config_asn1_req (const module_id_t           enb_mod_idP,
                                         const module_id_t           ue_mod_idP,
                                         const frame_t               frameP,
                                         const eNB_flag_t            enb_flagP,
                                         const SRB_ToAddModList_t   * const srb2add_listP,
                                         const DRB_ToAddModList_t   * const drb2add_listP,
                                         const DRB_ToReleaseList_t  * const drb2release_listP
#if defined(Rel10)
                                        ,const PMCH_InfoList_r9_t * const pmch_InfoList_r9_pP
#endif
  ) {
  //-----------------------------------------------------------------------------
  rb_id_t                rb_id           = 0;
  logical_chan_id_t      lc_id           = 0;
  DRB_Identity_t         drb_id          = 0;
  DRB_Identity_t*        pdrb_id         = NULL;
  long int               cnt             = 0;
  const SRB_ToAddMod_t  *srb_toaddmod_p  = NULL;
  const DRB_ToAddMod_t  *drb_toaddmod_p  = NULL;
  rlc_union_t           *rlc_union_p     = NULL;
  hash_key_t             key             = HASHTABLE_QUESTIONABLE_KEY_VALUE;
  hashtable_rc_t         h_rc;
#if defined(Rel10)
  int                        i, j;
  MBMS_SessionInfoList_r9_t *mbms_SessionInfoList_r9_p = NULL;
  MBMS_SessionInfo_r9_t     *MBMS_SessionInfo_p        = NULL;
  mbms_session_id_t          mbms_session_id;
  mbms_service_id_t          mbms_service_id;
  DL_UM_RLC_t                dl_um_rlc;


#endif

  LOG_D(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] CONFIG REQ ASN1 \n",
          frameP,
          (enb_flagP) ? "eNB" : "UE",
          enb_mod_idP,
          ue_mod_idP);

#ifdef OAI_EMU
    if (enb_flagP) {
        AssertFatal ((enb_mod_idP >= oai_emulation.info.first_enb_local) && (oai_emulation.info.nb_enb_local > 0),
            "eNB module id is too low (%u/%d)!\n",
            enb_mod_idP,
            oai_emulation.info.first_enb_local);
        AssertFatal ((enb_mod_idP < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local)) && (oai_emulation.info.nb_enb_local > 0),
            "eNB module id is too high (%u/%d)!\n",
            enb_mod_idP,
            oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local);
        AssertFatal (ue_mod_idP  < NB_UE_INST,
            "UE module id is too high (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
    } else {
        AssertFatal (ue_mod_idP  < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local),
            "UE module id is too high (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
        AssertFatal (ue_mod_idP  >= oai_emulation.info.first_ue_local,
            "UE module id is too low (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local);
    }
#endif
  if (srb2add_listP != NULL) {
      for (cnt=0;cnt<srb2add_listP->list.count;cnt++) {
         rb_id = srb2add_listP->list.array[cnt]->srb_Identity;
         lc_id  = rb_id + 2;

         LOG_D(RLC, "Adding SRB %d, rb_id %d\n",srb2add_listP->list.array[cnt]->srb_Identity,rb_id);
          srb_toaddmod_p = srb2add_listP->list.array[cnt];

          if (srb_toaddmod_p->rlc_Config) {
              switch (srb_toaddmod_p->rlc_Config->present) {
                  case SRB_ToAddMod__rlc_Config_PR_NOTHING:
                      break;
                  case SRB_ToAddMod__rlc_Config_PR_explicitValue:
                      switch (srb_toaddmod_p->rlc_Config->choice.explicitValue.present) {
                          case RLC_Config_PR_NOTHING:
                              break;
                          case RLC_Config_PR_am:
                              if (rrc_rlc_add_rlc (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_YES, MBMS_FLAG_NO, rb_id, lc_id, RLC_MODE_AM) != NULL) {
                                  config_req_rlc_am_asn1 (
                                      enb_mod_idP,
                                      ue_mod_idP,
                                      frameP,
                                      enb_flagP,
                                      SRB_FLAG_YES,
                                      &srb_toaddmod_p->rlc_Config->choice.explicitValue.choice.am,
                                      rb_id);
                              } else {
                                  LOG_E(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] ERROR IN ALLOCATING SRB %d \n",
                                      frameP,
                                      (enb_flagP) ? "eNB" : "UE",
                                      enb_mod_idP,
                                      ue_mod_idP,
                                      rb_id);
                              }
                              break;
                          case RLC_Config_PR_um_Bi_Directional:
                              if (rrc_rlc_add_rlc (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_YES, MBMS_FLAG_NO, rb_id, lc_id, RLC_MODE_UM) != NULL) {
                                  config_req_rlc_um_asn1(
                                      enb_mod_idP,
                                      ue_mod_idP,
                                      frameP,
                                      enb_flagP,
                                      SRB_FLAG_YES,
                                      MBMS_FLAG_NO,
                                      UNUSED_PARAM_MBMS_SESSION_ID,
                                      UNUSED_PARAM_MBMS_SERVICE_ID,
                                      &srb_toaddmod_p->rlc_Config->choice.explicitValue.choice.um_Bi_Directional.ul_UM_RLC,
                                      &srb_toaddmod_p->rlc_Config->choice.explicitValue.choice.um_Bi_Directional.dl_UM_RLC,
                                      rb_id);
                              } else {
                                  LOG_E(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] ERROR IN ALLOCATING SRB %d \n",
                                      frameP,
                                      (enb_flagP) ? "eNB" : "UE",
                                      enb_mod_idP,
                                      ue_mod_idP,
                                      rb_id);
                              }
                              break;
                          case RLC_Config_PR_um_Uni_Directional_UL:
                              if (rrc_rlc_add_rlc (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_YES, MBMS_FLAG_NO, rb_id, lc_id, RLC_MODE_UM) != NULL) {
                                  config_req_rlc_um_asn1(
                                      enb_mod_idP,
                                      ue_mod_idP,
                                      frameP,
                                      enb_flagP,
                                      SRB_FLAG_YES,
                                      MBMS_FLAG_NO,
                                      UNUSED_PARAM_MBMS_SESSION_ID,
                                      UNUSED_PARAM_MBMS_SERVICE_ID,
                                      &srb_toaddmod_p->rlc_Config->choice.explicitValue.choice.um_Uni_Directional_UL.ul_UM_RLC,
                                      NULL,
                                      rb_id);
                              } else {
                                  LOG_E(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] ERROR IN ALLOCATING SRB %d \n",
                                      frameP,
                                      (enb_flagP) ? "eNB" : "UE",
                                      enb_mod_idP,
                                      ue_mod_idP,
                                      rb_id);
                              }
                              break;
                          case RLC_Config_PR_um_Uni_Directional_DL:
                              if (rrc_rlc_add_rlc (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_YES, MBMS_FLAG_NO, rb_id, lc_id, RLC_MODE_UM) != NULL) {
                                  config_req_rlc_um_asn1(
                                      enb_mod_idP,
                                      ue_mod_idP,
                                      frameP,
                                      enb_flagP,
                                      SRB_FLAG_YES,
                                      MBMS_FLAG_NO,
                                      UNUSED_PARAM_MBMS_SESSION_ID,
                                      UNUSED_PARAM_MBMS_SERVICE_ID,
                                      NULL,
                                      &srb_toaddmod_p->rlc_Config->choice.explicitValue.choice.um_Uni_Directional_DL.dl_UM_RLC,
                                      rb_id);
                              } else {
                                  LOG_E(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] ERROR IN ALLOCATING SRB %d \n",
                                      frameP,
                                      (enb_flagP) ? "eNB" : "UE",
                                       enb_mod_idP,
                                       ue_mod_idP,
                                       rb_id);
                              }
                              break;

                          default:
                              LOG_E(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] UNKNOWN RLC CONFIG %d \n",
                                      frameP,
                                      (enb_flagP) ? "eNB" : "UE",
                                      enb_mod_idP,
                                      ue_mod_idP,
                                      srb_toaddmod_p->rlc_Config->choice.explicitValue.present);
                              break;
                      }
                      break;
                  case SRB_ToAddMod__rlc_Config_PR_defaultValue:
#warning TO DO SRB_ToAddMod__rlc_Config_PR_defaultValue
                    if (rrc_rlc_add_rlc   (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_YES, MBMS_FLAG_NO, rb_id, lc_id, RLC_MODE_UM) != NULL) {
                        config_req_rlc_um_asn1(
                            enb_mod_idP,
                            ue_mod_idP,
                            frameP,
                            enb_flagP,
                            SRB_FLAG_YES,
                            MBMS_FLAG_NO,
                            UNUSED_PARAM_MBMS_SESSION_ID,
                            UNUSED_PARAM_MBMS_SERVICE_ID,
                            NULL, // TO DO DEFAULT CONFIG
                            NULL, // TO DO DEFAULT CONFIG
                            rb_id);
                      } else {
                          LOG_D(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] ERROR IN ALLOCATING SRB %d \n",
                                  frameP,
                                  (enb_flagP) ? "eNB" : "UE",
                                  enb_mod_idP,
                                  ue_mod_idP,
                                  rb_id);
                      }
                      break;
                  default:;
              }
          }
      }
  }
  if (drb2add_listP != NULL) {
      for (cnt=0;cnt<drb2add_listP->list.count;cnt++) {
          drb_toaddmod_p = drb2add_listP->list.array[cnt];

          drb_id = drb_toaddmod_p->drb_Identity;
          lc_id  = drb_id + 2;

          LOG_D(RLC, "Adding DRB %d, lc_id %d\n",drb_id,lc_id);


          if (drb_toaddmod_p->rlc_Config) {

              switch (drb_toaddmod_p->rlc_Config->present) {
                  case RLC_Config_PR_NOTHING:
                      break;
                  case RLC_Config_PR_am:
                      if (rrc_rlc_add_rlc (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_NO, MBMS_FLAG_NO, drb_id, lc_id, RLC_MODE_AM) != NULL) {
                          config_req_rlc_am_asn1 (
                              enb_mod_idP,
                              ue_mod_idP,
                              frameP,
                              enb_flagP,
                              SRB_FLAG_NO,
                              &drb_toaddmod_p->rlc_Config->choice.am,
                              drb_id);
                      }
                      break;
                  case RLC_Config_PR_um_Bi_Directional:
                      if (rrc_rlc_add_rlc (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_NO, MBMS_FLAG_NO, drb_id, lc_id, RLC_MODE_UM) != NULL) {
                          config_req_rlc_um_asn1(
                              enb_mod_idP,
                              ue_mod_idP,
                              frameP,
                              enb_flagP,
                              SRB_FLAG_NO,
                              MBMS_FLAG_NO,
                              UNUSED_PARAM_MBMS_SESSION_ID,
                              UNUSED_PARAM_MBMS_SERVICE_ID,
                              &drb_toaddmod_p->rlc_Config->choice.um_Bi_Directional.ul_UM_RLC,
                              &drb_toaddmod_p->rlc_Config->choice.um_Bi_Directional.dl_UM_RLC,
                              drb_id);
                      }
                      break;
                  case RLC_Config_PR_um_Uni_Directional_UL:
                      if (rrc_rlc_add_rlc (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_NO, MBMS_FLAG_NO, drb_id, lc_id, RLC_MODE_UM) != NULL) {
                          config_req_rlc_um_asn1(
                              enb_mod_idP,
                              ue_mod_idP,
                              frameP,
                              enb_flagP,
                              SRB_FLAG_NO,
                              MBMS_FLAG_NO,
                              UNUSED_PARAM_MBMS_SESSION_ID,
                              UNUSED_PARAM_MBMS_SERVICE_ID,
                              &drb_toaddmod_p->rlc_Config->choice.um_Uni_Directional_UL.ul_UM_RLC,
                              NULL,
                              drb_id);
                      }
                      break;
                  case RLC_Config_PR_um_Uni_Directional_DL:
                      if (rrc_rlc_add_rlc (enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_NO, MBMS_FLAG_NO, drb_id, lc_id, RLC_MODE_UM) != NULL) {
                          config_req_rlc_um_asn1(
                              enb_mod_idP,
                              ue_mod_idP,
                              frameP,
                              enb_flagP,
                              SRB_FLAG_NO,
                              MBMS_FLAG_NO,
                              UNUSED_PARAM_MBMS_SESSION_ID,
                              UNUSED_PARAM_MBMS_SERVICE_ID,
                              NULL,
                              &drb_toaddmod_p->rlc_Config->choice.um_Uni_Directional_DL.dl_UM_RLC,
                              drb_id);
                      }
                      break;
                  default:
                      LOG_W(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u][RB %u] unknown drb_toaddmod_p->rlc_Config->present \n",
                              frameP,
                              (enb_flagP) ? "eNB" : "UE",
                              enb_mod_idP,
                              ue_mod_idP,
                              drb_id);
              }
          }
      }
  }
  if (drb2release_listP != NULL) {
      for (cnt=0;cnt<drb2release_listP->list.count;cnt++) {
          pdrb_id = drb2release_listP->list.array[cnt];
          rrc_rlc_remove_rlc(enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_NO, MBMS_FLAG_NO, *pdrb_id);
      }
  }

#if defined(Rel10)

  if (pmch_InfoList_r9_pP != NULL) {
      for (i=0;i<pmch_InfoList_r9_pP->list.count;i++) {
          mbms_SessionInfoList_r9_p = &(pmch_InfoList_r9_pP->list.array[i]->mbms_SessionInfoList_r9);
          for (j=0;j<mbms_SessionInfoList_r9_p->list.count;j++) {
              MBMS_SessionInfo_p = mbms_SessionInfoList_r9_p->list.array[j];
              mbms_session_id    = MBMS_SessionInfo_p->sessionId_r9->buf[0];
              lc_id              = mbms_session_id;
              mbms_service_id    = MBMS_SessionInfo_p->tmgi_r9.serviceId_r9.buf[2]; //serviceId is 3-octet string

              // can set the mch_id = i
              if (enb_flagP) {
                rb_id =  (mbms_service_id * maxSessionPerPMCH ) + mbms_session_id;//+ (maxDRB + 3) * MAX_MOBILES_PER_ENB; // 1
                rlc_mbms_lcid2service_session_id_eNB[enb_mod_idP][lc_id].service_id                     = mbms_service_id;
                rlc_mbms_lcid2service_session_id_eNB[enb_mod_idP][lc_id].session_id                     = mbms_session_id;

                rlc_mbms_enb_set_lcid_by_rb_id(enb_mod_idP,rb_id,lc_id);
              } else {
                rb_id =  (mbms_service_id * maxSessionPerPMCH ) + mbms_session_id; // + (maxDRB + 3); // 15
                rlc_mbms_lcid2service_session_id_ue[ue_mod_idP][lc_id].service_id                    = mbms_service_id;
                rlc_mbms_lcid2service_session_id_ue[ue_mod_idP][lc_id].session_id                    = mbms_session_id;
                rlc_mbms_ue_set_lcid_by_rb_id(ue_mod_idP,rb_id,lc_id);
              }
              key = RLC_COLL_KEY_MBMS_VALUE(enb_mod_idP, ue_mod_idP, enb_flagP, mbms_service_id, mbms_session_id);

              h_rc = hashtable_get(rlc_coll_p, key, (void**)&rlc_union_p);
              if (h_rc == HASH_TABLE_KEY_NOT_EXISTS) {
                  rlc_union_p = rrc_rlc_add_rlc   (
                      enb_mod_idP,
                      ue_mod_idP,
                      frameP,
                      enb_flagP,
                      SRB_FLAG_NO,
                      MBMS_FLAG_YES,
                      rb_id,
                      lc_id,
                      RLC_MODE_UM);
                  AssertFatal(rlc_union_p != NULL, "ADD MBMS RLC UM FAILED");
              }

              LOG_D(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] CONFIG REQ MBMS ASN1 LC ID %u RB ID %u SESSION ID %u SERVICE ID %u\n",
                    frameP,
                    (enb_flagP) ? "eNB" : "UE",
                    enb_mod_idP,
                    ue_mod_idP,
                    lc_id,
                    rb_id,
                    mbms_session_id,
                    mbms_service_id
                   );
              dl_um_rlc.sn_FieldLength = SN_FieldLength_size5;
              dl_um_rlc.t_Reordering   = T_Reordering_ms0;

              config_req_rlc_um_asn1 (
                  enb_mod_idP,
                  ue_mod_idP,
                  frameP,
                  enb_flagP,
                  SRB_FLAG_NO,
                  MBMS_FLAG_YES,
                  mbms_session_id,
                  mbms_service_id,
                  NULL,
                  &dl_um_rlc,
                  rb_id);
          }
      }
  }
#endif

  LOG_D(RLC, "[FRAME %5u][%s][RLC_RRC][MOD %u/%u] CONFIG REQ ASN1 END \n",
         frameP,
         (enb_flagP) ? "eNB" : "UE",
         enb_flagP,
         enb_mod_idP,
         ue_mod_idP);
  return RLC_OP_STATUS_OK;
}
//-----------------------------------------------------------------------------
void
rb_free_rlc_union (void *rlcu_pP)
{
//-----------------------------------------------------------------------------
  rlc_union_t * rlcu_p;
    if (rlcu_pP) {
        rlcu_p = (rlc_union_t *)(rlcu_pP);
        switch (rlcu_p->mode) {
          case RLC_MODE_AM:
              rlc_am_cleanup(&rlcu_p->rlc.am);
              break;
          case RLC_MODE_UM:
              rlc_um_cleanup(&rlcu_p->rlc.um);
              break;
          case RLC_MODE_TM:
              rlc_tm_cleanup(&rlcu_p->rlc.tm);
              break;
          default:
            break;
        }
        free(rlcu_p);
    }
}

//-----------------------------------------------------------------------------
rlc_op_status_t rrc_rlc_remove_rlc   (
    const module_id_t enb_mod_idP,
    const module_id_t ue_mod_idP,
    const frame_t     frameP,
    const eNB_flag_t  enb_flagP,
    const srb_flag_t  srb_flagP,
    const MBMS_flag_t MBMS_flagP,
    const rb_id_t     rb_idP) {
//-----------------------------------------------------------------------------
    logical_chan_id_t      lcid            = 0;
    hash_key_t             key             = HASHTABLE_QUESTIONABLE_KEY_VALUE;
    hashtable_rc_t         h_rc;
#ifdef Rel10
    rlc_mbms_id_t         *mbms_id_p  = NULL;
#endif
#ifdef OAI_EMU
    AssertFatal ((enb_mod_idP >= oai_emulation.info.first_enb_local) && (oai_emulation.info.nb_enb_local > 0),
        "eNB module id is too low (%u/%d)!\n",
        enb_mod_idP,
        oai_emulation.info.first_enb_local);
    AssertFatal (enb_mod_idP < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local),
        "eNB module id is too high (%u/%d)!\n",
        enb_mod_idP,
        oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local);
    if (enb_flagP) {
        AssertFatal (ue_mod_idP  < NB_UE_INST,
            "UE module id is too high (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
    } else {
        AssertFatal (ue_mod_idP  < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local),
            "UE module id is too high (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
        AssertFatal (ue_mod_idP  >= oai_emulation.info.first_ue_local,
            "UE module id is too low (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local);
    }
#endif

#ifdef Rel10
  if (MBMS_flagP == TRUE) {
      if (enb_flagP) {
          lcid = rlc_mbms_enb_get_lcid_by_rb_id(enb_mod_idP,rb_idP);
          mbms_id_p = &rlc_mbms_lcid2service_session_id_eNB[enb_mod_idP][lcid];

          rlc_mbms_lcid2service_session_id_eNB[enb_mod_idP][lcid].service_id = 0;
          rlc_mbms_lcid2service_session_id_eNB[enb_mod_idP][lcid].session_id = 0;
          rlc_mbms_rbid2lcid_ue[enb_mod_idP][rb_idP] = RLC_LC_UNALLOCATED;
      } else {
          lcid = rlc_mbms_ue_get_lcid_by_rb_id(ue_mod_idP,rb_idP);
          mbms_id_p = &rlc_mbms_lcid2service_session_id_ue[ue_mod_idP][lcid];

          rlc_mbms_lcid2service_session_id_eNB[ue_mod_idP][lcid].service_id = 0;
          rlc_mbms_lcid2service_session_id_eNB[ue_mod_idP][lcid].session_id = 0;
          rlc_mbms_rbid2lcid_ue[ue_mod_idP][rb_idP] = RLC_LC_UNALLOCATED;
      }
      key = RLC_COLL_KEY_MBMS_VALUE(enb_mod_idP, ue_mod_idP, enb_flagP, mbms_id_p->service_id, mbms_id_p->session_id);
  } else
#endif
  {
      key = RLC_COLL_KEY_VALUE(enb_mod_idP, ue_mod_idP, enb_flagP, rb_idP, srb_flagP);
  }


    AssertFatal (rb_idP < NB_RB_MAX, "RB id is too high (%u/%d)!\n", rb_idP, NB_RB_MAX);

    h_rc = hashtable_remove(rlc_coll_p, key);
    if (h_rc == HASH_TABLE_OK) {
        LOG_D(RLC, "[Frame %05u][%s][RLC_RRC][INST %u/%u][%s %u] RELEASED %s\n",
            frameP,
            (enb_flagP) ? "eNB" : "UE",
            enb_mod_idP,
            ue_mod_idP,
            (srb_flagP) ? "SRB" : "DRB",
            rb_idP,
            (srb_flagP) ? "SRB" : "DRB");
    } else if (h_rc == HASH_TABLE_KEY_NOT_EXISTS) {
        LOG_W(RLC, "[Frame %05u][%s][RLC_RRC][INST %u/%u][%s %u] RELEASE : RLC NOT FOUND %s\n",
            frameP,
            (enb_flagP) ? "eNB" : "UE",
            enb_mod_idP,
            ue_mod_idP,
            (srb_flagP) ? "SRB" : "DRB",
            rb_idP,
            (srb_flagP) ? "SRB" : "DRB");
    } else {
        LOG_E(RLC, "[Frame %05u][%s][RLC_RRC][INST %u/%u][%s %u] RELEASE : INTERNAL ERROR %s\n",
            frameP,
            (enb_flagP) ? "eNB" : "UE",
            enb_mod_idP,
            ue_mod_idP,
            (srb_flagP) ? "SRB" : "DRB",
            rb_idP,
            (srb_flagP) ? "SRB" : "DRB");    }

    return RLC_OP_STATUS_OK;
}
//-----------------------------------------------------------------------------
rlc_union_t* rrc_rlc_add_rlc   (
    const module_id_t       enb_mod_idP,
    const module_id_t       ue_mod_idP,
    const frame_t           frameP,
    const eNB_flag_t        enb_flagP,
    const srb_flag_t        srb_flagP,
    const MBMS_flag_t       MBMS_flagP,
    const rb_id_t           rb_idP,
    const logical_chan_id_t chan_idP,
    const rlc_mode_t        rlc_modeP) {
//-----------------------------------------------------------------------------
  hash_key_t             key         = HASHTABLE_QUESTIONABLE_KEY_VALUE;
  hashtable_rc_t         h_rc;
  rlc_union_t           *rlc_union_p = NULL;
#ifdef Rel10
    rlc_mbms_id_t         *mbms_id_p  = NULL;
    logical_chan_id_t      lcid            = 0;
#endif

#ifdef OAI_EMU
    if (enb_flagP) {
        AssertFatal ((enb_mod_idP >= oai_emulation.info.first_enb_local) && (oai_emulation.info.nb_enb_local > 0),
            "eNB module id is too low (%u/%d)!\n",
            enb_mod_idP,
            oai_emulation.info.first_enb_local);
        AssertFatal ((enb_mod_idP < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local)) && (oai_emulation.info.nb_enb_local > 0),
            "eNB module id is too high (%u/%d)!\n",
            enb_mod_idP,
            oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local);
        AssertFatal (ue_mod_idP  < NB_UE_INST,
            "UE module id is too high (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
    } else {
        AssertFatal (ue_mod_idP  < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local),
            "UE module id is too high (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
        AssertFatal (ue_mod_idP  >= oai_emulation.info.first_ue_local,
            "UE module id is too low (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local);
    }
#endif
    if (MBMS_flagP == FALSE) {
        AssertFatal (rb_idP < NB_RB_MAX, "RB id is too high (%u/%d)!\n", rb_idP, NB_RB_MAX);
        AssertFatal (chan_idP < RLC_MAX_LC, "LC id is too high (%u/%d)!\n", chan_idP, RLC_MAX_LC);
    }

#ifdef Rel10
  if (MBMS_flagP == TRUE) {
      if (enb_flagP) {
          lcid = rlc_mbms_enb_get_lcid_by_rb_id(enb_mod_idP,rb_idP);
          LOG_I(RLC,
                  "[Frame %05u] lcid %d = rlc_mbms_enb_get_lcid_by_rb_id(enb_mod_idP %u, rb_idP %u)\n",
                  frameP,lcid, enb_mod_idP, rb_idP);

          mbms_id_p = &rlc_mbms_lcid2service_session_id_eNB[enb_mod_idP][lcid];

          //LG 2014-04-15rlc_mbms_lcid2service_session_id_eNB[enb_mod_idP][lcid].service_id = 0;
          //LG 2014-04-15rlc_mbms_lcid2service_session_id_eNB[enb_mod_idP][lcid].session_id = 0;
          //LG 2014-04-15rlc_mbms_rbid2lcid_eNB[enb_mod_idP][rb_idP] = RLC_LC_UNALLOCATED;
      } else {
          lcid = rlc_mbms_ue_get_lcid_by_rb_id(ue_mod_idP,rb_idP);
          mbms_id_p = &rlc_mbms_lcid2service_session_id_ue[ue_mod_idP][lcid];

          //LG 2014-04-15rlc_mbms_lcid2service_session_id_eNB[ue_mod_idP][lcid].service_id = 0;
          //LG 2014-04-15rlc_mbms_lcid2service_session_id_eNB[ue_mod_idP][lcid].session_id = 0;
          //LG 2014-04-15rlc_mbms_rbid2lcid_ue[ue_mod_idP][rb_idP] = RLC_LC_UNALLOCATED;
      }
      key = RLC_COLL_KEY_MBMS_VALUE(enb_mod_idP, ue_mod_idP, enb_flagP, mbms_id_p->service_id, mbms_id_p->session_id);
  } else
#endif
  {
      key = RLC_COLL_KEY_VALUE(enb_mod_idP, ue_mod_idP, enb_flagP, rb_idP, srb_flagP);
  }

  h_rc = hashtable_get(rlc_coll_p, key, (void**)&rlc_union_p);
  if (h_rc == HASH_TABLE_OK) {
      LOG_W(RLC, "[Frame %05u][%s][RLC_RRC][INST %u/%u][%s %u] rrc_rlc_add_rlc , already exist %s\n",
          frameP,
          (enb_flagP) ? "eNB" : "UE",
          enb_mod_idP,
          ue_mod_idP,
          (srb_flagP) ? "SRB" : "DRB",
          rb_idP,
          (srb_flagP) ? "SRB" : "DRB");
      AssertFatal(rlc_union_p->mode == rlc_modeP, "Error rrc_rlc_add_rlc , already exist but RLC mode differ");
      return rlc_union_p;
  } else if (h_rc == HASH_TABLE_KEY_NOT_EXISTS) {
      rlc_union_p = calloc(1, sizeof(rlc_union_t));
      h_rc = hashtable_insert(rlc_coll_p, key, rlc_union_p);
      if (h_rc == HASH_TABLE_OK) {
#ifdef Rel10
          if (MBMS_flagP == TRUE) {
              LOG_I(RLC, "[Frame %05u][%s][RLC_RRC][INST %u/%u] RLC service id %u session id %u rrc_rlc_add_rlc\n",
                  frameP,
                  (enb_flagP) ? "eNB" : "UE",
                  enb_mod_idP,
                  ue_mod_idP,
                  mbms_id_p->service_id,
                  mbms_id_p->session_id);
          } else
#endif
          {
              LOG_I(RLC, "[Frame %05u][%s][RLC_RRC][INST %u/%u][%s %u] rrc_rlc_add_rlc  %s\n",
                  frameP,
                  (enb_flagP) ? "eNB" : "UE",
                  enb_mod_idP,
                  ue_mod_idP,
                  (srb_flagP) ? "SRB" : "DRB",
                  rb_idP,
                  (srb_flagP) ? "SRB" : "DRB");
          }
          rlc_union_p->mode = rlc_modeP;
          return rlc_union_p;
      } else {
          LOG_E(RLC, "[Frame %05u][%s][RLC_RRC][INST %u/%u][%s %u] rrc_rlc_add_rlc  FAILED %s\n",
              frameP,
              (enb_flagP) ? "eNB" : "UE",
              enb_mod_idP,
              ue_mod_idP,
              (srb_flagP) ? "SRB" : "DRB",
              rb_idP,
              (srb_flagP) ? "SRB" : "DRB");
          free(rlc_union_p);
          rlc_union_p = NULL;
          return NULL;
      }
  } else {
      LOG_E(RLC, "[Frame %05u][%s][RLC_RRC][INST %u/%u][%s %u] rrc_rlc_add_rlc , INTERNAL ERROR %s\n",
          frameP,
          (enb_flagP) ? "eNB" : "UE",
          enb_mod_idP,
          ue_mod_idP,
          (srb_flagP) ? "SRB" : "DRB",
          rb_idP,
          (srb_flagP) ? "SRB" : "DRB");
  }
  return NULL;
}
//-----------------------------------------------------------------------------
rlc_op_status_t rrc_rlc_config_req   (
    const module_id_t     enb_mod_idP,
    const module_id_t     ue_mod_idP,
    const frame_t         frameP,
    const eNB_flag_t      enb_flagP,
    const srb_flag_t      srb_flagP,
    const MBMS_flag_t     mbms_flagP,
    const config_action_t actionP,
    const rb_id_t         rb_idP,
    const rlc_info_t      rlc_infoP) {
//-----------------------------------------------------------------------------
    rlc_op_status_t status;

    LOG_D(RLC, "[FRAME %05u][%s][RLC][MOD %u/%u] CONFIG_REQ for Rab %u\n",
            frameP,
            (enb_flagP) ? "eNB" : "UE",
            enb_mod_idP,
            ue_mod_idP,
            rb_idP);

#ifdef OAI_EMU
    if (enb_flagP) {
        AssertFatal ((enb_mod_idP >= oai_emulation.info.first_enb_local) && (oai_emulation.info.nb_enb_local > 0),
            "eNB module id is too low (%u/%d)!\n",
            enb_mod_idP,
            oai_emulation.info.first_enb_local);
        AssertFatal ((enb_mod_idP < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local)) && (oai_emulation.info.nb_enb_local > 0),
            "eNB module id is too high (%u/%d)!\n",
            enb_mod_idP,
            oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local);
        AssertFatal (ue_mod_idP  < NB_UE_INST,
            "UE module id is too high (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
    } else {
        AssertFatal (ue_mod_idP  < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local),
            "UE module id is too high (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
        AssertFatal (ue_mod_idP  >= oai_emulation.info.first_ue_local,
            "UE module id is too low (%u/%d)!\n",
            ue_mod_idP,
            oai_emulation.info.first_ue_local);
    }
#endif
    AssertFatal (rb_idP < NB_RB_MAX, "RB id is too high (%u/%d)!\n", rb_idP, NB_RB_MAX);
    switch (actionP) {

        case CONFIG_ACTION_ADD:
            if (rrc_rlc_add_rlc(enb_mod_idP, ue_mod_idP, frameP, enb_flagP, srb_flagP, MBMS_FLAG_NO, rb_idP, rb_idP, rlc_infoP.rlc_mode) != NULL) {
              return RLC_OP_STATUS_INTERNAL_ERROR;
            }
            // no break, fall to next case
        case CONFIG_ACTION_MODIFY:
            switch (rlc_infoP.rlc_mode) {
                case RLC_MODE_AM:
                    LOG_I(RLC, "[Frame %05u][UE][RLC_RRC][INST %u/%u][RB %u] MODIFY RB AM\n",
                            frameP,
                            enb_mod_idP,
                            ue_mod_idP,
                            rb_idP);

                    config_req_rlc_am(
                        enb_mod_idP,
                        ue_mod_idP,
                        frameP,
                        enb_flagP,
                        srb_flagP,
                        &rlc_infoP.rlc.rlc_am_info,
                        rb_idP);
                    break;
                case RLC_MODE_UM:
                    LOG_I(RLC, "[Frame %05u][UE][RLC_RRC][INST %u/%u][RB %u] MODIFY RB UM\n",
                            frameP,
                            enb_mod_idP,
                            ue_mod_idP,
                            rb_idP);
                    config_req_rlc_um(
                        enb_mod_idP,
                        ue_mod_idP,
                        frameP,
                        enb_flagP,
                        srb_flagP,
                        &rlc_infoP.rlc.rlc_um_info,
                        rb_idP);
                    break;
                case RLC_MODE_TM:
                    LOG_I(RLC, "[Frame %05u][UE][RLC_RRC][INST %u/%u][RB %u] MODIFY RB TM\n",
                            frameP,
                            enb_mod_idP,
                            ue_mod_idP,
                            rb_idP);
                    config_req_rlc_tm(
                        enb_mod_idP,
                        ue_mod_idP,
                        frameP,
                        enb_flagP,
                        srb_flagP,
                        &rlc_infoP.rlc.rlc_tm_info,
                        rb_idP);
                    break;
                default:
                return RLC_OP_STATUS_BAD_PARAMETER;
            }
            break;

        case CONFIG_ACTION_REMOVE:
            return rrc_rlc_remove_rlc(enb_mod_idP, ue_mod_idP, frameP, enb_flagP, srb_flagP, mbms_flagP, rb_idP);
            break;
        default:
            return RLC_OP_STATUS_BAD_PARAMETER;
    }

    return RLC_OP_STATUS_OK;
}
//-----------------------------------------------------------------------------
rlc_op_status_t rrc_rlc_data_req     (
    const module_id_t enb_mod_idP,
    const module_id_t ue_mod_idP,
    const frame_t     frameP,
    const eNB_flag_t  enb_flagP,
    const MBMS_flag_t MBMS_flagP,
    const rb_id_t     rb_idP,
    const mui_t       muiP,
    const confirm_t   confirmP,
    const sdu_size_t  sdu_sizeP,
    char* sduP) {
//-----------------------------------------------------------------------------
  mem_block_t*   sdu;

  sdu = get_free_mem_block(sdu_sizeP);
  if (sdu != NULL) {
    memcpy (sdu->data, sduP, sdu_sizeP);
    return rlc_data_req(enb_mod_idP, ue_mod_idP, frameP, enb_flagP, SRB_FLAG_YES, MBMS_flagP, rb_idP, muiP, confirmP, sdu_sizeP, sdu);
  } else {
    return RLC_OP_STATUS_INTERNAL_ERROR;
  }
}

//-----------------------------------------------------------------------------
void rrc_rlc_register_rrc (rrc_data_ind_cb_t rrc_data_indP, rrc_data_conf_cb_t rrc_data_confP) {
//-----------------------------------------------------------------------------
   rlc_rrc_data_ind  = rrc_data_indP;
   rlc_rrc_data_conf = rrc_data_confP;
}

