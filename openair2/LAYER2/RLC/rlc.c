/*
                                rlc.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
*/


#define RLC_C
#include "rtos_header.h"
#include "rlc.h"
//#include "mpls.h"
#include "mem_block.h"
#include "../MAC/extern.h"
#include "./AM/rlc_am_util_proto_extern.h"
//#include "../PDCP/pdcp_proto_extern.h"
extern void pdcp_data_ind (module_id_t module_idP, rb_id_t rab_idP, sdu_size_t data_sizeP, mem_block_t * sduP);

#define DEBUG_RLC_PDCP_INTERFACE

#define DEBUG_RLC_DATA_REQ 1

//-----------------------------------------------------------------------------
rlc_op_status_t rlc_stat_req     (module_id_t module_idP,
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
rlc_op_status_t rlc_data_req     (module_id_t module_idP, rb_id_t rb_idP, mui_t muiP, confirm_t confirmP, sdu_size_t sdu_sizeP, mem_block_t *sduP) {
//-----------------------------------------------------------------------------
  mem_block_t* new_sdu;

#ifdef DEBUG_RLC_DATA_REQ
  msg("rlc_data_req: module_idP %d (%d), rb_idP %d (%d), muip %d, confirmP %d, sud_sizeP %d, sduP %p\n",module_idP,MAX_MODULES,rb_idP,MAX_RAB,muiP,confirmP,sdu_sizeP,sduP);
#endif
  if ((module_idP >= 0) && (module_idP < MAX_MODULES)) {
      if ((rb_idP >= 0) && (rb_idP < MAX_RAB)) {
          if (sduP != NULL) {
              if (sdu_sizeP > 0) {
#ifdef DEBUG_RLC_DATA_REQ
		msg("RLC_TYPE : %d\n",rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type);
#endif
                  switch (rlc[module_idP].m_rlc_pointer[rb_idP].rlc_type) {
                    case RLC_NONE:
                    free_mem_block(sduP);
                        //handle_event(WARNING,"FILE %s FONCTION rlc_data_req() LINE %s : no radio bearer configured :%d\n", __FILE__, __LINE__, rb_idP);
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
                          rlc_am_data_req(&rlc[module_idP].m_rlc_am_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], new_sdu);
                          return RLC_OP_STATUS_OK;
                        } else {
                          //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : out of memory\n", __FILE__, __LINE__);
                          return RLC_OP_STATUS_INTERNAL_ERROR;
                        }
                        break;

                    case RLC_UM:
		       // msg("[RLC] Getting RLC_UM memblock\n");
                        new_sdu = get_free_mem_block (sdu_sizeP + sizeof (struct rlc_um_data_req_alloc));

                        if (new_sdu != NULL) {
                          // PROCESS OF COMPRESSION HERE:
                          memset (new_sdu->data, 0, sizeof (struct rlc_um_data_req_alloc));
                          memcpy (&new_sdu->data[sizeof (struct rlc_um_data_req_alloc)], &sduP->data[0], sdu_sizeP);

                          ((struct rlc_um_data_req *) (new_sdu->data))->data_size = sdu_sizeP;
                          ((struct rlc_um_data_req *) (new_sdu->data))->data_offset = sizeof (struct rlc_um_data_req_alloc);
                          free_mem_block(sduP);

			  rlc_um_data_req(&rlc[module_idP].m_rlc_um_array[rlc[module_idP].m_rlc_pointer[rb_idP].rlc_index], new_sdu);

                          //free_mem_block(new_sdu);
                          return RLC_OP_STATUS_OK;
                        } else {
                          //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : out of memory\n", __FILE__, __LINE__);
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
void rlc_data_ind     (module_id_t module_idP, rb_id_t rb_idP, sdu_size_t sdu_sizeP, mem_block_t* sduP, boolean_t is_data_planeP) {
//-----------------------------------------------------------------------------
    if ((is_data_planeP)) {
#ifdef DEBUG_RLC_PDCP_INTERFACE
      msg("[RLC] TTI %d, INST %d : Receiving SDU (%p) of size %d bytes to Rb_id %d\n",
	  Mac_rlc_xface->frame, module_idP,
	  sduP,
	  sdu_sizeP,
          rb_idP);
#endif //DEBUG_RLC_PDCP_INTERFACE
       pdcp_data_ind (module_idP, rb_idP, sdu_sizeP, sduP);
    } else {
        if (rlc_rrc_data_ind != NULL) {
#ifdef DEBUG_RLC_PDCP_INTERFACE
            msg("[RLC] TTI %d, INST %d : Receiving SDU (%p) of size %d bytes to Rb_id %d\n",
	    Mac_rlc_xface->frame, module_idP,
	    sduP,
	    sdu_sizeP,
            rb_idP);
#endif //DEBUG_RLC_PDCP_INTERFACE
	  // msg("[RLC] RRC DATA IND\n");
            rlc_rrc_data_ind(module_idP , rb_idP , sdu_sizeP , sduP->data);
	  //msg("[RLC] Freeing SDU\n");
            free_mem_block(sduP);
        }
    }
}
//-----------------------------------------------------------------------------
void rlc_data_conf     (module_id_t module_idP, rb_id_t rb_idP, mui_t muiP, rlc_tx_status_t statusP, boolean_t is_data_planeP) {
//-----------------------------------------------------------------------------
    if (!(is_data_planeP)) {
        if (rlc_rrc_data_conf != NULL) {
            rlc_rrc_data_conf (module_idP , rb_idP , muiP, statusP);
        }
    }
}
//-----------------------------------------------------------------------------
int
rlc_module_init ()
{
//-----------------------------------------------------------------------------
   msg("[RLC] MODULE INIT\n");
   rlc_rrc_data_ind  = NULL;
   rlc_rrc_data_conf = NULL;
   memset(rlc, sizeof(rlc_t) * MAX_MODULES, 0);

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

