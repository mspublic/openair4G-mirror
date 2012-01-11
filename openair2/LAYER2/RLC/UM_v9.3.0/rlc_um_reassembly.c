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
#define RLC_UM_MODULE
#define RLC_UM_REASSEMBLY_C
#include "platform_types.h"
//-----------------------------------------------------------------------------
#ifdef USER_MODE
#include <string.h>
#endif
#include "rlc.h"
#include "rlc_um.h"
#include "rlc_primitives.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"

#define DEBUG_RLC_UM_REASSEMBLY 1
#define DEBUG_RLC_UM_DISPLAY_ASCII_DATA 1
//#define DEBUG_RLC_UM_SEND_SDU

//-----------------------------------------------------------------------------
inline void
rlc_um_clear_rx_sdu (rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
  rlcP->output_sdu_size_to_write = 0;
}

//-----------------------------------------------------------------------------
void
rlc_um_reassembly (u8_t * srcP, s32_t lengthP, rlc_um_entity_t *rlcP,u32_t frame)
{
//-----------------------------------------------------------------------------
  int             sdu_max_size;
#ifdef DEBUG_RLC_UM_DISPLAY_ASCII_DATA
  int             index;
#endif

  LOG_D(RLC, "[RLC_UM][MOD %d][RB %d][FRAME %05d][REASSEMBLY] reassembly()  %d bytes\n", rlcP->module_id, rlcP->rb_id, frame, lengthP);

  if (lengthP <= 0) {
      return;
  }

  if ((rlcP->is_data_plane)) {
    sdu_max_size = RLC_SDU_MAX_SIZE_DATA_PLANE;
  } else {
    sdu_max_size = RLC_SDU_MAX_SIZE_CONTROL_PLANE;
  }

  if (rlcP->output_sdu_in_construction == NULL) {
    //    msg("[RLC_UM_LITE] Getting mem_block ...\n");
    rlcP->output_sdu_in_construction = get_free_mem_block (RLC_SDU_MAX_SIZE_DATA_PLANE);
    rlcP->output_sdu_size_to_write = 0;
  }
  if ((rlcP->output_sdu_in_construction)) {

#ifdef DEBUG_RLC_UM_DISPLAY_ASCII_DATA
    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d][REASSEMBLY] DATA :", rlcP->module_id, rlcP->rb_id, frame);
    for (index = 0; index < lengthP; index++) {
      msg ("%02X.", srcP[index]);
    }
    msg ("\n");
#endif
    // check if no overflow in size
    if ((rlcP->output_sdu_size_to_write + lengthP) <= sdu_max_size) {
      memcpy (&rlcP->output_sdu_in_construction->data[rlcP->output_sdu_size_to_write], srcP, lengthP);
      rlcP->output_sdu_size_to_write += lengthP;
#ifdef DEBUG_RLC_UM_DISPLAY_ASCII_DATA
      rlcP->output_sdu_in_construction->data[rlcP->output_sdu_size_to_write] = 0;
#endif
    } else {
      LOG_E(RLC, "[RLC_UM][MOD %d][RB %d][FRAME %05d][REASSEMBLY] [max_sdu size %d] ERROR  SDU SIZE OVERFLOW SDU GARBAGED\n", rlcP->module_id, rlcP->rb_id, frame, sdu_max_size);
      // erase  SDU
      rlcP->output_sdu_size_to_write = 0;
    }
  } else {
    LOG_E(RLC, "[RLC_UM][MOD %d][RB %d][FRAME %05d][REASSEMBLY]ERROR  OUTPUT SDU IS NULL\n", rlcP->module_id, rlcP->rb_id, frame);
  }
}
//-----------------------------------------------------------------------------
void
rlc_um_send_sdu (rlc_um_entity_t *rlcP,u32_t frame, u8_t eNB_flag)
{
//-----------------------------------------------------------------------------
/*#ifndef USER_MODE
  unsigned long int rlc_um_time_us;
  int min, sec, usec;
#endif*/

  if ((rlcP->output_sdu_in_construction)) {
    LOG_D(RLC, "\n\n\n[RLC_UM][MOD %d][RB %d][FRAME %05d][SEND_SDU] %d bytes sdu %p\n", rlcP->module_id, rlcP->rb_id, frame, rlcP->output_sdu_size_to_write, rlcP->output_sdu_in_construction);
/*#ifndef USER_MODE
  rlc_um_time_us = (unsigned long int)(rt_get_time_ns ()/(RTIME)1000);
  sec = (rlc_um_time_us/ 1000000);
  min = (sec / 60) % 60;
  sec = sec % 60;
  usec =  rlc_um_time_us % 1000000;
  msg ("[RLC_UM_LITE][RB  %d] at time %2d:%2d.%6d\n", rlcP->rb_id, min, sec , usec);
#endif*/

    if (rlcP->output_sdu_size_to_write > 0) {
        rlcP->rx_sdus += 1;
        //msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d][SEND_SDU] ASCII=%s\n",rlcP->module_id, rlcP->rb_id, frame, rlcP->output_sdu_in_construction->data);
/*#ifdef USER_MODE
        if (strncmp(tcip_sdu, (char*)(&rlcP->output_sdu_in_construction->data[0]), strlen(tcip_sdu)) == 0) {
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d][SEND_SDU] OK SDU TCP-IP\n\n\n", rlcP->module_id, rlcP->rb_id, frame);
        } else if (strncmp(voip_sdu, rlcP->output_sdu_in_construction->data, strlen(voip_sdu)) == 0) {
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d][SEND_SDU] OK SDU VOIP\n\n\n", rlcP->module_id, rlcP->rb_id, frame);
        } else if (strncmp(very_small_sdu, rlcP->output_sdu_in_construction->data, strlen(very_small_sdu)) == 0) {
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d][SEND_SDU] OK SDU SMALL\n\n\n", rlcP->module_id, rlcP->rb_id, frame);
        } else {
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d][SEND_SDU] UNKNOWN SDU\n\n\n", rlcP->module_id, rlcP->rb_id, frame);
        }
#endif*/
#ifdef TEST_RLC_UM
        rlc_um_v9_3_0_test_data_ind (rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_size_to_write,
							 rlcP->output_sdu_in_construction);
#else
        // msg("[RLC] DATA IND ON MOD_ID %d RB ID %d, size %d\n",rlcP->module_id, rlcP->rb_id, frame,rlcP->output_sdu_size_to_write);
        rlc_data_ind (rlcP->module_id, frame, eNB_flag, rlcP->rb_id, rlcP->output_sdu_size_to_write, rlcP->output_sdu_in_construction,rlcP->is_data_plane);
#endif
        rlcP->output_sdu_in_construction = NULL;
    } else {
      LOG_E(RLC, "[RLC_UM][MOD %d][RB %d][FRAME %05d][SEND_SDU] ERROR SIZE <= 0 ... DO NOTHING, SET SDU SIZE TO 0\n",rlcP->module_id, rlcP->rb_id, frame);
    }
    rlcP->output_sdu_size_to_write = 0;
  }
}
