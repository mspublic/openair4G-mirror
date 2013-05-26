/*
                                rlc_rrc.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMlAIL   : Lionel.Gauthier@eurecom.fr
*/

#define RLC_RRC_C
#include "rlc.h"
#include "rlc_am.h"
#include "rlc_um.h"
#include "rlc_tm.h"
#include "UTIL/LOG/log.h"

#include "LAYER2/MAC/extern.h"
//-----------------------------------------------------------------------------
rlc_op_status_t
rb_release_rlc_tm (struct rlc_tm_entity *rlcP, module_id_t module_idP)
{
//-----------------------------------------------------------------------------
  rlc_tm_cleanup(rlcP);
  return RLC_OP_STATUS_OK;
}
//-----------------------------------------------------------------------------
rlc_op_status_t
rb_release_rlc_um (struct rlc_um_entity *rlcP, module_id_t module_idP)
{
//-----------------------------------------------------------------------------
  rlc_um_cleanup(rlcP);
  return RLC_OP_STATUS_OK;
}
//-----------------------------------------------------------------------------
rlc_op_status_t
rb_release_rlc_am (struct rlc_am_entity *rlcP, u32_t frame, module_id_t module_idP)
{
//-----------------------------------------------------------------------------
  rlc_am_cleanup(rlcP,frame);
  return RLC_OP_STATUS_OK;
}
//-----------------------------------------------------------------------------
rlc_op_status_t rrc_rlc_remove_rlc   (module_id_t module_idP, rb_id_t rb_idP, u32_t frame) {
//-----------------------------------------------------------------------------
  int rlc_mode = rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type;
  //
  rlc_op_status_t status;
  switch (rlc_mode) {
  case RLC_AM:
      LOG_D(RLC, "[RLC_RRC][MOD_id %d] RELEASE RAB AM %d \n",module_idP,rb_idP);
      status = rb_release_rlc_am(&rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], frame, module_idP);
    rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index].allocation = 0;
        break;
  case RLC_TM:
     LOG_D(RLC, "[RLC_RRC][MOD_id %d] RELEASE RAB TM %d \n",module_idP,rb_idP);
    status = rb_release_rlc_tm(&rlc[module_idP].m_rlc_tm_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], module_idP);
    rlc[module_idP].m_rlc_tm_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index].allocation = 0;
    break;
  case RLC_UM:
      LOG_D(RLC, "[RLC_RRC][MOD_id %d] RELEASE RAB UM %d \n",module_idP,rb_idP);
    status = rb_release_rlc_um(&rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], module_idP);
    rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index].allocation = 0;
    break;
  default:
      LOG_E(RLC, "[RLC_RRC][MOD_id %d] RELEASE RAB %d RLC_MODE %d INDEX %d\n",module_idP,rb_idP,rlc_mode, rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index);
    mac_xface->macphy_exit("[RLC]REMOVE RB ERROR: UNKNOWN RLC MODE");
    return RLC_OP_STATUS_BAD_PARAMETER;
    break;
  }
  rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type  = RLC_NONE;
    rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index = -1;
    return status;
}
//-----------------------------------------------------------------------------
rlc_op_status_t rrc_rlc_add_rlc   (module_id_t module_idP, u32_t frameP, rb_id_t rb_idP, rlc_mode_t rlc_modeP) {
//-----------------------------------------------------------------------------
    unsigned int index;
    unsigned int index_max;
    unsigned int allocation;
    
    switch (rlc_modeP) {
        case RLC_AM:
            index_max = RLC_MAX_NUM_INSTANCES_RLC_AM;
            break;
        case RLC_TM:
            index_max = RLC_MAX_NUM_INSTANCES_RLC_TM;
            break;
        case RLC_UM:
            index_max = RLC_MAX_NUM_INSTANCES_RLC_UM;
            break;
        default:
	  LOG_E(RLC,"Got bad RLC type %d\n",rlc_modeP);
	  return RLC_OP_STATUS_BAD_PARAMETER;
    }

    for (index = 0; index < index_max; index++) {
        switch (rlc_modeP) {
            case RLC_AM:

                allocation = rlc[module_idP].m_rlc_am_array[index].allocation;
                if (!(allocation)) {
                    rlc[module_idP].m_rlc_am_array[index].allocation = 1;
                    rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index  = index;
                    rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type   = rlc_modeP;
                    LOG_D(RLC, "[RLC_RRC][MOD ID %d][RB %d] ADD RB AM INDEX IS %d\n", module_idP, rb_idP, index);
                    LOG_D(RLC,  "[MSC_NEW][FRAME %05d][RLC_AM][MOD %02d][RB %02d]\n", frameP, module_idP,rb_idP);
                    return RLC_OP_STATUS_OK;
                } else {
		  LOG_D(RLC,"[RLC_RRC][MOD ID %d][RB %d] ADD RB AM INDEX %d IS ALREADY ALLOCATED\n", module_idP, rb_idP, index);
                }
            break;
            case RLC_TM:
                allocation = rlc[module_idP].m_rlc_tm_array[index].allocation;
                if (!(allocation)) {
                    rlc[module_idP].m_rlc_tm_array[index].allocation = 1;
                    rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index  = index;
                    rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type   = rlc_modeP;
                    LOG_D(RLC, "[RLC_RRC][MOD ID %d][RB %d] ADD RB TM INDEX IS %d\n", module_idP, rb_idP, index);
                    LOG_D(RLC, "[MSC_NEW][FRAME %05d][RLC_TM][MOD %02d][RB %02d]\n", frameP, module_idP, rb_idP);
                    return RLC_OP_STATUS_OK;
                } else {
                    msg ("[RLC_RRC][MOD ID %d][RB %d] ADD RB TM INDEX %d IS ALLOCATED\n", module_idP, rb_idP, index);
                }
            break;
            case RLC_UM:
                allocation = rlc[module_idP].m_rlc_um_array[index].allocation;
                if (!(allocation)) {
                    rlc[module_idP].m_rlc_um_array[index].allocation = 1;
                    rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index  = index;
                    rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type   = rlc_modeP;
                    LOG_D(RLC, "[RLC_RRC][MOD ID %d][RB %d] ADD RB UM INDEX IS %d  RLC_MODE %d\n", module_idP, rb_idP, index, rlc_modeP);
                    LOG_D(RLC, "[MSC_NEW][FRAME %05d][RLC_UM][MOD %02d][RB %02d]\n", frameP, module_idP, rb_idP);
                    return RLC_OP_STATUS_OK;
                } else {
                    msg ("[RLC_RRC][MOD ID %d][RB %d] ADD RB UM INDEX %d IS ALREADY ALLOCATED\n", module_idP, rb_idP, index);
                }
            break;
            default:
                LOG_E(RLC, "[RLC_RRC][MOD ID %d][RB %d] ADD RB WITH INDEX %d ERROR\n", module_idP, rb_idP, index);
              mac_xface->macphy_exit("");
              return RLC_OP_STATUS_BAD_PARAMETER;
        }
    }
    LOG_C(RLC, "[RLC_RRC][ADD_RB] Out of Ressources\n");
    mac_xface->macphy_exit("");

    return RLC_OP_STATUS_OUT_OF_RESSOURCES;
}
//-----------------------------------------------------------------------------
rlc_op_status_t rrc_rlc_config_req   (module_id_t module_idP, u32_t frame, u8_t eNB_flagP, config_action_t actionP, rb_id_t rb_idP, rb_type_t rb_typeP, rlc_info_t rlc_infoP) {
//-----------------------------------------------------------------------------
  msg("[RLC][MOD_id %d] CONFIG_REQ for Rab %d\n",module_idP,rb_idP);
#warning TO DO rrc_rlc_config_req
    rlc_op_status_t status;

    switch (actionP) {

        case ACTION_ADD:
            if (rb_typeP != SIGNALLING_RADIO_BEARER) {
                LOG_D(PDCP, "[MSC_NEW][FRAME %05d][PDCP][MOD %02d][RB %02d]\n", frame, module_idP, rb_idP);
                LOG_D(PDCP, "[MSC_NEW][FRAME %05d][IP][MOD %02d][]\n", frame, module_idP);
            }
            if ((status = rrc_rlc_add_rlc(module_idP, frame, rb_idP, rlc_infoP.rlc_mode)) != RLC_OP_STATUS_OK) {
              return status;
            }
        case ACTION_MODIFY:
            switch (rlc_infoP.rlc_mode) {
                case RLC_AM:
                    LOG_D(RLC, "[RLC_RRC][MOD ID %d][RB %d] MODIFY RB AM\n", module_idP, rb_idP);
                    config_req_rlc_am(&rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index],
				      frame,
				      eNB_flagP,
				      module_idP,
				      &rlc_infoP.rlc.rlc_am_info,
				      rb_idP, rb_typeP);
                    break;
                case RLC_UM:
                    LOG_D(RLC, "[RLC_RRC][MOD ID %d][RB %d] MODIFY RB UM\n", module_idP, rb_idP);
                    config_req_rlc_um(&rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index],
				      frame,
				      eNB_flagP,
				      module_idP,
				      &rlc_infoP.rlc.rlc_um_info,
				      rb_idP, rb_typeP);
                    break;
                case RLC_TM:
                    LOG_D(RLC, "[RLC_RRC][MOD ID %d][RB %d] MODIFY RB TM\n", module_idP, rb_idP);
                    config_req_rlc_tm(&rlc[module_idP].m_rlc_tm_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index],
				      frame,
                      eNB_flagP,
				      module_idP,
				      &rlc_infoP.rlc.rlc_tm_info,
				      rb_idP, rb_typeP);
                    break;
                default:
                return RLC_OP_STATUS_BAD_PARAMETER;
            }
            break;

        case ACTION_REMOVE:
	  return rrc_rlc_remove_rlc(module_idP, rb_idP,frame);
            break;
        default:
            return RLC_OP_STATUS_BAD_PARAMETER;
    }

    return RLC_OP_STATUS_OK;
}
//-----------------------------------------------------------------------------
rlc_op_status_t rrc_rlc_data_req     (module_id_t module_idP, u32_t frame, u8_t eNB_flagP, rb_id_t rb_idP, mui_t muiP, confirm_t confirmP, sdu_size_t sdu_sizeP, char* sduP) {
//-----------------------------------------------------------------------------
  mem_block_t*   sdu;

  sdu = get_free_mem_block(sdu_sizeP);
  if (sdu != NULL) {
    //    msg("[RRC_RLC] MEM_ALLOC %p\n",sdu);
    memcpy (sdu->data, sduP, sdu_sizeP);
    return rlc_data_req(module_idP, frame, eNB_flagP, rb_idP, muiP, confirmP, sdu_sizeP, sdu);
  } else {
    return RLC_OP_STATUS_INTERNAL_ERROR;
  }
}

//-----------------------------------------------------------------------------
void   rrc_rlc_register_rrc ( void            (*rrc_data_indP)  (module_id_t module_idP, u32_t frame, u8_t eNB_id, rb_id_t rb_idP, sdu_size_t sdu_sizeP, char* sduP),
							  void            (*rrc_data_confP) (module_id_t module_idP, rb_id_t rb_idP, mui_t muiP, rlc_tx_status_t statusP) ) {
//-----------------------------------------------------------------------------
   rlc_rrc_data_ind  = rrc_data_indP;
   rlc_rrc_data_conf = rrc_data_confP;
}

