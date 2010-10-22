/*
                             rlc_um_structs.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#    ifndef __RLC_UM_STRUCTS_H__
#        define __RLC_UM_STRUCTS_H__

#        include "platform_types.h"
#        include "list.h"
#        include "rlc_am_constants.h"
#        include "mac_primitives.h"
#        include "rlc_primitives.h"
//#ifdef USER_MODE
#        include "mac_rlc_primitives.h"
//#endif //USER_MODE
//-----------------------
typedef struct rlc_um_tx_sdu_management {
  u8_t             *first_byte;
  s32_t             sdu_creation_time;
  u16_t             sdu_remaining_size;
  u16_t             sdu_test_remaining_size;
  u16_t             sdu_segmented_size;
  u16_t             sdu_size;
}rlc_um_tx_sdu_management_t;
//-----------------------
struct rlc_um_tx_data_pdu_management {
  union {
    struct mac_tb_req tb_req;
    struct mac_tx_tb_management tb_mngt;
#        ifdef BYPASS_L1
    struct mac_tb_ind dummy2;
    struct mac_rx_tb_management dummy3;
    struct rlc_indication dummy4;
#        endif
  } dummy;
};
//-----------------------
typedef struct rlc_um_pdu_sn_5 {
  unsigned fi:2;
  unsigned e:1;
  unsigned sn:5;
  u8_t     data[1];
}rlc_um_pdu_sn_5_t ;
//-----------------------
typedef struct rlc_um_pdu_sn_10 {
  unsigned r1:3;
  unsigned fi:2;
  unsigned e:1;
  unsigned sn:10;
  u8_t     data[1];
}rlc_um_pdu_sn_10_t ;

typedef struct rlc_um_e_li {
    unsigned e1:1;
    unsigned li1:11;
    unsigned e2:1;
    unsigned li2:11;
}rlc_um_e_li_t;
//-----------------------
struct rlc_um_data_req_alloc {  // alloc enought bytes for sdu mngt also
  union {
    struct rlc_um_data_req dummy1;
    struct rlc_um_tx_sdu_management dummy2;
  } dummy;
};
#    endif
