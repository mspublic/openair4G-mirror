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
//-----------------------------------------------------------------------------
void
config_req_rlc_um (rlc_um_entity_t *rlcP, module_id_t module_idP, rlc_um_info_t * config_umP, u8_t rb_idP, rb_type_t rb_typeP)
{
//-----------------------------------------------------------------------------
  mem_block_t *mb;

  msg ("[RLC_UM][MOD %d][RB %d] config_req_rlc_um\n", module_idP, rb_idP);
  mb = get_free_mem_block (sizeof (struct crlc_primitive));
  ((struct crlc_primitive *) mb->data)->type = CRLC_CONFIG_REQ;
  ((struct crlc_primitive *) mb->data)->primitive.c_config_req.parameters.um_parameters.e_r  = RLC_E_R_ESTABLISHMENT;
  ((struct crlc_primitive *) mb->data)->primitive.c_config_req.parameters.um_parameters.stop = 0;
  ((struct crlc_primitive *) mb->data)->primitive.c_config_req.parameters.um_parameters.cont = 1;

  ((struct crlc_primitive *) mb->data)->primitive.c_config_req.parameters.um_parameters.frame_tick_milliseconds = &mac_xface->frame;
  ((struct crlc_primitive *) mb->data)->primitive.c_config_req.parameters.um_parameters.size_input_sdus_buffer = 256;
  ((struct crlc_primitive *) mb->data)->primitive.c_config_req.parameters.um_parameters.rb_id = rb_idP;
  send_rlc_um_control_primitive (rlcP, module_idP, mb);
  if (rb_typeP != SIGNALLING_RADIO_BEARER) {
    rlcP->data_plane = 1;
  } else {
    rlcP->data_plane = 0;
  }
}
//-----------------------------------------------------------------------------
void
send_rlc_um_control_primitive (rlc_um_entity_t *rlcP, module_id_t module_idP, mem_block_t *cprimitiveP)
{
//-----------------------------------------------------------------------------

  switch (((struct crlc_primitive *) cprimitiveP->data)->type) {

      case CRLC_CONFIG_REQ:


        switch (((struct crlc_primitive *) cprimitiveP->data)->primitive.c_config_req.parameters.um_parameters.e_r) {
            case RLC_E_R_ESTABLISHMENT:
              rlcP->module_id = module_idP;
              if (rlc_um_fsm_notify_event (rlcP, RLC_UM_RECEIVE_CRLC_CONFIG_REQ_ENTER_DATA_TRANSFER_READY_STATE_EVENT)) {
                rlc_um_set_configured_parameters (rlcP, cprimitiveP);   // the order of the calling of procedures...
                rlc_um_reset_state_variables (rlcP);    // ...must not ...
              }
              break;

            case RLC_E_R_MODIFICATION:
              msg ("[RLC_UM][ERROR] send_rlc_um_control_primitive(CRLC_CONFIG_REQ) RLC_AM_E_R_MODIFICATION not handled\n");
              break;

            case RLC_E_R_RELEASE:
              if (rlc_um_fsm_notify_event (rlcP, RLC_UM_RECEIVE_CRLC_CONFIG_REQ_ENTER_NULL_STATE_EVENT)) {
                rlc_um_free_all_resources (rlcP);
              }
              break;

            default:
              msg ("[RLC_UM][ERROR] send_rlc_um_control_primitive(CRLC_CONFIG_REQ) unknown parameter E_R\n");
        }
        break;

      case CRLC_RESUME_REQ:
        msg ("[RLC_UM][ERROR] send_rlc_um_control_primitive(CRLC_RESUME_REQ) cprimitive not handled\n");
        break;

      default:
        msg ("[RLC_UM][RB %d][ERROR] send_rlc_um_control_primitive(UNKNOWN CPRIMITIVE)\n", rlcP->rb_id);
  }
  free_mem_block (cprimitiveP);
}
//-----------------------------------------------------------------------------
void
init_rlc_um (rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------

  memset (rlcP, 0, sizeof (rlc_um_entity_t));
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
rlc_um_free_all_resources (rlc_um_entity_t *rlcP)
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
void
rlc_um_set_configured_parameters (rlc_um_entity_t *rlcP, mem_block_t *cprimitiveP)
{
//-----------------------------------------------------------------------------
  rlcP->sn_length          = 10;
  rlcP->sn_modulo          = UM_SN_10_BITS_MODULO;
  rlcP->um_window_size     = UM_WINDOW_SIZE_SN_10_BITS;
  rlcP->header_min_length_in_bytes = 2;
  
  rlcP->last_reassemblied_missing_sn = rlcP->sn_modulo - 1;
  rlcP->reassembly_missing_sn_detected = 0;
  // timers
  rlcP->timer_reordering         = 0;
  rlcP->timer_reordering_init    = 500;
  rlcP->timer_reordering_running = 0;
  // SPARE : not 3GPP
  rlcP->frame_tick_milliseconds = ((struct crlc_primitive *)
                                   cprimitiveP->data)->primitive.c_config_req.parameters.um_parameters.frame_tick_milliseconds;
  rlcP->size_input_sdus_buffer = ((struct crlc_primitive *)
                                  cprimitiveP->data)->primitive.c_config_req.parameters.um_parameters.size_input_sdus_buffer;
  rlcP->rb_id = ((struct crlc_primitive *) cprimitiveP->data)->primitive.c_config_req.parameters.um_parameters.rb_id;

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
