/***************************************************************************
                          rlc_tm_reassembly.c  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr


 ***************************************************************************/
#define RLC_TM_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_tm_structs.h"
#include "rlc_primitives.h"
#include "rlc_tm_constants.h"
#include "list.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
rlc_tm_send_sdu_no_segment (struct rlc_tm_entity *rlcP, u8_t error_indicationP, u8 * srcP, u16_t length_in_bitsP)
{
//-----------------------------------------------------------------------------
  int             length_in_bytes;
#ifdef DEBUG_RLC_TM_DISPLAY_ASCII_DATA
  int             index;
#endif
#ifdef DEBUG_RLC_TM_REASSEMBLY
  msg ("[RLC_TM %p][SEND_SDU] %d bits\n", rlcP, length_in_bitsP);
#endif
  length_in_bytes = (length_in_bitsP + 7) >> 3;
  if (rlcP->output_sdu_in_construction == NULL) {
    rlcP->output_sdu_in_construction = get_free_mem_block (length_in_bytes);
  }
  if ((rlcP->output_sdu_in_construction)) {
#ifdef DEBUG_RLC_TM_DISPLAY_ASCII_DATA
    msg ("[RLC_TM %p][SEND_SDU] DATA :", rlcP);
    for (index = 0; index < length_in_bytes; index++) {
      msg ("%c", srcP[index]);
    }
    msg ("\n");
#endif

    memcpy (&rlcP->output_sdu_in_construction->data[rlcP->output_sdu_size_to_write], srcP, length_in_bytes);

#warning loss of error indication parameter
    rlc_data_ind (rlcP->module_id, rlcP->rb_id, length_in_bytes, rlcP->output_sdu_in_construction, rlcP->data_plane);
    rlcP->output_sdu_in_construction = NULL;
  } else {
    msg ("[RLC_TM %p][SEND_SDU] ERROR  OUTPUT SDU IS NULL\n", rlcP);
  }
}

//-----------------------------------------------------------------------------
void
rlc_tm_send_sdu_segment (struct rlc_tm_entity *rlcP, u8_t error_indicationP)
{
//-----------------------------------------------------------------------------
  if ((rlcP->output_sdu_in_construction) && (rlcP->output_sdu_size_to_write)) {
#ifdef DEBUG_RLC_TM_SEND_SDU
    msg ("[RLC_TM %p] SEND_SDU   %d bytes\n", rlcP, rlcP->output_sdu_size_to_write);
#endif
#warning loss of error indication parameter
    rlc_data_ind (rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_size_to_write, rlcP->output_sdu_in_construction, rlcP->data_plane);
    rlcP->output_sdu_in_construction = NULL;
    rlcP->output_sdu_size_to_write = 0;
  }
}
