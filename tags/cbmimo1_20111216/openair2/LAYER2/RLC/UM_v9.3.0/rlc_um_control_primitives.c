#define RLC_UM_MODULE
#define RLC_UM_CONTROL_PRIMITIVES_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc_um.h"
#include "rlc_primitives.h"
#include "list.h"
#include "rrm_config_structs.h"
#include "LAYER2/MAC/extern.h"

#include "rlc_um_control_primitives.h"
//-----------------------------------------------------------------------------
void config_req_rlc_um (rlc_um_entity_t *rlcP, module_id_t module_idP, rlc_um_info_t * config_umP, u8_t rb_idP, rb_type_t rb_typeP)
{
    //-----------------------------------------------------------------------------
    rlc_um_init(rlcP);
    if (rlc_um_fsm_notify_event (rlcP, RLC_UM_RECEIVE_CRLC_CONFIG_REQ_ENTER_DATA_TRANSFER_READY_STATE_EVENT)) {
        rlc_um_set_debug_infos(rlcP, module_idP, rb_idP, rb_typeP);
        rlc_um_configure(rlcP,
                     config_umP->timer_reordering,
                     config_umP->sn_field_length,
                     config_umP->is_mXch);
    }
}
//-----------------------------------------------------------------------------
void
rlc_um_init (rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------

  int saved_allocation = rlcP->allocation;
  memset (rlcP, 0, sizeof (rlc_um_entity_t));
  rlcP->allocation = saved_allocation;
  // TX SIDE
  list_init (&rlcP->pdus_to_mac_layer, NULL);

  rlcP->protocol_state = RLC_NULL_STATE;
  rlcP->nb_sdu           = 0;
  rlcP->next_sdu_index   = 0;
  rlcP->current_sdu_index = 0;

  rlcP->vt_us = 0;

  // RX SIDE
  list_init (&rlcP->pdus_from_mac_layer, NULL);
  rlcP->vr_ur = 0;
  rlcP->vr_ux = 0;
  rlcP->vr_uh = 0;
  rlcP->output_sdu_size_to_write = 0;
  rlcP->output_sdu_in_construction = NULL;

  rlcP->sn_length          = 10;
  rlcP->header_min_length_in_bytes = 2;


  rlcP->tx_pdcp_sdu                 = 0;
  rlcP->tx_pdcp_sdu_discarded       = 0;
  rlcP->tx_data_pdu                 = 0;
  rlcP->rx_sdu                      = 0;
  rlcP->rx_error_pdu                = 0;
  rlcP->rx_data_pdu                 = 0;
  rlcP->rx_data_pdu_out_of_window   = 0;


  // SPARE : not 3GPP
  rlcP->size_input_sdus_buffer =128;

  if ((rlcP->input_sdus_alloc == NULL) && (rlcP->size_input_sdus_buffer > 0)) {
      rlcP->input_sdus_alloc = get_free_mem_block (rlcP->size_input_sdus_buffer * sizeof (void *));
      rlcP->input_sdus = (mem_block_t **) (rlcP->input_sdus_alloc->data);
      memset (rlcP->input_sdus, 0, rlcP->size_input_sdus_buffer * sizeof (void *));
  }
  if (rlcP->dar_buffer_alloc == NULL) {
      rlcP->dar_buffer_alloc = get_free_mem_block (1024 * sizeof (void *));
      rlcP->dar_buffer = (mem_block_t **) (rlcP->dar_buffer_alloc->data);
      memset (rlcP->dar_buffer, 0, 1024 * sizeof (void *));
  }

  rlcP->first_pdu = 1;
}
//-----------------------------------------------------------------------------
void
rlc_um_reset_state_variables (rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
  rlcP->buffer_occupancy = 0;
  rlcP->nb_sdu = 0;
  rlcP->next_sdu_index = 0;
  rlcP->current_sdu_index = 0;

  rlcP->last_reassemblied_sn = 0;
  //rlcP->reassembly_missing_pdu_detected = 0;

  // TX SIDE
  rlcP->vt_us = 0;
  // RX SIDE
  rlcP->vr_ur = 0;
  rlcP->vr_ux = 0;
  rlcP->vr_uh = 0;
}
//-----------------------------------------------------------------------------
void
rlc_um_cleanup (rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
  int             index;
  // TX SIDE
  list_free (&rlcP->pdus_to_mac_layer);

  if (rlcP->input_sdus_alloc) {
    for (index = 0; index < rlcP->size_input_sdus_buffer; index++) {
      if (rlcP->input_sdus[index]) {
        free_mem_block (rlcP->input_sdus[index]);
      }
    }
    free_mem_block (rlcP->input_sdus_alloc);
    rlcP->input_sdus_alloc = NULL;
  }
  // RX SIDE
  list_free (&rlcP->pdus_from_mac_layer);
  if ((rlcP->output_sdu_in_construction)) {
    free_mem_block (rlcP->output_sdu_in_construction);
  }
  if (rlcP->dar_buffer_alloc) {
    for (index = 0; index < 1024; index++) {
      if (rlcP->dar_buffer[index]) {
        free_mem_block (rlcP->dar_buffer[index]);
      }
    }
    free_mem_block (rlcP->dar_buffer_alloc);
    rlcP->dar_buffer_alloc = NULL;
  }
}

//-----------------------------------------------------------------------------
void rlc_um_configure(rlc_um_entity_t *rlcP,
                      u32_t timer_reorderingP,
                      u32_t sn_field_lengthP,
                      u32_t is_mXchP)
//-----------------------------------------------------------------------------
{
    if (sn_field_lengthP == 10) {
        rlcP->sn_length          = 10;
        rlcP->sn_modulo          = RLC_UM_SN_10_BITS_MODULO;
        rlcP->um_window_size     = RLC_UM_WINDOW_SIZE_SN_10_BITS;
        rlcP->header_min_length_in_bytes = 2;
    } else if (sn_field_lengthP == 5) {
        msg ("[FRAME %05d][RLC_UM][MOD %02d][RB %02d][CONFIGURE] SN LENGTH 5 BITS NOT IMPLEMENTED YET, RLC NOT CONFIGURED\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
        /*rlcP->sn_length          = 5;
        rlcP->sn_modulo          = RLC_UM_SN_5_BITS_MODULO;
        rlcP->um_window_size     = RLC_UM_WINDOW_SIZE_SN_5_BITS;
        rlcP->header_min_length_in_bytes = 1;*/
        return;
    } else {
        msg ("[FRAME %05d][RLC_UM][MOD %02d][RB %02d][CONFIGURE] INVALID SN LENGTH %d BITS NOT IMPLEMENTED YET, RLC NOT CONFIGURED\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, sn_field_lengthP);
        return;
    }

    if (is_mXchP > 0) {
        rlcP->um_window_size = 0;
    }

    rlcP->last_reassemblied_missing_sn = rlcP->sn_modulo - 1;
    rlcP->reassembly_missing_sn_detected = 0;
    // timers
    rlcP->timer_reordering         = 0;
    rlcP->timer_reordering_init    = timer_reorderingP;
    rlcP->timer_reordering_running = 0;

    rlcP->first_pdu = 1;

    rlc_um_reset_state_variables (rlcP);
}
//-----------------------------------------------------------------------------
void rlc_um_set_debug_infos(rlc_um_entity_t *rlcP, module_id_t module_idP, rb_id_t rb_idP, rb_type_t rb_typeP)
//-----------------------------------------------------------------------------
{
    msg ("[FRAME %05d][RLC_UM][MOD %02d][RB %02d][SET DEBUG INFOS] module_id %d rb_id %d rb_type %d\n", mac_xface->frame, module_idP, rb_idP, module_idP, rb_idP, rb_typeP);

    rlcP->module_id = module_idP;
    rlcP->rb_id     = rb_idP;
    if (rb_typeP != SIGNALLING_RADIO_BEARER) {
        rlcP->is_data_plane = 1;
    } else {
        rlcP->is_data_plane = 0;
    }
}
