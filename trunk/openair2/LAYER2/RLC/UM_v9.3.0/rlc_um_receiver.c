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
#define RLC_UM_RECEIVER_C
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_um.h"
#include "rlc_um_structs.h"
#include "rlc_primitives.h"
#include "mac_primitives.h"
#include "list.h"
#include "MAC_INTERFACE/extern.h"
#include "UTIL/LOG/log.h"

#define DEBUG_RLC_UM_DISPLAY_TB_DATA
#define DEBUG_RLC_UM_RX

//-----------------------------------------------------------------------------
void rlc_um_display_rx_window(struct rlc_um_entity *rlcP)
//-----------------------------------------------------------------------------
{
/*
 *
 * #define RLC_FG_COLOR_BLACK            "\e[0;30m30:"
#define RLC_FG_COLOR_RED              "\e[0;31m31:"
#define RLC_FG_COLOR_GREEN            "\e[0;32m32:"
#define RLC_FG_COLOR_ORANGE           "\e[0;33m33:"
#define RLC_FG_COLOR_BLUE             "\e[0;34m34:"
#define RLC_FG_COLOR_MAGENTA          "\e[0;35m35:"
#define RLC_FG_COLOR_CYAN             "\e[0;36m36:"
#define RLC_FG_COLOR_GRAY_BLACK       "\e[0;37m37:"
#define RLC_FG_COLOR_DEFAULT          "\e[0;39m39:"
#define RLC_FG_BRIGHT_COLOR_DARK_GRAY "\e[1;30m30:"
#define RLC_FG_BRIGHT_COLOR_RED       "\e[1;31m31:"
#define RLC_FG_BRIGHT_COLOR_GREEN     "\e[1;32m32:"
#define RLC_FG_BRIGHT_COLOR_YELLOW    "\e[1;33m33:"
#define RLC_FG_BRIGHT_COLOR_BLUE      "\e[1;34m34:"
#define RLC_FG_BRIGHT_COLOR_MAGENTA   "\e[1;35m35:"
#define RLC_FG_BRIGHT_COLOR_CYAN      "\e[1;36m36:"
#define RLC_FG_BRIGHT_COLOR_WHITE     "\e[1;37m37:"
#define RLC_FG_BRIGHT_COLOR_DEFAULT   "\e[0;39m39:"
#define RLC_REVERSE_VIDEO             "\e[7m"
#define RLC_NORMAL_VIDEO              "\e[27m]"

 *
 */
    unsigned long sn = 0;
    unsigned long end_sn = 0;
    char         str[4];
    char         time_out_str[11];
    int          str_index;
    char         color[32];

    LOG_T(RLC, "\n");
    LOG_T(RLC, "+-------------------------------------------------------------------------------------------------------+");
    LOG_T(RLC, "\n");
    sprintf(time_out_str, "%010d", rlcP->timer_reordering  + rlcP->timer_reordering_init);
    time_out_str[10] = 0;
    LOG_T(RLC, "| RLC UM RB %02d    VR(UR)=%03d    VR(UX)=%03d    VR(UH)=%03d    t-Reordering: %s %s %s             |",
          rlcP->rb_id, rlcP->vr_ur, rlcP->vr_ux, rlcP->vr_uh,
      (rlcP->timer_reordering_running)?" ON":"OFF",
      (rlcP->timer_reordering_running)?"Time-out frame:":"               ",
      (rlcP->timer_reordering_running)?time_out_str:"          ");
    LOG_T(RLC, "\n");
    LOG_T(RLC, "+------+------------------------------------------------------------------------------------------------+");
    LOG_T(RLC, "\n");
    LOG_T(RLC, "|      |00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 |");
    LOG_T(RLC, "\n");
    LOG_T(RLC, "+------+------------------------------------------------------------------------------------------------+");
    LOG_T(RLC, "\n");
    if (rlcP->rx_sn_length == 10) {
        end_sn = RLC_UM_SN_10_BITS_MODULO;
    } else {
        end_sn = RLC_UM_SN_5_BITS_MODULO;
    }


    for (sn = 0; sn < end_sn; sn++) {
        str[0]    = ' ';
        str[1]    = ' ';
        str[2]    = ' ';
        str[3]    = 0;
        str_index = 0;
        if ((sn % 32) == 0){
            if ((sn != 0)){
                LOG_T(RLC, "%s%s|", RLC_FG_COLOR_DEFAULT, RLC_NORMAL_VIDEO);
                LOG_T(RLC, "\n");
            }
            LOG_T(RLC, "%s%s| %04d |", RLC_FG_COLOR_DEFAULT, RLC_NORMAL_VIDEO, sn);
        }
        strcpy(color, RLC_FG_COLOR_DEFAULT);
        if (sn == rlcP->vr_ur) {
            str[str_index++] = 'R';
            strcpy(color, RLC_FG_COLOR_BLUE);
        }
        if (sn == rlcP->vr_ux) {
            str[str_index++] = 'X';
            strcpy(color, RLC_FG_COLOR_ORANGE);
        }
        if (sn == rlcP->vr_uh) {
            str[str_index++] = 'H';
            strcpy(color, RLC_FG_COLOR_RED);
        }

        if (rlc_um_get_pdu_from_dar_buffer(rlcP, sn)) {
            // test RLC_REVERSE_VIDEO
            if (str_index <= 2) str[str_index] = '.';
            LOG_T(RLC, "%s%s%s", color, RLC_REVERSE_VIDEO, str);
        } else {
            LOG_T(RLC, "%s%s%s", color, RLC_NORMAL_VIDEO, str);
        }
    }
    LOG_T(RLC, "%s%s|", RLC_FG_COLOR_DEFAULT, RLC_NORMAL_VIDEO);
    LOG_T(RLC, "\n");
    LOG_T(RLC, "+------+------------------------------------------------------------------------------------------------+");
    LOG_T(RLC, "\n");
}

//-----------------------------------------------------------------------------
void
rlc_um_receive (struct rlc_um_entity *rlcP, u32_t frame, u8_t eNB_flag, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------

    mem_block_t        *tb;
    u8_t               *first_byte;
    u16_t               tb_size_in_bytes;

    while ((tb = list_remove_head (&data_indP.data))) {
#ifdef DEBUG_RLC_STATS
        rlcP->rx_pdus += 1;
#endif


#ifdef RLC_UM_GENERATE_ERRORS
        if (random() % 10 == 4) {
            ((struct mac_tb_ind *) (tb->data))->error_indication = 1;
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d]  RX PDU GENERATE ERROR", rlcP->module_id, rlcP->rb_id, frame);
        }
#endif

        if (!(((struct mac_tb_ind *) (tb->data))->error_indication)) {
            first_byte = ((struct mac_tb_ind *) (tb->data))->data_ptr;
            tb_size_in_bytes = ((struct mac_tb_ind *) (tb->data))->size;
            if (tb_size_in_bytes > 0) {
                rlc_um_receive_process_dar (rlcP, frame, eNB_flag, tb, (rlc_um_pdu_sn_10_t *)first_byte, tb_size_in_bytes);
                LOG_D(RLC, "[RLC_UM][MOD %d][RB %d][FRAME %05d] VR(UR)=%03d VR(UX)=%03d VR(UH)=%03d\n", rlcP->module_id, rlcP->rb_id, frame, rlcP->vr_ur, rlcP->vr_ux, rlcP->vr_uh);
                rlc_um_display_rx_window(rlcP);
            }
        } else {
            rlcP->rx_pdus_in_error += 1;
            LOG_D(RLC, "[MSC_NBOX][FRAME %05d][RLC_UM][MOD %02d][RB %d][TB indicated in error by MAC, dropped][RLC_UM][MOD %02d][RB %d]\n",
                 frame, rlcP->module_id, rlcP->rb_id, rlcP->module_id, rlcP->rb_id);
#ifdef DEBUG_RLC_UM_RX
            LOG_D(RLC, "[RLC_UM][MOD %d][RB %d][FRAME %05d] RX PDU WITH ERROR INDICATED BY LOWER LAYERS -> GARBAGE\n", rlcP->module_id, rlcP->rb_id, frame);
#endif
            free_mem_block (tb);
        }
    }                           // end while
}
