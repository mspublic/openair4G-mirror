#define RLC_AM_MODULE
#define RLC_AM_REASSEMBLY_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_am.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"
#define TRACE_RLC_AM_SEND_SDU
#define TRACE_RLC_AM_REASSEMBLY
#define TRACE_RLC_AM_DISPLAY_ASCII_DATA
//#define TRACE_RLC_AM_RX_DECODE
//-----------------------------------------------------------------------------
inline void rlc_am_clear_rx_sdu (rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
  rlcP->output_sdu_size_to_write = 0;
}
//-----------------------------------------------------------------------------
void rlc_am_reassembly (u8_t * srcP, s32_t lengthP, rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
#ifdef TRACE_RLC_AM_DISPLAY_HEX_DATA
  int             index;
#endif

#ifdef TRACE_RLC_AM_REASSEMBLY
  msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PAYLOAD] reassembly()  %d bytes\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, lengthP);
#endif

  if (rlcP->output_sdu_in_construction == NULL) {
    rlcP->output_sdu_in_construction = get_free_mem_block (RLC_SDU_MAX_SIZE);
    rlcP->output_sdu_size_to_write = 0;
    assert(rlcP->output_sdu_in_construction != NULL);
  }
  if (rlcP->output_sdu_in_construction != NULL) {

#ifdef TRACE_RLC_AM_DISPLAY_HEX_DATA
    msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PAYLOAD] DATA :", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
    for (index = 0; index < lengthP; index++) {
      msg ("%02X.", srcP[index]);
    }
    msg ("\n");
#endif
    // check if no overflow in size
    if ((rlcP->output_sdu_size_to_write + lengthP) <= RLC_SDU_MAX_SIZE) {
        memcpy (&rlcP->output_sdu_in_construction->data[rlcP->output_sdu_size_to_write], srcP, lengthP);
#ifdef TRACE_RLC_AM_DISPLAY_ASCII_DATA
        rlcP->output_sdu_in_construction->data[rlcP->output_sdu_size_to_write + lengthP] = 0;
        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PAYLOAD] DATA :", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
        msg ("%s\n", &rlcP->output_sdu_in_construction->data[rlcP->output_sdu_size_to_write]);
#endif
        rlcP->output_sdu_size_to_write += lengthP;
    } else {
      msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PAYLOAD] ERROR  SDU SIZE OVERFLOW SDU GARBAGED\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
      // erase  SDU
      rlcP->output_sdu_size_to_write = 0;
    }
  } else {
    msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PAYLOAD] ERROR  OUTPUT SDU IS NULL\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
  }
}
//-----------------------------------------------------------------------------
void rlc_am_send_sdu (rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
/*#ifndef USER_MODE
  unsigned long int rlc_um_time_us;
  int min, sec, usec;
#endif*/

  if ((rlcP->output_sdu_in_construction)) {
#ifdef TRACE_RLC_AM_SEND_SDU
    msg ("\n\n\n[FRAME %05d][RLC_AM][MOD %02d][RB %02d][SEND_SDU] %d bytes sdu %p\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_size_to_write, rlcP->output_sdu_in_construction);
/*#ifndef USER_MODE
  rlc_um_time_us = (unsigned long int)(rt_get_time_ns ()/(RTIME)1000);
  sec = (rlc_um_time_us/ 1000000);
  min = (sec / 60) % 60;
  sec = sec % 60;
  usec =  rlc_um_time_us % 1000000;
  msg ("[RLC_UM_LITE][RB  %d] at time %2d:%2d.%6d\n", rlcP->rb_id, min, sec , usec);
#endif*/
#endif
    if (rlcP->output_sdu_size_to_write > 0) {
#ifdef TRACE_RLC_STATS
        rlcP->rx_sdus += 1;
#endif
        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][SEND_SDU] ASCII=%s\n", mac_xface->frame,rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_in_construction->data);

#ifdef TEST_RLC_AM
        rlc_am_v9_3_0_test_data_ind (rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_size_to_write,
rlcP->output_sdu_in_construction);
#else
        rlc_data_ind (rlcP->module_id, rlcP->rb_id, rlcP->output_sdu_size_to_write, rlcP->output_sdu_in_construction, rlcP->is_data_plane);
#endif
        rlcP->output_sdu_in_construction = NULL;
    } else {
        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][SEND_SDU] ERROR SIZE <= 0 ... DO NOTHING, SET SDU SIZE TO 0\n", mac_xface->frame,rlcP->module_id, rlcP->rb_id);
      //msg("[RLC_AM][MOD %d] Freeing mem_block ...\n", rlcP->module_id);
      //free_mem_block (rlcP->output_sdu_in_construction);
      assert(3==4);
    }
    rlcP->output_sdu_size_to_write = 0;
  }
}
//-----------------------------------------------------------------------------
void rlc_am_reassemble_pdu(rlc_am_entity_t* rlcP, mem_block_t* tbP) {
//-----------------------------------------------------------------------------
    int i,j;

    rlc_am_pdu_info_t* pdu_info        = &((rlc_am_rx_pdu_management_t*)(tbP->data))->pdu_info;
#ifdef TRACE_RLC_AM_REASSEMBLY
    msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU SN=%03d\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, pdu_info->sn);
    rlc_am_display_data_pdu_infos(rlcP, pdu_info);
#endif

    if (pdu_info->e == RLC_E_FIXED_PART_DATA_FIELD_FOLLOW) {
        switch (pdu_info->fi) {
            case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef TRACE_RLC_AM_RX_DECODE
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU NO E_LI FI=11 (00)\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
#endif
                // one complete SDU
                rlc_am_send_sdu(rlcP); // may be not necessary
                rlc_am_reassembly (pdu_info->payload, pdu_info->payload_size, rlcP);
                rlc_am_send_sdu(rlcP); // may be not necessary
                //rlcP->reassembly_missing_sn_detected = 0;
                break;
            case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_PDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef TRACE_RLC_AM_RX_DECODE
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU NO E_LI FI=10 (01)\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
#endif
                // one beginning segment of SDU in PDU
                rlc_am_send_sdu(rlcP); // may be not necessary
                rlc_am_reassembly (pdu_info->payload, pdu_info->payload_size, rlcP);
                //rlcP->reassembly_missing_sn_detected = 0;
                break;
            case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef TRACE_RLC_AM_RX_DECODE
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU NO E_LI FI=01 (10)\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
#endif
                // one last segment of SDU
                //if (rlcP->reassembly_missing_sn_detected == 0) {
                    rlc_am_reassembly (pdu_info->payload, pdu_info->payload_size, rlcP);
                    rlc_am_send_sdu(rlcP);
                //} // else { clear sdu already done
                //rlcP->reassembly_missing_sn_detected = 0;
                break;
            case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef TRACE_RLC_AM_RX_DECODE
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU NO E_LI FI=00 (11)\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
#endif
                //if (rlcP->reassembly_missing_sn_detected == 0) {
                    // one whole segment of SDU in PDU
                    rlc_am_reassembly (pdu_info->payload, pdu_info->payload_size, rlcP);
                //} else {
                //    rlcP->reassembly_missing_sn_detected = 1; // not necessary but for readability of the code
                //}

                break;
#ifdef USER_MODE
            default:
                assert(0 != 0);
#endif
        }
    } else {
        switch (pdu_info->fi) {
            case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef TRACE_RLC_AM_RX_DECODE
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU FI=11 (00) Li=", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
                for (i=0; i < pdu_info->num_li; i++) {
                    msg("%d ",pdu_info->li_list[i]);
                }
                msg("\n");
                //msg(" remaining size %d\n",size);
#endif
                // N complete SDUs
                rlc_am_send_sdu(rlcP);
                j = 0;
                for (i = 0; i < pdu_info->num_li; i++) {
                    rlc_am_reassembly (&pdu_info->payload[j], pdu_info->li_list[i], rlcP);
                    rlc_am_send_sdu(rlcP);
                    j = j + pdu_info->li_list[i];
                }
                if (pdu_info->hidden_size > 0) { // normally should always be > 0 but just for help debug
                    // data is already ok, done by last loop above
                    rlc_am_reassembly (&pdu_info->payload[j], pdu_info->hidden_size, rlcP);
                    rlc_am_send_sdu(rlcP);
                }
                //rlcP->reassembly_missing_sn_detected = 0;
                break;
            case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_PDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef TRACE_RLC_AM_RX_DECODE
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU FI=10 (01) Li=", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
                for (i=0; i < pdu_info->num_li; i++) {
                    msg("%d ",pdu_info->li_list[i]);
                }
                msg("\n");
                //msg(" remaining size %d\n",size);
#endif
                // N complete SDUs + one segment of SDU in PDU
                rlc_am_send_sdu(rlcP);
                j = 0;
                for (i = 0; i < pdu_info->num_li; i++) {
                    rlc_am_reassembly (&pdu_info->payload[j], pdu_info->li_list[i], rlcP);
                    rlc_am_send_sdu(rlcP);
                    j = j + pdu_info->li_list[i];
                }
                if (pdu_info->hidden_size > 0) { // normally should always be > 0 but just for help debug
                    // data is already ok, done by last loop above
                    rlc_am_reassembly (&pdu_info->payload[j], pdu_info->hidden_size, rlcP);
                }
                //rlcP->reassembly_missing_sn_detected = 0;
                break;
            case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef TRACE_RLC_AM_RX_DECODE
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU FI=01 (10) Li=", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
                for (i=0; i < pdu_info->num_li; i++) {
                    msg("%d ",pdu_info->li_list[i]);
                }
                msg("\n");
                //msg(" remaining size %d\n",size);
#endif
                // one last segment of SDU + N complete SDUs in PDU
                j = 0;
                for (i = 0; i < pdu_info->num_li; i++) {
                    rlc_am_reassembly (&pdu_info->payload[j], pdu_info->li_list[i], rlcP);
                    rlc_am_send_sdu(rlcP);
                    j = j + pdu_info->li_list[i];
                }
                if (pdu_info->hidden_size > 0) { // normally should always be > 0 but just for help debug
                    // data is already ok, done by last loop above
                    rlc_am_reassembly (&pdu_info->payload[j], pdu_info->hidden_size, rlcP);
                    rlc_am_send_sdu(rlcP);
                }
                //rlcP->reassembly_missing_sn_detected = 0;
                break;
            case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef TRACE_RLC_AM_RX_DECODE
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][REASSEMBLY PDU] TRY REASSEMBLY PDU FI=00 (11) Li=", mac_xface->frame, rlcP->module_id, rlcP->rb_id);
                for (i=0; i < pdu_info->num_li; i++) {
                    msg("%d ",pdu_info->li_list[i]);
                }
                msg("\n");
                //msg(" remaining size %d\n",size);
#endif
                j = 0;
                for (i = 0; i < pdu_info->num_li; i++) {
                    rlc_am_reassembly (&pdu_info->payload[j], pdu_info->li_list[i], rlcP);
                    rlc_am_send_sdu(rlcP);
                    j = j + pdu_info->li_list[i];
                }
                if (pdu_info->hidden_size > 0) { // normally should always be > 0 but just for help debug
                    // data is already ok, done by last loop above
                    rlc_am_reassembly (&pdu_info->payload[j], pdu_info->hidden_size, rlcP);
                } else {
#ifdef USER_MODE
        //assert (5!=5);
#endif
                }
                //rlcP->reassembly_missing_sn_detected = 0;
                break;
#ifdef USER_MODE
            default:
                assert(1 != 1);
#endif
        }
    }
    free_mem_block(tbP);
}
