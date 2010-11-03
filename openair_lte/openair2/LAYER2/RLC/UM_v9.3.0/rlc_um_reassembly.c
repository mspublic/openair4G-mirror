#define RLC_UM_MODULE
#define RLC_UM_REASSEMBLY_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include <string.h>
#include "rlc.h"
#include "rlc_um.h"
#include "rlc_primitives.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"

#define DEBUG_RLC_UM_REASSEMBLY 1
#define DEBUG_RLC_UM_DISPLAY_ASCII_DATA 1
#define DEBUG_RLC_UM_SEND_SDU

//-----------------------------------------------------------------------------
inline void
rlc_um_clear_rx_sdu (rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
  rlcP->output_sdu_size_to_write = 0;
}

//-----------------------------------------------------------------------------
void
rlc_um_reassembly (u8_t * srcP, s32_t lengthP, rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
  int             sdu_max_size;
#ifdef DEBUG_RLC_UM_DISPLAY_ASCII_DATA
  int             index;
#endif

#ifdef DEBUG_RLC_UM_REASSEMBLY
  msg ("[RLC_UM][MOD %d][RB %d][REASSEMBLY] reassembly()  %d bytes\n", rlcP->module_id, rlcP->rb_id, lengthP);
#endif

  if ((rlcP->data_plane)) {
    sdu_max_size = RLC_SDU_MAX_SIZE_DATA_PLANE;
  } else {
    sdu_max_size = RLC_SDU_MAX_SIZE_CONTROL_PLANE;
  }
  if (rlcP->output_sdu_in_construction == NULL) {
    //    msg("[RLC_UM_LITE] Getting mem_block ...\n");
    rlcP->output_sdu_in_construction = get_free_mem_block (sdu_max_size);
    rlcP->output_sdu_size_to_write = 0;
  }
  if ((rlcP->output_sdu_in_construction)) {

#ifdef DEBUG_RLC_UM_DISPLAY_ASCII_DATA
    msg ("[RLC_UM][MOD %d][RB %d][REASSEMBLY] DATA :", rlcP->module_id, rlcP->rb_id);
    for (index = 0; index < lengthP; index++) {
      msg ("%02X.", srcP[index]);
    }
    msg ("\n");
#endif
    // check if no overflow in size
    if ((rlcP->output_sdu_size_to_write + lengthP) <= sdu_max_size) {
      memcpy (&rlcP->output_sdu_in_construction->data[rlcP->output_sdu_size_to_write], srcP, lengthP);
      rlcP->output_sdu_size_to_write += lengthP;
    } else {
      msg ("[RLC_UM][MOD %d][RB %d][REASSEMBLY] ERROR  SDU SIZE OVERFLOW SDU GARBAGED\n", rlcP->module_id, rlcP->rb_id);
      // erase  SDU
      rlcP->output_sdu_size_to_write = 0;
    }
  } else {
    msg ("[RLC_UM][MOD %d][RB %d][REASSEMBLY] ERROR  OUTPUT SDU IS NULL\n", rlcP->module_id, rlcP->rb_id);
  }
}
//-----------------------------------------------------------------------------
void
rlc_um_send_sdu (rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
/*#ifndef USER_MODE
  unsigned long int rlc_um_time_us;
  int min, sec, usec;
#endif*/

  if ((rlcP->output_sdu_in_construction)) {
#ifdef DEBUG_RLC_UM_SEND_SDU
    msg ("\n\n\n[RLC_UM][MOD %d][RB %d][SEND_SDU] %d bytes frame %d sdu %p\n", rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_size_to_write, Mac_rlc_xface->frame, rlcP->output_sdu_in_construction);
/*#ifndef USER_MODE
  rlc_um_time_us = (unsigned long int)(rt_get_time_ns ()/(RTIME)1000);
  sec = (rlc_um_time_us/ 1000000);
  min = (sec / 60) % 60;
  sec = sec % 60;
  usec =  rlc_um_time_us % 1000000;
  msg ("[RLC_UM_LITE][RB  %d] at time %2d:%2d.%6d\n", rlcP->rb_id, min, sec , usec);
#endif*/
    msg ("[RLC_UM][MOD %d][RB %d][SEND_SDU] ASCII=%s\n",rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_in_construction->data);
    if (strncmp(tcip_sdu, (char*)(&rlcP->output_sdu_in_construction->data[0]), strlen(tcip_sdu)) == 0) {
        msg ("[RLC_UM][MOD %d][RB %d][SEND_SDU] OK SDU TCP-IP\n\n\n", rlcP->module_id, rlcP->rb_id);
    } else if (strncmp(voip_sdu, rlcP->output_sdu_in_construction->data, strlen(voip_sdu)) == 0) {
        msg ("[RLC_UM][MOD %d][RB %d][SEND_SDU] OK SDU VOIP\n\n\n", rlcP->module_id, rlcP->rb_id);
    } else if (strncmp(very_small_sdu, rlcP->output_sdu_in_construction->data, strlen(very_small_sdu)) == 0) {
        msg ("[RLC_UM][MOD %d][RB %d][SEND_SDU] OK SDU SMALL\n\n\n", rlcP->module_id, rlcP->rb_id);
    } else {
        msg ("[RLC_UM][MOD %d][RB %d][SEND_SDU] UNKNOWN SDU\n\n\n", rlcP->module_id, rlcP->rb_id);
    }
#endif
    if (rlcP->output_sdu_size_to_write > 0) {
#ifdef DEBUG_RLC_STATS
      rlcP->rx_sdus += 1;
#endif
      // msg("[RLC] DATA IND ON MOD_ID %d RB ID %d, size %d\n",rlcP->module_id, rlcP->rb_id,rlcP->output_sdu_size_to_write);
      rlc_data_ind (rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_size_to_write, rlcP->output_sdu_in_construction, rlcP->data_plane);
    } else {

      msg ("[RLC_UM][MOD %d][RB %d][SEND_SDU] ERROR SIZE <= 0\n",rlcP->module_id, rlcP->rb_id);
      msg("[RLC_UM][MOD %d] Freeing mem_block ...\n", rlcP->module_id);
      free_mem_block (rlcP->output_sdu_in_construction);
    }
    rlcP->output_sdu_in_construction = NULL;
    rlcP->output_sdu_size_to_write = 0;
  }
}
