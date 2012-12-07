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

#define RLC_C
#include "rlc.h"
#include "mem_block.h"
#include "../MAC/extern.h"
#include "UTIL/LOG/log.h"
extern void pdcp_data_ind (module_id_t module_idP, u32_t frame, u8_t eNB_flag, rb_id_t rab_idP, sdu_size_t data_sizeP, mem_block_t * sduP, u8 is_data_plane);

#define DEBUG_RLC_PDCP_INTERFACE

#define DEBUG_RLC_DATA_REQ 1

//-----------------------------------------------------------------------------
void rlc_util_print_hex_octets(comp_name_t componentP, unsigned char* dataP, unsigned long sizeP)
//-----------------------------------------------------------------------------
{
  unsigned long octet_index = 0;

  if (dataP == NULL) {
    return;
  }


  LOG_D(componentP, "      |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f |\n");
  LOG_D(componentP, "------+-------------------------------------------------|\n");
  for (octet_index = 0; octet_index < sizeP; octet_index++) {
    if ((octet_index % 16) == 0){
      if (octet_index != 0) {
          LOG_T(componentP, " |\n");
      }
      LOG_T(componentP, " %04d |", octet_index);
    }
    /*
     * Print every single octet in hexadecimal form
     */
    LOG_T(componentP, " %02x", dataP[octet_index]);
    /*
     * Align newline and pipes according to the octets in groups of 2
     */
  }

  /*
   * Append enough spaces and put final pipe
   */
  unsigned char index;
  for (index = octet_index; index < 16; ++index)
    LOG_T(componentP, "   ");
  LOG_T(componentP, " |\n");
}

//-----------------------------------------------------------------------------
rlc_op_status_t rlc_stat_req     (module_id_t module_idP,
				  u32_t frame,
				  rb_id_t        rb_idP,
				  unsigned int* tx_pdcp_sdu,
				  unsigned int* tx_pdcp_sdu_discarded,
				  unsigned int* tx_retransmit_pdu_unblock,
				  unsigned int* tx_retransmit_pdu_by_status,
				  unsigned int* tx_retransmit_pdu,
				  unsigned int* tx_data_pdu,
				  unsigned int* tx_control_pdu,
				  unsigned int* rx_sdu,
				  unsigned int* rx_error_pdu,
				  unsigned int* rx_data_pdu,
				  unsigned int* rx_data_pdu_out_of_window,
				  unsigned int* rx_control_pdu) {
//-----------------------------------------------------------------------------
  if ((module_idP >= 0) && (module_idP < MAX_MODULES)) {
      if ((rb_idP >= 0) && (rb_idP < MAX_RAB)) {
            switch (rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type) {
                    case RLC_NONE:
                			*tx_pdcp_sdu = 0;
                			*tx_pdcp_sdu_discarded = 0;
                			*tx_retransmit_pdu_unblock = 0;
                			*tx_retransmit_pdu_by_status = 0;
                			*tx_retransmit_pdu = 0;
                			*tx_data_pdu = 0;
                			*tx_control_pdu = 0;
                			*rx_sdu = 0;
                			*rx_error_pdu = 0;
                			*rx_data_pdu = 0;
                			*rx_data_pdu_out_of_window = 0;
                			*rx_control_pdu = 0;
                        return RLC_OP_STATUS_BAD_PARAMETER;
                        break;

                    case RLC_AM:
		      rlc_am_stat_req     (&rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index],
					   frame,
					   tx_pdcp_sdu,
					   tx_pdcp_sdu_discarded,
					   tx_retransmit_pdu_unblock,
					   tx_retransmit_pdu_by_status,
					   tx_retransmit_pdu,
					   tx_data_pdu,
					   tx_control_pdu,
					   rx_sdu,
					   rx_error_pdu,
					   rx_data_pdu,
					   rx_data_pdu_out_of_window,
					   rx_control_pdu);
                          return RLC_OP_STATUS_OK;
                        break;

                    case RLC_UM:
            			*tx_pdcp_sdu = 0;
            			*tx_pdcp_sdu_discarded = 0;
            			*tx_retransmit_pdu_unblock = 0;
            			*tx_retransmit_pdu_by_status = 0;
            			*tx_retransmit_pdu = 0;
            			*tx_data_pdu = 0;
            			*tx_control_pdu = 0;
            			*rx_sdu = 0;
            			*rx_error_pdu = 0;
            			*rx_data_pdu = 0;
            			*rx_data_pdu_out_of_window = 0;
            			*rx_control_pdu = 0;
				rlc_um_stat_req     (&rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index],
						     frame,
						     tx_pdcp_sdu,
						     tx_pdcp_sdu_discarded,
						     tx_data_pdu,
						     rx_sdu,
						     rx_error_pdu,
						     rx_data_pdu,
						     rx_data_pdu_out_of_window);
				return RLC_OP_STATUS_OK;
                        break;

                    case RLC_TM:
		      *tx_pdcp_sdu = 0;
		      *tx_pdcp_sdu_discarded = 0;
		      *tx_retransmit_pdu_unblock = 0;
		      *tx_retransmit_pdu_by_status = 0;
		      *tx_retransmit_pdu = 0;
		      *tx_data_pdu = 0;
		      *tx_control_pdu = 0;
		      *rx_sdu = 0;
		      *rx_error_pdu = 0;
		      *rx_data_pdu = 0;
		      *rx_data_pdu_out_of_window = 0;
		      *rx_control_pdu = 0;
		      return RLC_OP_STATUS_BAD_PARAMETER;
		      break;

                    default:
		      *tx_pdcp_sdu = 0;
		      *tx_pdcp_sdu_discarded = 0;
		      *tx_retransmit_pdu_unblock = 0;
		      *tx_retransmit_pdu_by_status = 0;
		      *tx_retransmit_pdu = 0;
		      *tx_data_pdu = 0;
		      *tx_control_pdu = 0;
		      *rx_sdu = 0;
		      *rx_error_pdu = 0;
		      *rx_data_pdu = 0;
		      *rx_data_pdu_out_of_window = 0;
		      *rx_control_pdu = 0;
		      return RLC_OP_STATUS_BAD_PARAMETER;

                  }
      } else {
          return RLC_OP_STATUS_BAD_PARAMETER;
      }
  } else {
      return RLC_OP_STATUS_BAD_PARAMETER;
  }
}
//-----------------------------------------------------------------------------
rlc_op_status_t rlc_data_req     (module_id_t module_idP, u32_t frame, u8_t eNB_flagP, rb_id_t rb_idP, mui_t muiP, confirm_t confirmP, sdu_size_t sdu_sizeP, mem_block_t *sduP) {
//-----------------------------------------------------------------------------
  mem_block_t* new_sdu;

#ifdef DEBUG_RLC_DATA_REQ
  msg("rlc_data_req: module_idP %d (%d), rb_idP %d (%d), muip %d, confirmP %d, sud_sizeP %d, sduP %p\n",module_idP,MAX_MODULES,rb_idP,MAX_RAB,muiP,confirmP,sdu_sizeP,sduP);
#endif
  if ((module_idP >= 0) && (module_idP < MAX_MODULES)) {
      if ((rb_idP >= 0) && (rb_idP < MAX_RAB)) {
          if (sduP != NULL) {
              if (sdu_sizeP > 0) {
                  LOG_D(RLC, "[FRAME %05d][RLC][MOD %02d][RB %02d] Display of rlc_data_req:\n",
                                 frame, module_idP, rb_idP);
                  rlc_util_print_hex_octets(RLC, sduP->data, sdu_sizeP);

#ifdef DEBUG_RLC_DATA_REQ
                  LOG_D(RLC,"RLC_TYPE : %d ",rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type);
#endif
                  switch (rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type) {
                    case RLC_NONE:
                        free_mem_block(sduP);
                        return RLC_OP_STATUS_BAD_PARAMETER;
                        break;

                    case RLC_AM:
#ifdef DEBUG_RLC_DATA_REQ
		                msg("RLC_AM\n");
#endif
                        new_sdu = get_free_mem_block (sdu_sizeP + sizeof (struct rlc_am_data_req_alloc));

                        if (new_sdu != NULL) {
                          // PROCESS OF COMPRESSION HERE:
                          memset (new_sdu->data, 0, sizeof (struct rlc_am_data_req_alloc));
                          memcpy (&new_sdu->data[sizeof (struct rlc_am_data_req_alloc)], &sduP->data[0], sdu_sizeP);

                          ((struct rlc_am_data_req *) (new_sdu->data))->data_size = sdu_sizeP;
                          ((struct rlc_am_data_req *) (new_sdu->data))->conf = confirmP;
                          ((struct rlc_am_data_req *) (new_sdu->data))->mui  = muiP;
                          ((struct rlc_am_data_req *) (new_sdu->data))->data_offset = sizeof (struct rlc_am_data_req_alloc);
                    	  free_mem_block(sduP);
                          LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);

                          if (rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index].is_data_plane) {
                              LOG_D(RLC, "[MSC_MSG][FRAME %05d][PDCP][MOD %02d][RB %02d][--- RLC_AM_DATA_REQ/%d Bytes --->][RLC_AM][MOD %02d][RB %02d]\n",
                                 frame, module_idP, rb_idP, sdu_sizeP, module_idP, rb_idP);
                          } else {
                              if (eNB_flagP) {
                                  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- RLC_AM_DATA_REQ/%d Bytes --->][RLC_AM][MOD %02d][RB %02d]\n",
                                     frame, module_idP, sdu_sizeP, module_idP, rb_idP);
                              } else {
                                  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- RLC_AM_DATA_REQ/%d Bytes --->][RLC_AM][MOD %02d][RB %02d]\n",
                                     frame, module_idP,  sdu_sizeP, module_idP, rb_idP);
                              }
                          }
                          LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
                          rlc_am_data_req(&rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], frame, new_sdu);
                          return RLC_OP_STATUS_OK;
                        } else {
                          return RLC_OP_STATUS_INTERNAL_ERROR;
                        }
                        break;

                    case RLC_UM:
                        new_sdu = get_free_mem_block (sdu_sizeP + sizeof (struct rlc_um_data_req_alloc));

                        if (new_sdu != NULL) {
                          // PROCESS OF COMPRESSION HERE:
                          memset (new_sdu->data, 0, sizeof (struct rlc_um_data_req_alloc));
                          memcpy (&new_sdu->data[sizeof (struct rlc_um_data_req_alloc)], &sduP->data[0], sdu_sizeP);

                          ((struct rlc_um_data_req *) (new_sdu->data))->data_size = sdu_sizeP;
                          ((struct rlc_um_data_req *) (new_sdu->data))->data_offset = sizeof (struct rlc_um_data_req_alloc);
                          free_mem_block(sduP);

                          LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);
                          if (rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index].is_data_plane) {
                              LOG_D(RLC, "[MSC_MSG][FRAME %05d][PDCP][MOD %02d][RB %02d][--- RLC_UM_DATA_REQ/%d Bytes --->][RLC_UM][MOD %02d][RB %02d]\n",
                                 frame, module_idP, rb_idP, sdu_sizeP, module_idP, rb_idP);
                          } else {
                              if (eNB_flagP) {
                                  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- RLC_UM_DATA_REQ/%d Bytes --->][RLC_UM][MOD %02d][RB %02d]\n",
                                     frame, module_idP,  sdu_sizeP, module_idP, rb_idP);
                              } else {
                                  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- RLC_UM_DATA_REQ/%d Bytes --->][RLC_UM][MOD %02d][RB %02d]\n",
                                     frame, module_idP,  sdu_sizeP, module_idP, rb_idP);
                              }
                          }
                          LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
                          rlc_um_data_req(&rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], frame, new_sdu);

                          //free_mem_block(new_sdu);
                          return RLC_OP_STATUS_OK;
                        } else {
                          return RLC_OP_STATUS_INTERNAL_ERROR;
                        }
                        break;

                    case RLC_TM:
                        new_sdu = get_free_mem_block (sdu_sizeP + sizeof (struct rlc_tm_data_req_alloc));

                        if (new_sdu != NULL) {
                          // PROCESS OF COMPRESSION HERE:
                          memset (new_sdu->data, 0, sizeof (struct rlc_tm_data_req_alloc));
                          memcpy (&new_sdu->data[sizeof (struct rlc_tm_data_req_alloc)], &sduP->data[0], sdu_sizeP);

                          ((struct rlc_tm_data_req *) (new_sdu->data))->data_size = sdu_sizeP;
                          ((struct rlc_tm_data_req *) (new_sdu->data))->data_offset = sizeof (struct rlc_tm_data_req_alloc);
                          free_mem_block(sduP);
                          LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);
                          if (rlc[module_idP].m_rlc_tm_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index].is_data_plane) {
                              LOG_D(RLC, "[MSC_MSG][FRAME %05d][PDCP][MOD %02d][RB %02d][--- RLC_TM_DATA_REQ/%d Bytes --->][RLC_TM][MOD %02d][RB %02d]\n",
                                 frame, module_idP, rb_idP, sdu_sizeP, module_idP, rb_idP);
                          } else {
                              if (eNB_flagP) {
                                  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- RLC_TM_DATA_REQ/%d Bytes --->][RLC_TM][MOD %02d][RB %02d]\n",
                                     frame, module_idP, rb_idP, sdu_sizeP, module_idP, rb_idP);
                              } else {
                                  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- RLC_TM_DATA_REQ/%d Bytes --->][RLC_TM][MOD %02d][RB %02d]\n",
                                     frame, module_idP, rb_idP, sdu_sizeP, module_idP, rb_idP);
                              }
                          }
                          LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
                          rlc_tm_data_req(&rlc[module_idP].m_rlc_tm_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], new_sdu);
                          return RLC_OP_STATUS_OK;
                        } else {
                          //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : out of memory\n", __FILE__, __LINE__);
                          return RLC_OP_STATUS_INTERNAL_ERROR;
                        }
                        break;

                    default:
                   free_mem_block(sduP);
                        //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : no RLC found for this radio bearer %d\n", __FILE__, __LINE__, rb_idP);
                        return RLC_OP_STATUS_INTERNAL_ERROR;

                  }
              } else {
                free_mem_block(sduP);
                //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : SDU size is 0\n", __FILE__, __LINE__);
                return RLC_OP_STATUS_BAD_PARAMETER;
              }
          } else {
                free_mem_block(sduP);
                //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : SDU is NULL\n", __FILE__, __LINE__);
                return RLC_OP_STATUS_BAD_PARAMETER;
          }
      } else {
          free_mem_block(sduP);
          //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : parameter rb_id out of bounds :%d\n", __FILE__, __LINE__, rb_idP);
          return RLC_OP_STATUS_BAD_PARAMETER;
      }
  } else {
                free_mem_block(sduP);
      //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : parameter module_id out of bounds :%d\n", __FILE__, __LINE__, module_idP);
      return RLC_OP_STATUS_BAD_PARAMETER;
  }
}

//-----------------------------------------------------------------------------
void rlc_data_ind     (module_id_t module_idP, u32_t frame, u8_t eNB_flag, rb_id_t rb_idP, sdu_size_t sdu_sizeP, mem_block_t* sduP, boolean_t is_data_planeP) {
//-----------------------------------------------------------------------------
    LOG_D(RLC, "[FRAME %05d][RLC][MOD %02d][RB %02d] Display of rlc_data_ind:\n", frame, module_idP, rb_idP);
    rlc_util_print_hex_octets(RLC, sduP->data, sdu_sizeP);
    //check_mem_area();
    // now demux is done at PDCP 
    //  if ((is_data_planeP)) { 
#ifdef DEBUG_RLC_PDCP_INTERFACE
      msg("[RLC] TTI %d, INST %d : Receiving SDU (%p) of size %d bytes to Rb_id %d\n",
	  frame, module_idP,
	  sduP,
	  sdu_sizeP,
          rb_idP);
#endif //DEBUG_RLC_PDCP_INTERFACE
      switch (rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type) {
         case RLC_AM:
             LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_AM][MOD %02d][RB %02d][--- RLC_DATA_IND/%d Bytes --->][PDCP][MOD %02d][RB %02d]\n",frame, module_idP,rb_idP,sdu_sizeP, module_idP,rb_idP);
             break;
         case RLC_UM:
             LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_UM][MOD %02d][RB %02d][--- RLC_DATA_IND/%d Bytes --->][PDCP][MOD %02d][RB %02d]\n",frame, module_idP,rb_idP,sdu_sizeP, module_idP,rb_idP);
             break;
         case RLC_TM:
             LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_TM][MOD %02d][RB %02d][--- RLC_DATA_IND/%d Bytes --->][PDCP][MOD %02d][RB %02d]\n",frame, module_idP,rb_idP,sdu_sizeP, module_idP,rb_idP);
             break;
      }
      pdcp_data_ind (module_idP, frame, eNB_flag, rb_idP, sdu_sizeP, sduP, is_data_planeP);
      /*     } else {
        if (rlc_rrc_data_ind != NULL) {
#ifdef DEBUG_RLC_PDCP_INTERFACE
            msg("[RLC] Frame %d, INST %d : Receiving SDU (%p) of size %d bytes to Rb_id %d\n",
	    frame, module_idP,
	    sduP,
	    sdu_sizeP,
            rb_idP);
#endif //DEBUG_RLC_PDCP_INTERFACE
	    switch (rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type) {
            case RLC_AM:
                LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_AM][MOD %02d][RB %02d][--- RLC_DATA_IND/%d Bytes --->][RRC_%s][MOD %02d][]\n",frame, module_idP,rb_idP,sdu_sizeP, ( eNB_flag == 1) ? "eNB":"UE", module_idP);
                break;
            case RLC_UM:
                LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_UM][MOD %02d][RB %02d][--- RLC_DATA_IND/%d Bytes --->][RRC_%s][MOD %02d][]\n",frame, module_idP,rb_idP,sdu_sizeP, ( eNB_flag == 1) ? "eNB":"UE", module_idP);
                break;
            case RLC_TM:
                LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_TM][MOD %02d][RB %02d][--- RLC_DATA_IND/%d Bytes --->][RRC_%s][MOD %02d][]\n",frame, module_idP,rb_idP,sdu_sizeP, ( eNB_flag == 1) ? "eNB":"UE", module_idP);
                break;
        }
	  // msg("[RLC] RRC DATA IND\n");
            rlc_rrc_data_ind(module_idP , frame, eNB_flag, rb_idP , sdu_sizeP , sduP->data);
	  //msg("[RLC] Freeing SDU\n");
            free_mem_block(sduP);
	    }*/
	 // }
}
//-----------------------------------------------------------------------------
void rlc_data_conf     (module_id_t module_idP, u32_t frame, u8_t eNB_flag, rb_id_t rb_idP, mui_t muiP, rlc_tx_status_t statusP, boolean_t is_data_planeP) {
//-----------------------------------------------------------------------------
    if (!(is_data_planeP)) {
        if (rlc_rrc_data_conf != NULL) {
            LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);
            switch (rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type) {
                case RLC_AM:
                    LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_AM][MOD %02d][RB %02d][--- RLC_DATA_CONF /MUI %d --->][RRC_%s][MOD %02d][][RLC_DATA_CONF/ MUI %d]\n",frame, module_idP,rb_idP, ( eNB_flag == 1) ? "eNB":"UE", muiP, module_idP);
                    break;
                case RLC_UM:
                    LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_UM][MOD %02d][RB %02d][--- RLC_DATA_CONF /MUI %d --->][RRC_%s][MOD %02d][][RLC_DATA_CONF/ MUI %d]\n",frame, module_idP,rb_idP, ( eNB_flag == 1) ? "eNB":"UE", muiP, module_idP);
                    break;
                case RLC_TM:
                    LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_TM][MOD %02d][RB %02d][--- RLC_DATA_CONF /MUI %d --->][RRC_%s][MOD %02d][][RLC_DATA_CONF/ MUI %d]\n",frame, module_idP,rb_idP, ( eNB_flag == 1) ? "eNB":"UE", muiP, module_idP);
                    break;
            }
            LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
            rlc_rrc_data_conf (module_idP , rb_idP , muiP, statusP);
        }
    }
}
//-----------------------------------------------------------------------------
int
rlc_module_init ()
{
//-----------------------------------------------------------------------------
   LOG_D(RLC, "MODULE INIT\n");
   rlc_rrc_data_ind  = NULL;
   rlc_rrc_data_conf = NULL;
   memset(rlc, 0, sizeof(rlc_t) * MAX_MODULES);

   pool_buffer_init();

   return(0);
}
//-----------------------------------------------------------------------------
void
rlc_module_cleanup ()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
void
rlc_layer_init ()
{
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
void
rlc_layer_cleanup ()
//-----------------------------------------------------------------------------
{
}

