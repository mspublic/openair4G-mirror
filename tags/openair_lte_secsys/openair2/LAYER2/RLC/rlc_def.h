/*
                               rlc_def.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#ifndef __RLC_DEF_H__
#    define __RLC_DEF_H__
//----------------------------------------------------------
// protocol states
#    define RLC_NULL_STATE                            0x00
#    define RLC_DATA_TRANSFER_READY_STATE             0x01
#    define RLC_RESET_PENDING_STATE                   0x12
#    define RLC_RESET_AND_SUSPEND_STATE               0x14
#    define RLC_LOCAL_SUSPEND_STATE                   0x08
//----------------------------------------------------------
#    define RLC_SDU_MAX_SIZE_CONTROL_PLANE              600    // RRC CONNECTION SETUP MAY BE HUGE
#    define RLC_SDU_MAX_SIZE_DATA_PLANE                 5004
//----------------------------------------------------------
// dimensions
#    define SN_12BITS_MASK                            0x0FFF
#    define RLC_SN_OVERFLOW                           0xFFFF
//----------------------------------------------------------
// DISCARD
//----------------------------------------------------------
#    define RLC_SDU_NO_DISCARD_MAX_DAT_RETRANSMISSION 0x00
                                                        // AM (do not change this value)
#    define RLC_SDU_DISCARD_TIMER_BASED_EXPLICIT      0x01
                                                        // AM     (must>0)
#    define RLC_SDU_DISCARD_TIMER_BASED_NO_EXPLICIT   0x02
                                                        // UM, TM (must>0)
#    define RLC_SDU_DISCARD_MAX_DAT_RETRANSMISSION    0x04
                                                        // AM     (must>0)
#    define RLC_SDU_DISCARD_NOT_CONFIGURED            0x10
                                                        // UM, TM (must>0)
//----------------------------------------------------------
// DATA, CONTROL PDU parameters
//----------------------------------------------------------
// D/C field (values shifted 7 bits left)
#    define RLC_DC_CONTROL_PDU                        0
#    define RLC_DC_DATA_PDU                           128
#    define RLC_DC_MASK                               128
// HE field
#    define RLC_HE_SUCC_BYTE_CONTAINS_DATA            0x00
#    define RLC_HE_SUCC_BYTE_CONTAINS_LI_E            0x01
#    define RLC_HE_MASK                               0x03
// Extension bit
#    define RLC_E_NEXT_FIELD_IS_DATA                  0x00
#    define RLC_E_NEXT_FIELD_IS_LI_E                  0x01
#    define RLC_E_MASK                                0x01
// li field (values shifted 1 bit left)
#    define RLC_LI_LAST_PDU_EXACTLY_FILLED            0x0000
#    define RLC_LI_LAST_PDU_ONE_BYTE_SHORT            0xFFF6
#    define RLC_LI_1ST_BYTE_SDU_IS_1ST_BYTE_PDU       0xFFF8
#    define RLC_LI_PDU_PIGGY_BACKED_STATUS            0xFFFC
#    define RLC_LI_PDU_PADDING                        0xFFFE
#    define RLC_LI_UNDEFINED                          0xF0F0
#    define RLC_LI_MASK                               0xFFFE
// piggybacked status PDU (values shifted 4 bits left)
#    define RLC_PIGGY_PDU_TYPE_STATUS                 0x00
#    define RLC_PDU_TYPE_STATUS                       0x00
#    define RLC_PIGGY_PDU_TYPE_RESET                  0x10
#    define RLC_PDU_TYPE_RESET                        0x10
#    define RLC_PIGGY_PDU_TYPE_RESET_ACK              0x20
#    define RLC_PDU_TYPE_RESET_ACK                    0x20
#    define RLC_PIGGY_PDU_TYPE_MASK                   0x70
#    define RLC_PDU_TYPE_MASK                         0x70

#    define GUARD_CRC_LIH_SIZE                        0x03
                                                        // in bytes

#endif
