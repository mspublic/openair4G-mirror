/*
                                rlc_mac.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
*/

//-----------------------------------------------------------------------------
#define RLC_MAC_C
#include "rtos_header.h"
#include "rlc.h"

#define DEBUG_MAC_INTERFACE

// tb_size_t in bytes
//-----------------------------------------------------------------------------
struct mac_data_ind mac_rlc_deserialize_tb (char* bufferP, tb_size_t tb_sizeP, num_tb_t num_tbP, crc_t *crcsP) {
//-----------------------------------------------------------------------------

  struct mac_data_ind  data_ind;
  mem_block_t*         tb;
  num_tb_t             nb_tb_read;
  tbs_size_t   tbs_size;

  nb_tb_read = 0;
  tbs_size   = 0;
  list_init(&data_ind.data, NULL);

  while (num_tbP > 0) {
        tb = get_free_mem_block(sizeof (mac_rlc_max_rx_header_size_t) + tb_sizeP);
        if (tb != NULL) {
            ((struct mac_tb_ind *) (tb->data))->first_bit = 0;
            ((struct mac_tb_ind *) (tb->data))->data_ptr = &tb->data[sizeof (mac_rlc_max_rx_header_size_t)];
            ((struct mac_tb_ind *) (tb->data))->size = tb_sizeP;
            if (crcsP)
	      ((struct mac_tb_ind *) (tb->data))->error_indication = crcsP[nb_tb_read];

            memcpy(((struct mac_tb_ind *) (tb->data))->data_ptr, &bufferP[tbs_size], tb_sizeP);

#ifdef DEBUG_MAC_INTERFACE
            int tb_size_in_bytes;
            msg ("[MAC-RLC] DUMP RX PDU(%d bytes):", tb_sizeP);
            for (tb_size_in_bytes = 0; tb_size_in_bytes < tb_sizeP; tb_size_in_bytes++) {
                msg ("%02X.", ((struct mac_tb_ind *) (tb->data))->data_ptr[tb_size_in_bytes]);
            }
            msg ("\n");
#endif
            nb_tb_read = nb_tb_read + 1;
            tbs_size   = tbs_size   + tb_sizeP;
            list_add_tail_eurecom(tb, &data_ind.data);
        }
        num_tbP = num_tbP - 1;
  }
  data_ind.no_tb            = nb_tb_read;
  //data_ind.error_indication = 0;
  data_ind.tb_size          = tb_sizeP << 3;
  // msg("[RLC] DESERIALIZE: TB_SIZE %d\n",tb_sizeP);

  return data_ind;
}
//-----------------------------------------------------------------------------
tbs_size_t mac_rlc_serialize_tb (char* bufferP, list_t transport_blocksP) {
//-----------------------------------------------------------------------------
  mem_block_t* tb;
  tbs_size_t   tbs_size;
  tbs_size_t   tb_size;

  tbs_size = 0;
  while (transport_blocksP.nb_elements > 0) {
    tb = list_remove_head (&transport_blocksP);
    if (tb != NULL) {
       tb_size = ((struct mac_tb_req *) (tb->data))->tb_size_in_bits>>3;
#ifdef DEBUG_MAC_INTERFACE
        int tb_size_in_bytes;
        msg ("[MAC-RLC] DUMP TX PDU(%d bytes):", tb_size);
        for (tb_size_in_bytes = 0; tb_size_in_bytes < tb_size; tb_size_in_bytes++) {
            msg ("%02X.", ((struct mac_tb_req *) (tb->data))->data_ptr[tb_size_in_bytes]);
        }
        msg ("\n");
#endif
      // printf("mac_rlc_serialize_tb() tb size %d\n", tb_size);
       memcpy(&bufferP[tbs_size], &((struct mac_tb_req *) (tb->data))->data_ptr[0], tb_size);
       //msg("[RLC-MAC] if RAB UM TB SN %d\n", (unsigned int)(bufferP[tbs_size] >> 1) & 0x7F);
       tbs_size = tbs_size + tb_size;
       free_mem_block(tb);
    }
  }
  return tbs_size;
}
//-----------------------------------------------------------------------------
tbs_size_t mac_rlc_data_req     (module_id_t module_idP, chan_id_t channel_idP, char* bufferP) {
//-----------------------------------------------------------------------------
  struct mac_data_req data_request;
  if ((module_idP >= 0) && (module_idP < MAX_MODULES)) {
      if ((channel_idP >= 0) && (channel_idP < MAX_RAB)) {
                  switch (rlc[module_idP].m_rlc_pointer[channel_idP].rlc_type) {
                    case RLC_NONE:
                        //handle_event(WARNING,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : no radio bearer configured :%d\n", __FILE__, __LINE__, channel_idP);
                        break;

                    case RLC_AM:
                        data_request = rlc_am_mac_data_request(&rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[channel_idP].rlc_index]);
                        return mac_rlc_serialize_tb(bufferP, data_request.data);
                        break;

                    case RLC_UM:
                        data_request = rlc_um_mac_data_request(&rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[channel_idP].rlc_index]);
                        return mac_rlc_serialize_tb(bufferP, data_request.data);
                        break;

                    case RLC_TM:
                        data_request = rlc_tm_mac_data_request(&rlc[module_idP].m_rlc_tm_array[rlc[module_idP].m_rlc_pointer[channel_idP].rlc_index]);
                        return mac_rlc_serialize_tb(bufferP, data_request.data);
                        break;

                    default:;
                        //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : no RLC found for this radio bearer %d\n", __FILE__, __LINE__, channel_idP);

                  }
      } else {
          //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : parameter rb_id out of bounds :%d\n", __FILE__, __LINE__, channel_idP);
          msg("mac_rlc_data_req() : parameter rb_id out of bounds :%d\n", channel_idP);
	  return(-1);
#ifdef USER_MODE
	  exit(-1);
#endif
      }
  } else {
      //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : parameter module_id out of bounds :%d\n", __FILE__, __LINE__, module_idP);
    msg("FONCTION mac_rlc_data_req() : parameter module_id out of bounds :%d\n", module_idP);
  }
  return (tbs_size_t)0;
}
//-----------------------------------------------------------------------------
void mac_rlc_data_ind     (module_id_t module_idP, chan_id_t rb_idP, char* bufferP, tb_size_t tb_sizeP, num_tb_t num_tbP, crc_t *crcs) {
//-----------------------------------------------------------------------------
#ifdef DEBUG_MAC_INTERFACE
  msg("\n[RLC] Inst %d: MAC_RLC_DATA_IND on RB %d, Num_tb %d\n",module_idP,rb_idP,num_tbP);
  #endif // DEBUG_MAC_INTERFACE

  if ((module_idP >= 0) && (module_idP < MAX_MODULES)) {
      if ((rb_idP >= 0) && (rb_idP < MAX_RAB)) {
	//msg("OK for RB_ID\n");
              if (num_tbP > 0) {
		//msg("OK for NB_TB\n");
                  struct mac_data_ind data_ind = mac_rlc_deserialize_tb(bufferP, tb_sizeP, num_tbP, crcs);


                  switch (rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type) {
                    case RLC_NONE:
                        //handle_event(WARNING,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : no radio bearer configured :%d\n", __FILE__, __LINE__, rb_idP);
                        msg(" FONCTION mac_rlc_data_ind()  : no radio bearer configured :%d\n",  rb_idP);
                        break;

                    case RLC_AM:
#ifdef DEBUG_MAC_INTERFACE
		      msg("MAC DATA IND TO RLC_AM MOD_ID %d RB_INDEX %d (%d) MOD_ID_RLC %d\n", module_idP, rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index, rb_idP, rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index].module_id);
#endif

		      rlc_am_mac_data_indication(&rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], data_ind);
                        break;

                    case RLC_UM:
#ifdef DEBUG_MAC_INTERFACE
		      msg("MAC DATA IND TO RLC_UM MOD_ID %d RB_INDEX %d MOD_ID_RLC %d\n", module_idP, rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index, rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index].module_id);
#endif
                        rlc_um_mac_data_indication(&rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], data_ind);
                        break;

                    case RLC_TM:
                        rlc_tm_mac_data_indication(&rlc[module_idP].m_rlc_tm_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], data_ind);
                        break;

                    default:
                        //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : no RLC found for this radio bearer %d\n", __FILE__, __LINE__, rb_idP);
                        msg("FILE  FONCTION mac_rlc_data_ind() LINE  : no RLC found for this radio bearer %d\n",  rb_idP);
                        ;

                  }
              }
      } else {
          //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : parameter rb_id out of bounds :%d\n", __FILE__, __LINE__, rb_idP);
      }
  } else {
      //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : parameter module_id out of bounds :%d\n", __FILE__, __LINE__, module_idP);
  }
}
//-----------------------------------------------------------------------------
mac_rlc_status_resp_t mac_rlc_status_ind     (module_id_t module_idP, chan_id_t channel_idP, tb_size_t tb_sizeP) {
//-----------------------------------------------------------------------------
  mac_rlc_status_resp_t mac_rlc_status_resp;
  mac_rlc_status_resp.bytes_in_buffer = 0;
  if ((module_idP >= 0) && (module_idP < MAX_MODULES)) {
      if ((channel_idP >= 0) && (channel_idP < MAX_RAB)) {
          struct mac_status_resp status_resp;
          struct mac_status_ind tx_status;
                  switch (rlc[module_idP].m_rlc_pointer[channel_idP].rlc_type) {
                    case RLC_NONE:
                        //handle_event(WARNING,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : no radio bearer configured :%d\n", __FILE__, __LINE__, channel_idP);
                        break;

                    case RLC_AM:
                        status_resp = rlc_am_mac_status_indication(&rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[channel_idP].rlc_index], tb_sizeP, tx_status);
                        mac_rlc_status_resp.bytes_in_buffer = status_resp.buffer_occupancy_in_bytes;
                        mac_rlc_status_resp.pdus_in_buffer = status_resp.buffer_occupancy_in_pdus;
                        return mac_rlc_status_resp;
                        break;

                    case RLC_UM:
                        msg("[RLC_UM][MOD %d] mac_rlc_status_ind  tb_size %d\n", module_idP,  tb_sizeP);

                        status_resp = rlc_um_mac_status_indication(&rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[channel_idP].rlc_index],  tb_sizeP, tx_status);
                        mac_rlc_status_resp.bytes_in_buffer = status_resp.buffer_occupancy_in_bytes;
                        //mac_rlc_status_resp.pdus_in_buffer = status_resp.buffer_occupancy_in_pdus;
                        return mac_rlc_status_resp;
                        break;

                    case RLC_TM:
                        status_resp = rlc_tm_mac_status_indication(&rlc[module_idP].m_rlc_tm_array[rlc[module_idP].m_rlc_pointer[channel_idP].rlc_index], tb_sizeP, tx_status);
                        mac_rlc_status_resp.bytes_in_buffer = status_resp.buffer_occupancy_in_bytes;
                        mac_rlc_status_resp.pdus_in_buffer = status_resp.buffer_occupancy_in_pdus;
                        return mac_rlc_status_resp;
                        break;

                    default:;
                        //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : no RLC found for this radio bearer %d\n", __FILE__, __LINE__, channel_idP);

                  }
      } else {
          //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : parameter rb_id out of bounds :%d\n", __FILE__, __LINE__, channel_idP);
      }
  } else {
      //handle_event(ERROR,"FILE %s FONCTION mac_rlc_data_ind() LINE %s : parameter module_id out of bounds :%d\n", __FILE__, __LINE__, module_idP);
  }
  return mac_rlc_status_resp;
}

