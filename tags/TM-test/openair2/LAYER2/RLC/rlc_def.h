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
#    define RLC_SDU_MAX_SIZE                            1800
#    define RLC_SDU_MAX_SIZE_CONTROL_PLANE              2000
#    define RLC_SDU_MAX_SIZE_DATA_PLANE                 1800
#    define RLC_MAX_FLEXIBLE_DATA_PDU_SIZE              1503
//----------------------------------------------------------
// dimensions
#    define SN_12BITS_MASK                            0x0FFF
#    define RLC_SN_OVERFLOW                           0xFFFF
//----------------------------------------------------------
// DISCARD
//----------------------------------------------------------
enum RLC_SDU_DISCARD_MODE { SDU_DISCARD_MODE_RESET = 0x00,
  SDU_DISCARD_MODE_TIMER_BASED_EXPLICIT = 0x01,
  SDU_DISCARD_MODE_MAX_DAT_RETRANSMISSION = 0x04,
  SDU_DISCARD_MODE_NOT_CONFIGURED = 0x10
};


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
#    define RLC_HE_SUCC_BYTE_CONTAINS_DATA                    0x00 // v9.2 ok
#    define RLC_HE_SUCC_BYTE_CONTAINS_LI_E                    0x01 // v9.2 ok
#    define RLC_HE_SUCC_BYTE_CONTAINS_DATA_END_PDU_IS_END_SDU 0x02 // v9.2 ok
#    define RLC_HE_MASK                                       0x03
// Extension bit
/* Section 9.2.2.5:
The interpretation of this bit depends on RLC mode and higher layer configuration:
-	In the UMD PDU, the "Extension bit" in the first octet has either the normal
	E-bit interpretation or the alternative E-bit interpretation depending on
	higher layer configuration. The "Extension bit" in all the other octects always
	has the normal E-bit interpretation.
-	In the AMD PDU, the "Extension bit" always has the normal E-bit interpretation.*/
#    define RLC_E_NEXT_FIELD_IS_COMPLETE_SDU          0x00 // alternative E-bit interpretation v9.2 ok
#    define RLC_E_NEXT_FIELD_IS_DATA                  0x00 // v9.2 ok
#    define RLC_E_NEXT_FIELD_IS_LI_E                  0x01 // v9.2 ok
#    define RLC_E_MASK                                0x01
// li field (values shifted 1 bit left)
#    define RLC_LI_LAST_PDU_ONE_BYTE_SHORT 0xFFF6 // TO BE REMOVED ONLY FOR COMPILATION

#    define RLC_LI_LAST_PDU_EXACTLY_FILLED                                      0x0000 // v9.2 ok
#    define RLC_LI_1ST_BYTE_PDU_IS_1ST_BYTE_SDU_LAST_BYTE_IGNORED               0xFFF4 // v9.2 ok UM only
#    define RLC_LI_LAST_PDU_ONE_BYTE_SHORT_FILLED_BY_SDU                        0xFFF6 // v9.2 ok
#    define RLC_LI_1ST_BYTE_SDU_IS_1ST_BYTE_PDU                                 0xFFF8 // v9.2 ok UM only
#    define RLC_LI_1ST_BYTE_SDU_IS_1ST_BYTE_PDU_LAST_BYTE_SDU_IS_LAST_BYTE_PDU  0xFFFA // v9.2 ok UM only
#    define RLC_LI_PDU_PIGGY_BACKED_STATUS                                      0xFFFC // v9.2 ok AM
#    define RLC_LI_PDU_NOT_FIRST_NOT_LAST_BYTE_SDU                              0xFFFC // v9.2 ok UM only
#    define RLC_LI_PDU_PADDING                                                  0xFFFE // v9.2 ok
#    define RLC_LI_UNDEFINED                                                    0xF0F0
#    define RLC_LI_MASK                                                         0xFFFE
// piggybacked status PDU (values shifted 4 bits left)
#    define RLC_PIGGY_PDU_TYPE_STATUS                 0x00
#    define RLC_PDU_TYPE_STATUS                       0x00 // v9.2 ok
#    define RLC_PIGGY_PDU_TYPE_RESET                  0x10
#    define RLC_PDU_TYPE_RESET                        0x10 // v9.2 ok
#    define RLC_PIGGY_PDU_TYPE_RESET_ACK              0x20
#    define RLC_PDU_TYPE_RESET_ACK                    0x20 // v9.2 ok
#    define RLC_PIGGY_PDU_TYPE_MASK                   0x70
#    define RLC_PDU_TYPE_MASK                         0x70 // v9.2 ok

#    define GUARD_CRC_LIH_SIZE                        0x03
                                                        // in bytes

#endif
