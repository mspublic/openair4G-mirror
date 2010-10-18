/*
                             rlc_um_proto_extern.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#    ifndef __RLC_UM_PROTO_EXTERN_H__
#        define __RLC_UM_PROTO_EXTERN_H__
#        ifdef RLC_UM_C
#            define private_rlc_um(x)    
#            define protected_rlc_um(x)  
#            define public_rlc_um(x)     
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um(x) 
#                define protected_rlc_um(x)  extern x
#                define public_rlc_um(x)     extern x
#            else
#                define private_rlc_um(x) 
#                define protected_rlc_um(x)  
#                define public_rlc_um(x)     extern x
#            endif
#        endif
#        include "rlc_um_entity.h"
#        include "mem_block.h"

public_rlc_um( void     rlc_um_stat_req     (struct rlc_um_entity *rlcP, 
							  unsigned int* tx_pdcp_sdu,
							  unsigned int* tx_pdcp_sdu_discarded,
							  unsigned int* tx_data_pdu,
							  unsigned int* rx_sdu,
							  unsigned int* rx_error_pdu,  
							  unsigned int* rx_data_pdu,
							  unsigned int* rx_data_pdu_out_of_window);)
private_rlc_um(   void     rlc_um_get_pdus (void *argP);)
protected_rlc_um( void     rlc_um_rx (void *, struct mac_data_ind);)
public_rlc_um(    struct mac_status_resp rlc_um_mac_status_indication (void *rlcP, u16_t no_tbP, u16_t tb_sizeP, struct mac_status_ind tx_statusP);)
public_rlc_um(    struct mac_data_req rlc_um_mac_data_request (void *rlcP);)
private_rlc_um(   void     rlc_um_mac_data_indication (void *rlcP, struct mac_data_ind data_indP);)
public_rlc_um(    void     rlc_um_data_req (void *rlcP, mem_block_t *sduP);)
#    endif
