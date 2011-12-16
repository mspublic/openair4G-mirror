#ifndef __RLC_DEF_LTE_H__
#    define __RLC_DEF_LTE_H__
//----------------------------------------------------------

#    define RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU         0x0
#    define RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_PDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU     0x1
#    define RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU     0x2
#    define RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU 0x3

#    define RLC_E_FIXED_PART_DATA_FIELD_FOLLOW                                          0x0
#    define RLC_E_FIXED_PART_E_AND_LI_FIELD_FOLLOW                                      0x1

#    define RLC_E_EXTENSION_PART_DATA_FIELD_FOLLOW                                      0x0
#    define RLC_E_EXTENSION_PART_E_AND_LI_FIELD_FOLLOW                                  0x1

#endif
