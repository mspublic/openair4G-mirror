/*
                             rlc_um_receiver.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#define RLC_UM_MODULE
#define RLC_UM_RECEIVER_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_um_entity.h"
#include "rlc_um_constants.h"
#include "rlc_um_structs.h"
#include "rlc_primitives.h"
#include "rlc_def.h"
#include "mac_primitives.h"
#include "list.h"
#include "rlc_um_reassembly_proto_extern.h"
#include "rlc_um_segment_proto_extern.h"
//#define DEBUG_RLC_UM_RX 1
//#define DEBUG_RLC_UM_RX_DECODE_LI 1
//#define DEBUG_RLC_UM_DISPLAY_TB_DATA 1
//-----------------------------------------------------------------------------
inline int
rlc_um_get_length_indicators_7 (u16_t * li_arrayP, u8_t * li_array_in_pduP)
{
//-----------------------------------------------------------------------------
    int                 nb_li = 0;
    while (((li_arrayP[nb_li] = ((u16_t) li_array_in_pduP[nb_li])) & RLC_E_NEXT_FIELD_IS_LI_E)
           && (nb_li < RLC_UM_SEGMENT_NB_MAX_LI_PER_PDU)) {
        li_arrayP[nb_li] = li_arrayP[nb_li] & (~(u8_t) RLC_E_NEXT_FIELD_IS_LI_E);
        nb_li++;
    }
    nb_li++;
    return nb_li;
}
//-----------------------------------------------------------------------------
inline int
rlc_um_get_length_indicators_15 (u16_t * li_arrayP, u16_t * li_array_in_pduP)
{
//-----------------------------------------------------------------------------
    int                 nb_li = 0;
    while ((li_arrayP[nb_li] = ((((u16_t) li_array_in_pduP[nb_li << 1]) << 8) + li_array_in_pduP[(nb_li << 1) + 1]))
           & RLC_E_NEXT_FIELD_IS_LI_E) {

        li_arrayP[nb_li] = li_arrayP[nb_li] & (~(u16_t) RLC_E_NEXT_FIELD_IS_LI_E);
        nb_li++;
    }
    nb_li++;                    // count the last li
    return nb_li;
}
//-----------------------------------------------------------------------------
void
rlc_um_receive_process_pdu_out_of_sequence_delivery (struct rlc_um_entity *rlcP, struct rlc_um_rx_pdu_management *pdu_mngtP, struct
                                                     rlc_um_rx_data_pdu_struct *dataP, u16_t tb_sizeP, u16 bad_crc_l1P)
{
//-----------------------------------------------------------------------------
    // 25.322v9.2.0 section 11.2.3.2:
    // To enable the recovery of SDUs from UMD PDUs that are received in different transmissions the receiving function shall store PDUs until all SDUs that are associated with the PDU can be reconstructed or until they are discarded in accordance with the procedures described below. SDUs are transferred to the upper layers as soon as all PDUs that contain the segments of the SDU and the "Length Indicator" indicating the end of the SDU  have been received.
    // Upon delivery of a set of UMD PDUs from the lower layer, the Receiver shall for each PDU (in the following SN denotes the sequence number of each PDU):
    // -   If the PDU is the first PDU received (after the receiving entity is established or re-established or after Timer_OSD expires):
    // -   VR(UOH) shall be assigned the value SN-1.
    // -   if VR(UOH) > SN > VR(UOH) – OSD_Window_Size then:
    // -   if a PDU with sequence number SN is already stored:
    // -   discard the PDU;
    // -   else:
    // -   store the PDU in sequence number order.
    // -   else:
    // -   VR(UOH) shall be assigned the value SN, thereby advancing the storage window;
    // -   store the PDU in sequence number order;
    // -   remove from storage any PDUs whose sequence numbers, SN, are outside of the storage window VR(UOH) > SN > VR(UOH) – OSD_Window_Size;
    // -   if Timer_OSD is active then Timer_OSD shall be stopped;
    // -   Timer_OSD shall be started.
    // -   if a PDU with sequence number SN was stored:
    // -   if the PDU contains one or more complete SDUs and/or if the PDU contains segments of  SDUs for which all the remaining segments and length indicators are contained in stored PDUs:
    // -   re-assemble the SDUs;
    // -   submit the SDUs to upper layers through the UM-SAP;
    // -   remove from storage any PDUs which do not contain any segment of a SDU that has not been re-assembled, and do not contain one of the special length indicators "0000 000", "0000 0000 0000 000" or "1111 1111 1111 011" that indicate the end of a SDU that has not been re-assembled.
    // NOTE 0: If PDUs are removed from storage after SDU recovery then retransmitted PDUs may result in the duplicate transfer of SDUs to the higher layers.
    // -   if Timer_OSD expires:
    // -   remove from storage all stored PDUs.
    // NOTE 1: When configured for out of sequence SDU delivery, the transmitter should consider the possibility that a loss of a number of 128  OSD_Window_Size consecutively numbered PDUs may result in an undetected protocol error in the receiver, if the transmit state variable VT(US), at the end of a time interval equal to the duration of Timer_OSD, is greater than 128 + SN  OSD_Window_Size + 1, where SN is the lowest sequence number of any PDU transmitted or retransmitted within that time interval.
    // NOTE 2: The transmitter should not concatenate within a single PDU, SDUs or fractions of SDUs that contain MBMS Access Information messages with SDUs or fractions of SDUs that contain other MCCH message types.
    // NOTE 3: SDUs are contained within consecutively numbered PDUs. To enable SDUs containing MBMS Access Information messages to be transmitted at their designated times, the transmitter may transmit PDUs out of sequence order.
    // NOTE 4: The transmitter should not transmit within a single PDU, SDUs or fractions of SDUs that contain MBMS Access Information messages with the special length indicator "0000 000","0000 0000 0000 000", and "1111 1111 1111 011".

}
//-----------------------------------------------------------------------------
void
rlc_um_receive_process_pdu_in_sequence_delivery (struct rlc_um_entity *rlcP, struct rlc_um_rx_pdu_management *pdu_mngtP, struct
                                                 rlc_um_rx_data_pdu_struct *dataP, u16_t tb_sizeP, u16 bad_crc_l1P)
{
//-----------------------------------------------------------------------------
    u8_t               *data_pdu;
    s32_t               remaining_data_size;
    int                 nb_li;
    int                 li_index;
    int                 length_indicator_index;
    int                 search_first_li;
    u16_t               li[RLC_UM_SEGMENT_NB_MAX_LI_PER_PDU];
    u16_t               li_synched;
    // 25.322v9.2.0 section 11.2.3.1:
    // Upon delivery of a set of UMD PDUs from the lower layer or from the duplicate avoidance and reordering subentity, the Receiver shall:
    // -if out-of-sequence reception is configured and  SN ≥ VR(UM):
    //      -discard the UMD PDU.
    // -else:
    //      -update VR(US) according to each received UMD PDU (see subclause 9.4);
    //      -if the updating step of VR(US) is not equal to one (i.e. one or more UMD PDUs are missing):
    //              -discard the SDUs that could have segments or "Length Indicators" indicating the end of the SDUs in the missing UMD PDUs according to subclauses 9.2.2.8 and 9.2.2.9.
    //      -if the special "Length Indicator" "1111 100" or "1111 1111 1111 100" is the first "Length Indicator" of a UMD PDU received on the downlink:
    //              -consider the first data octet in this UMD PDU as the first octet of an RLC SDU.
    //      -if the "Extension bit" indicates that the UMD PDU contains a complete SDU which is not segmented, concatenated or padded:
    //              -consider the data part in this UMD PDU as one complete RLC SDU.
    // -if the special "Length Indicator" "1111 101" or “1111 1111 1111 101” is the first "Length Indicator" of a UMD PDU received on the downlink:
    //      -consider the first data octet in this UMD PDU as the first octet of an RLC SDU and the last data octet as the last octet of the same RLC SDU.
    // -if the special "Length Indicator" "1111 1111 1111 010" is the first "Length Indicator" of a UMD PDU received on the downlink:
    //      -consider the first data octet in this UMD PDU as the first octet of an RLC SDU and the second last data octet as the last octet of the same RLC SDU.
    // -reassemble the received UMD PDUs into RLC SDUs;
    // -submit the RLC SDUs to upper layers through the UM-SAP.
    pdu_mngtP->sn = dataP->byte1 >> 1;

    // it seems that only pdu_mngtP->sn >= rlcP->vr_um condition should be OK
    // whatever is located the round robin created by vr_us and vr_um
    if ((rlcP->out_of_sequence_delivery) && (pdu_mngtP->sn >= rlcP->vr_um)
        ) {
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d] RX IN SEQ PDU SN %03d hex >= VR(UM) %03d hex -> GARBAGE\n", rlcP->module_id, rlcP->rb_id, pdu_mngtP->sn, rlcP->vr_um);
#endif
        return;
    }
#ifdef DEBUG_RLC_UM_RX
    msg ("[RLC_UM][MOD %d][RB %d] RX PDU SN %03d VR(US) %03d TBsize %d\n", rlcP->module_id, rlcP->rb_id, pdu_mngtP->sn, rlcP->vr_us, tb_sizeP);
#endif

    //   -update VR(US) according to each received UMD PDU
    if (pdu_mngtP->sn != rlcP->vr_us) {
        rlc_um_clear_rx_sdu (rlcP);
        // Parse the L.I. of the PDU
        //---------------------------------
        // NO LENGTH INDICATOR IN PDU
        //---------------------------------
        if ((dataP->byte1 & RLC_E_MASK) == (u8_t) RLC_E_NEXT_FIELD_IS_DATA) {
            // consider this pdu as lost = do not update vr_us
            return;
        } else {
            if (tb_sizeP <= 125) {
                nb_li = rlc_um_get_length_indicators_7 (li, dataP->li_data_7);
            } else {
                nb_li = rlc_um_get_length_indicators_15 (li, dataP->li_data_7);
            }
            if (nb_li > 0) {
                length_indicator_index = 0;
                search_first_li = 1;
                while ((length_indicator_index != nb_li) && ((search_first_li))) {
                    switch ((u8_t) li[length_indicator_index]) {
                        case (u8_t) RLC_LI_PDU_PADDING:
                            search_first_li = 0;
                            length_indicator_index++;
                            rlcP->vr_us = (pdu_mngtP->sn + 1) & 127;
                            return;
                            break;
                        case (u8_t) RLC_LI_LAST_PDU_EXACTLY_FILLED:
                        case (u8_t) RLC_LI_LAST_PDU_ONE_BYTE_SHORT_FILLED_BY_SDU:
                            search_first_li = 1;
                            length_indicator_index++;
                            break;
                        case (u8_t) RLC_LI_1ST_BYTE_PDU_IS_1ST_BYTE_SDU_LAST_BYTE_IGNORED:
                        case (u8_t) RLC_LI_1ST_BYTE_SDU_IS_1ST_BYTE_PDU:
                        case (u8_t) RLC_LI_1ST_BYTE_SDU_IS_1ST_BYTE_PDU_LAST_BYTE_SDU_IS_LAST_BYTE_PDU:
                            search_first_li = 0;
                            break;
                        case (u8_t) RLC_LI_PDU_NOT_FIRST_NOT_LAST_BYTE_SDU:
                            return;
                        default:
                            search_first_li = 1;
                            length_indicator_index++;
                    }
                }
            } else {
                return;
            }
            // if reach here we can process the LIs of the PDU at length_indicator_index
            // let's do it in the default case
        }
    } else {
        //---------------------------------
        // NO LENGTH INDICATOR IN PDU
        //---------------------------------
        if ((dataP->byte1 & RLC_E_MASK) == (u8_t) RLC_E_NEXT_FIELD_IS_DATA) {
            rlc_um_reassembly ((u8_t *) (&dataP->li_data_7[0]), tb_sizeP - 1, rlcP);
            //---------------------------------
            // 1 OR MORE LENGTH INDICATOR IN PDU
            //---------------------------------
        } else {
            nb_li = 0;
            if (tb_sizeP <= 125) {
                nb_li = rlc_um_get_length_indicators_7 (li, dataP->li_data_7);
            } else {
                nb_li = rlc_um_get_length_indicators_7 (li, dataP->li_data_7);
            }
        }
    }

    if (tb_sizeP <= 125) {
        remaining_data_size = tb_sizeP - 1 - nb_li;
        data_pdu = (u8_t *) (&dataP->li_data_7[nb_li]);
    } else {
        remaining_data_size = tb_sizeP - 1 - (nb_li << 1);
        data_pdu = (u8_t *) (&dataP->li_data_7[nb_li << 1]);
    }

    li_index = 0;
    while (li_index < nb_li) {
        switch (li[li_index]) {
            case (u8_t) RLC_LI_LAST_PDU_EXACTLY_FILLED:
#ifdef DEBUG_RLC_UM_RX_DECODE_LI
                msg ("[RLC_UM][MOD %d][RB %d] RX_7 PDU %p Li RLC_LI_LAST_PDU_EXACTLY_FILLED\n", rlcP->module_id, rlcP->rb_id, pdu_mngtP);
#endif
                if ((li_synched)) {
                    rlc_um_send_sdu (rlcP);
                }
                li_synched = 1;
                break;

            case (u8_t) RLC_LI_LAST_PDU_ONE_BYTE_SHORT:
#ifdef DEBUG_RLC_UM_RX_DECODE_LI
                msg ("[RLC_UM][MOD %d][RB %d] RX_7 PDU %p Li RLC_LI_LAST_PDU_ONE_BYTE_SHORT\n", rlcP->module_id, rlcP->rb_id, pdu_mngtP);
#endif
                if ((li_synched)) {
                    rlc_um_send_sdu_minus_1_byte (rlcP);
                }
                li_synched = 1;
                break;

            case (u8_t) RLC_LI_PDU_PIGGY_BACKED_STATUS:    // ignore for RLC-AM
            case (u8_t) RLC_LI_PDU_PADDING:
#ifdef DEBUG_RLC_UM_RX_DECODE_LI
                msg ("[RLC_UM][MOD %d][RB %d] RX_7 PDU %p Li RLC_LI_PDU_PADDING\n", rlcP->module_id, rlcP->rb_id, pdu_mngtP);
#endif
                remaining_data_size = 0;
                break;

            case (u8_t) RLC_LI_1ST_BYTE_SDU_IS_1ST_BYTE_PDU:
#ifdef DEBUG_RLC_UM_RX_DECODE_LI
                msg ("[RLC_UM][MOD %d][RB %d] RX_7 PDU %p Li RLC_LI_1ST_BYTE_SDU_IS_1ST_BYTE_PDU\n", rlcP->module_id, rlcP->rb_id, pdu_mngtP);
#endif
                rlc_um_clear_rx_sdu (rlcP);
                li_synched = 1;
                break;

            default:           // li is length
#ifdef DEBUG_RLC_UM_RX_DECODE_LI
                msg ("[RLC_UM][MOD %d][RB %d] RX_7 PDU %p Li LI_SIZE %d Bytes\n", rlcP->module_id, rlcP->rb_id, pdu_mngtP, li[li_index] >> 1);
#endif
                remaining_data_size = remaining_data_size - (li[li_index] >> 1);
                if ((li_synched)) {
                    rlc_um_reassembly (data_pdu, (li[li_index] >> 1), rlcP);
                    rlc_um_send_sdu (rlcP);
                }
                li_synched = 1;
                data_pdu = (u8_t *) ((u32_t) data_pdu + (li[li_index] >> 1));
        }
        li_index++;
    }
    if ((remaining_data_size > 0)) {
        rlc_um_reassembly (data_pdu, remaining_data_size, rlcP);
        remaining_data_size = 0;
    }
    rlcP->vr_us = (pdu_mngtP->sn + 1) & 127;
}

//-----------------------------------------------------------------------------
void
rlc_um_receive (struct rlc_um_entity *rlcP, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------

    struct rlc_um_rx_data_pdu_struct *data;
    struct rlc_um_rx_pdu_management *pdu_mngt;
    mem_block_t        *tb;
    u8_t               *first_byte;
    u8_t                tb_size_in_bytes;
    u8_t                first_bit;
    u8_t                bits_to_shift;
    u8_t                bits_to_shift_last_loop;

    while ((tb = list_remove_head (&data_indP.data))) {
#ifdef DEBUG_RLC_STATS
        rlcP->rx_pdus += 1;
#endif

#ifdef DEBUG_RLC_UM_DISPLAY_TB_DATA
        msg ("[RLC_UM][MOD %d][RB %d] DUMP RX PDU:", rlcP->module_id, rlcP->rb_id);
        for (tb_size_in_bytes = 0; tb_size_in_bytes < data_indP.tb_size; tb_size_in_bytes++) {
            msg ("%02X.", ((struct mac_tb_ind *) (tb->data))->data_ptr[tb_size_in_bytes]);
        }
        msg ("\n");
#endif
        if (!(((struct mac_tb_ind *) (tb->data))->error_indication)) {
            first_byte = ((struct mac_tb_ind *) (tb->data))->data_ptr;
            pdu_mngt = (struct rlc_um_rx_pdu_management *) (tb->data);
            tb_size_in_bytes = data_indP.tb_size;
            first_bit = ((struct mac_tb_ind *) (tb->data))->first_bit;
            pdu_mngt->first_byte = first_byte;
            data = (struct rlc_um_rx_data_pdu_struct *) (first_byte);
            if (rlcP->out_of_sequence_delivery) {
                rlc_um_receive_process_pdu_out_of_sequence_delivery (rlcP, pdu_mngt, data, data_indP.tb_size, ((struct mac_tb_ind *) (tb->data))->error_indication);
            } else {
                rlc_um_receive_process_pdu_in_sequence_delivery (rlcP, pdu_mngt, data, data_indP.tb_size, ((struct mac_tb_ind *) (tb->data))->error_indication);
            }
        } else {
#ifdef DEBUG_RLC_STATS
            rlcP->rx_pdus_in_error += 1;
#endif
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU  WITH ERROR INDICATED BY LOWER LAYERS -> GARBAGE\n", rlcP->module_id, rlcP->rb_id);
#endif
        }
        free_mem_block (tb);
    }                           // end while
}
