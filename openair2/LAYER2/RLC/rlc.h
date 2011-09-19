/*
                                 rlc.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#ifndef __RLC_H__
#    define __RLC_H__

#    include "platform_types.h"
#    include "platform_constants.h"
#    include "rlc_am_proto_extern.h"
#    include "rlc_tm_proto_extern.h"
#    include "rlc_am.h"
#    include "rlc_um.h"
#    include "rlc_am_structs.h"
#    include "rlc_tm_structs.h"
#    include "rlc_um_structs.h"
#    include "mem_block.h"
#    include "PHY/defs.h"
//-----------------------------------------------------------------------------
#    ifdef RLC_MAC_C
#        define private_rlc_mac(x) x
#        define public_rlc_mac(x) x
#    else
#        define private_rlc_mac(x)
#        define public_rlc_mac(x) extern x
#    endif

#    ifdef RLC_MPLS_C
#        define private_rlc_mpls(x) x
#        define public_rlc_mpls(x) x
#    else
#        define private_rlc_mpls(x)
#        define public_rlc_mpls(x) extern x
#    endif

#    ifdef RLC_RRC_C
#        define private_rlc_rrc(x) x
#        define public_rlc_rrc(x) x
#    else
#        define private_rlc_rrc(x)
#        define public_rlc_rrc(x) extern x
#    endif

#    ifdef RLC_C
#        define private_rlc(x) x
#        define protected_rlc(x) x
#        define public_rlc(x) x
#    else
#        define private_rlc(x)
#        if defined(RLC_MAC_C) || defined(RLC_MPLS_C) || defined(RLC_RRC_C) || defined(RLC_AM_C) || defined(RLC_TM_C) || defined(RLC_UM_C) || defined (PDCP_C)
#            define protected_rlc(x) extern x
#        else
#            define protected_rlc(x)
#        endif
#        define public_rlc(x) extern x
#    endif


//-----------------------------------------------------------------------------
// ERROR
// WARNING
// INFO


#define  RLC_OP_STATUS_OK                1
#define  RLC_OP_STATUS_BAD_PARAMETER     22
#define  RLC_OP_STATUS_INTERNAL_ERROR    2
#define  RLC_OP_STATUS_OUT_OF_RESSOURCES 3

#define  RLC_SDU_CONFIRM_YES   1
#define  RLC_SDU_CONFIRM_NO    0

#define  RLC_MUI_UNDEFINED     0




typedef volatile struct {
  u32_t             e_r;
  u32_t             timer_discard;
  u32_t             sdu_discard_mode;
  u32_t             segmentation_indication;
  u32_t             delivery_of_erroneous_sdu;
} rlc_tm_info_t;


typedef volatile struct {
  rlc_mode_t             rlc_mode;
  union {
      rlc_am_info_t              rlc_am_info;
      rlc_tm_info_t              rlc_tm_info;
      rlc_um_info_t              rlc_um_info;
  }rlc;
} rlc_info_t;

typedef  struct {
  u32_t                        bytes_in_buffer;
  u32_t                        pdus_in_buffer;
} mac_rlc_status_resp_t;


typedef struct {
  union {
    struct rlc_am_rx_pdu_management dummy1;
    struct rlc_tm_rx_pdu_management dummy2;
    //struct rlc_um_rx_pdu_management dummy3;
    struct mac_tb_ind dummy4;
    struct mac_rx_tb_management dummy5;
  } dummy;
} mac_rlc_max_rx_header_size_t;

//-----------------------------------------------------------------------------
//   PRIVATE INTERNALS OF RLC
//-----------------------------------------------------------------------------
#define  RLC_MAX_NUM_INSTANCES_RLC_AM  MAX_RB/2
#define  RLC_MAX_NUM_INSTANCES_RLC_UM  MAX_RB
#define  RLC_MAX_NUM_INSTANCES_RLC_TM  MAX_RB



protected_rlc(void            (*rlc_rrc_data_ind)  (module_id_t , rb_id_t , sdu_size_t , char* );)
protected_rlc(void            (*rlc_rrc_data_conf) (module_id_t , rb_id_t , mui_t, rlc_tx_status_t );)

typedef struct rlc_pointer_t {
    rlc_mode_t rlc_type;
    int        rlc_index;
} rlc_pointer_t;

typedef struct rlc_t {
    rlc_pointer_t        m_rlc_pointer[MAX_RB];
    rlc_am_entity_t      m_rlc_am_array[RLC_MAX_NUM_INSTANCES_RLC_AM];
    rlc_um_entity_t      m_rlc_um_array[RLC_MAX_NUM_INSTANCES_RLC_UM];
    struct rlc_tm_entity m_rlc_tm_array[RLC_MAX_NUM_INSTANCES_RLC_TM];
}rlc_t;

// RK-LG was protected, public for debug
public_rlc(rlc_t rlc[MAX_MODULES];)

private_rlc_mac(tbs_size_t            mac_rlc_serialize_tb   (char*, list_t);)
private_rlc_mac(struct mac_data_ind   mac_rlc_deserialize_tb (char*, tb_size_t, num_tb_t, crc_t *);)


//-----------------------------------------------------------------------------
//   PUBLIC INTERFACE WITH RRC
//-----------------------------------------------------------------------------
private_rlc_rrc(rlc_op_status_t rrc_rlc_remove_rlc   (module_id_t, rb_id_t );)
private_rlc_rrc(rlc_op_status_t rrc_rlc_add_rlc      (module_id_t, rb_id_t, rlc_mode_t);)
public_rlc_rrc( rlc_op_status_t rrc_rlc_config_req   (module_id_t, config_action_t, rb_id_t, rb_type_t, rlc_info_t );)
public_rlc_rrc( rlc_op_status_t rrc_rlc_data_req     (module_id_t, rb_id_t, mui_t, confirm_t, sdu_size_t, char *);)
public_rlc_rrc( void   rrc_rlc_register_rrc ( void (*rrc_data_indP)  (module_id_t , rb_id_t , sdu_size_t , char*),
                void (*rrc_data_conf) (module_id_t , rb_id_t , mui_t, rlc_tx_status_t) );)

//-----------------------------------------------------------------------------
//   PUBLIC INTERFACE WITH MPLS
//-----------------------------------------------------------------------------
//public_rlc_mpls(void mpls_rlc_data_req    (module_id_t, rb_id_t, sdu_size_t, mem_block_t*);)

//-----------------------------------------------------------------------------
//   PUBLIC INTERFACE WITH MAC
//-----------------------------------------------------------------------------
public_rlc_mac(tbs_size_t            mac_rlc_data_req     (module_id_t, chan_id_t, char*);)
public_rlc_mac(void                  mac_rlc_data_ind     (module_id_t, chan_id_t, char*, tb_size_t, num_tb_t, crc_t* );)
public_rlc_mac(mac_rlc_status_resp_t mac_rlc_status_ind   (module_id_t, chan_id_t, tb_size_t );)

//-----------------------------------------------------------------------------
//   PUBLIC RLC CONSTANTS
//-----------------------------------------------------------------------------
#define  RLC_NONE  0
#define  RLC_AM    1
#define  RLC_UM    2
#define  RLC_TM    4

//-----------------------------------------------------------------------------
//   RLC methods
//-----------------------------------------------------------------------------
public_rlc(rlc_op_status_t rlc_data_req     (module_id_t, rb_id_t, mui_t, confirm_t, sdu_size_t, mem_block_t*);)
public_rlc(void            rlc_data_ind     (module_id_t, rb_id_t, sdu_size_t, mem_block_t*, boolean_t);)
public_rlc(void            rlc_data_conf    (module_id_t, rb_id_t, mui_t, rlc_tx_status_t, boolean_t );)


public_rlc(rlc_op_status_t rlc_stat_req     (module_id_t module_idP,
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
							  unsigned int* rx_control_pdu) ;)

public_rlc(int rlc_module_init(void);)


#endif
