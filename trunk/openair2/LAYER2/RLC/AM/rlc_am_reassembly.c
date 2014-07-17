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
/***************************************************************************
                          rlc_am_reassembly.c  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr


 ***************************************************************************/
#define RLC_AM_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_am_entity.h"
#include "rlc_am_structs.h"
#include "rlc_primitives.h"
#include "rlc_am_constants.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"
#define DEBUG_RLC_AM_SEND_SDU
//#define DEBUG_REASSEMBLY
//#define DEBUG_RLC_AM_DISPLAY_ASCII_DATA

//-----------------------------------------------------------------------------
void
reassembly (uint8_t * srcP, uint16_t lengthP, struct rlc_am_entity *rlcP)
{
//-----------------------------------------------------------------------------
  int             sdu_max_size_allowed;
#ifdef DEBUG_RLC_AM_DISPLAY_ASCII_DATA
  int             index;
#endif
#ifdef DEBUG_REASSEMBLY
  msg ("[RLC_AM][RB %d][REASSEMBLY] reassembly()  %d bytes\n", rlcP->rb_id, lengthP);
#endif

  if ((rlcP->data_plane)) {
    sdu_max_size_allowed = RLC_SDU_MAX_SIZE_DATA_PLANE;
  } else {
    sdu_max_size_allowed = RLC_SDU_MAX_SIZE_CONTROL_PLANE;
  }

  if (rlcP->output_sdu_in_construction == NULL) {
    rlcP->output_sdu_in_construction = get_free_mem_block (sdu_max_size_allowed);
    rlcP->output_sdu_size_to_write = 0;
  }
#ifdef DEBUG_REASSEMBLY
  msg ("[RLC_AM][RB %d][REASSEMBLY] reassembly()  %d bytes sdu_max_size allowed %d\n", rlcP->rb_id, lengthP, sdu_max_size_allowed);
#endif
  if ((rlcP->output_sdu_in_construction)) {
#ifdef DEBUG_RLC_AM_DISPLAY_ASCII_DATA
    msg ("[RLC_AM][RB %d][REASSEMBLY] DATA :", rlcP->rb_id);
    for (index = 0; index < lengthP; index++) {
      //msg ("%c", srcP[index]);
      msg ("%02X.", srcP[index]);
    }
    msg ("\n");
#endif

    // OOOPS
    if ((lengthP + rlcP->output_sdu_size_to_write) <= sdu_max_size_allowed) {

      memcpy (&rlcP->output_sdu_in_construction->data[rlcP->output_sdu_size_to_write], srcP, lengthP);
      rlcP->output_sdu_size_to_write += lengthP;
    } else {
      rlcP->output_sdu_size_to_write = 0;
#ifdef DEBUG_REASSEMBLY
      msg ("[RLC_AM %p][REASSEMBLY] ERROR SDU IN CONSTRUCTION TOO BIG %d Bytes MAX SIZE ALLOWED %d\n", rlcP, lengthP + rlcP->output_sdu_size_to_write, sdu_max_size_allowed);
#endif
    }

  }
#ifdef DEBUG_REASSEMBLY
  else {
    msg ("[RLC_AM %p][REASSEMBLY] ERROR  OUTPUT SDU IS NULL\n", rlcP);
  }
#endif
}

//-----------------------------------------------------------------------------
void
send_sdu (struct rlc_am_entity *rlcP)
{
//-----------------------------------------------------------------------------
  if ((rlcP->output_sdu_in_construction) && (rlcP->output_sdu_size_to_write)) {

#ifdef DEBUG_RLC_AM_SEND_SDU
    msg ("[RLC_AM][RB %d][SEND_SDU] send_SDU()  %d bytes\n", rlcP->rb_id, rlcP->output_sdu_size_to_write);
#endif




#ifdef BENCH_QOS_L2
    fprintf (bench_l2, "[SDU DELIVERY] FRAME %d SIZE %d RB %d RLC-AM %p\n", Mac_rlc_xface->frame, rlcP->output_sdu_size_to_write, rlcP->rb_id, rlcP);
#endif

    rlc_data_ind (rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_size_to_write, rlcP->output_sdu_in_construction, rlcP->data_plane);
    rlcP->output_sdu_in_construction = NULL;
    rlcP->output_sdu_size_to_write = 0;
    rlcP->stat_rx_sdu += 1;
  }
}
