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

/***************************************************************************
                          umts_timer.c  -  description
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr



 ***************************************************************************/
//#include "rtos_header.h"
#include "platform_types.h"

#include "list.h"
#include "umts_timer_struct.h"
#include "mem_block.h"
#include "openair_defs.h"
//-----------------------------------------------------------------------------
void
umts_timer_check_time_out (list2_t * atimer_listP, uint32_t current_frame_tick_millisecondsP)
{
//-----------------------------------------------------------------------------
  struct timer_unit *timer;
  mem_block_t      *mem_unit;
  uint8_t              time_out = 255;
  mem_unit = atimer_listP->head;
  // do it simple now.
  while ((mem_unit) && (time_out)) {
    timer = (struct timer_unit *) (mem_unit->data);
    if ((current_frame_tick_millisecondsP - timer->frame_tick_start) >= timer->frame_time_out) {

      mem_unit = list2_remove_head (atimer_listP);
      (*(timer->proc)) (timer->protocol, timer->timer_id);
      free_mem_block (mem_unit);

      mem_unit = atimer_listP->head;
    } else {
      time_out = 0;
    }
  }
}

//-----------------------------------------------------------------------------
void
umts_timer_delete_timer (list2_t * atimer_listP, void *timer_idP)
{
//-----------------------------------------------------------------------------
  mem_block_t      *mem_unit;
  mem_unit = atimer_listP->head;

  while ((mem_unit)) {
    if (((struct timer_unit *) (mem_unit->data))->timer_id == timer_idP) {
      list2_remove_element (mem_unit, atimer_listP);
      free_mem_block (mem_unit);
      return;
    }
    mem_unit = mem_unit->next;
  }
}

//-----------------------------------------------------------------------------
mem_block_t      *
umts_add_timer_list_up (list2_t * atimer_listP, void (*procP) (void *, void *), void *protocolP, void *timer_idP, uint32_t frame_time_outP, uint32_t current_frame_tick_millisecondsP)
{
//-----------------------------------------------------------------------------
  struct mem_block_t *mb;
  struct timer_unit *timer;
  mem_block_t      *mem_unit;
  int32_t             remaining_time;
  uint8_t              inserted = 0;

  mb = get_free_mem_block (sizeof (struct timer_unit));
  ((struct timer_unit *) (mb->data))->proc = procP;
  ((struct timer_unit *) (mb->data))->protocol = protocolP;
  ((struct timer_unit *) (mb->data))->timer_id = timer_idP;
  ((struct timer_unit *) (mb->data))->frame_time_out = frame_time_outP;
  ((struct timer_unit *) (mb->data))->frame_tick_start = current_frame_tick_millisecondsP;

  // insert the timer in list in ascending order
  mem_unit = atimer_listP->head;
  while ((mem_unit) && (!inserted)) {
    timer = (struct timer_unit *) (mem_unit->data);

    remaining_time = timer->frame_time_out - current_frame_tick_millisecondsP + timer->frame_tick_start;
    // not timed out
    if ((remaining_time > 0) && (frame_time_outP < remaining_time)) {
      inserted = 255;
      if (mem_unit == atimer_listP->head) {
#ifdef DEBUG_TIMER
        msg ("[TIMER][CREATION] added timer_id %p at head time out %d current time %d proc %p \n", timer_idP, frame_time_outP, current_frame_tick_millisecondsP, *procP);
#endif
        list2_add_head (mb, atimer_listP);
      } else {
#ifdef DEBUG_TIMER
        msg ("[TIMER][CREATION] inserted timer_id %p  time out %d current time %d proc %p \n", timer_idP, frame_time_outP, current_frame_tick_millisecondsP, *procP);
#endif
        mb->previous = mem_unit->previous;
        mb->next = mem_unit;
        mem_unit->previous->next = mb;
        mem_unit->previous = mb;
      }
    } else {
      mem_unit = mem_unit->next;
    }
  }
  if (!inserted) {
#ifdef DEBUG_TIMER
    msg ("[TIMER][CREATION] added timer_id %p at tail time out %d current time %d proc %p \n", timer_idP, frame_time_outP, current_frame_tick_millisecondsP, *procP);
#endif
    list2_add_tail (mb, atimer_listP);
  }
  return mb;
}

//-----------------------------------------------------------------------------
void
umts_stop_all_timers (list2_t * atimer_listP)
{
//-----------------------------------------------------------------------------
  list2_free (atimer_listP);
}

//-----------------------------------------------------------------------------
void
umts_stop_all_timers_except (list2_t * atimer_listP, void (*procP) (void *, void *))
{
//-----------------------------------------------------------------------------
  struct timer_unit *timer;
  mem_block_t      *mem_unit;
  mem_block_t      *mem_unit_to_delete;
  mem_unit = atimer_listP->head;
  while ((mem_unit)) {
    timer = (struct timer_unit *) (mem_unit->data);
    if (timer->proc != procP) {
      mem_unit_to_delete = mem_unit;
      mem_unit = mem_unit->next;
      list2_remove_element (mem_unit_to_delete, atimer_listP);
      free_mem_block (mem_unit_to_delete);
    } else {
      mem_unit = mem_unit->next;
    }
  }
}
